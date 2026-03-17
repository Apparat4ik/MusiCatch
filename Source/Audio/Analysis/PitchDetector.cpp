#include "PitchDetector.h"
#include <cmath>

PitchDetector::PitchDetector(int fftOrder)
    : fft(fftOrder) {
    int fftSize = 1 << fftOrder;
    fftData.malloc(fftSize * 2); 

    hannWindow.setSize(1, fftSize, false, false, true); // окно Ханна
    auto* w = hannWindow.getWritePointer(0);
    for (int i = 0; i < fftSize; ++i) {
        // формула для вычисления окна (приведение к нулю крайних значений)
        w[i] = 0.5f * (1.0f - std::cos(juce::MathConstants<float>::twoPi * i / fftSize));
    }
}

PitchDetector::~PitchDetector() = default;

std::vector<NoteData> PitchDetector::analyzeTrack(const juce::AudioBuffer<float>& buffer
                                                  , double sampleRate) {
    std::vector<NoteData> detectedNotes;
    if (buffer.getNumChannels() == 0 || buffer.getNumSamples() < fft.getSize())
        return detectedNotes;

    int numSamples = buffer.getNumSamples();
    int fftSize = fft.getSize();
    int hopSize = fftSize / 2; // Шаг окна

    int currentMidiPitch = -1;
    float currentNoteStart = 0.0f;
    int consecutiveFrames = 0;

    juce::AudioBuffer<float> windowBuffer(1, fftSize);

    // Скользим по всему записанному буферу
    for (int i = 0; i <= numSamples - fftSize; i += hopSize) {
        // Копируем окно данных
        windowBuffer.copyFrom(0, 0, buffer, 0, i, fftSize);
        
        // Используем базовый метод для определения питча в этом окне
        auto notes = analyze(windowBuffer, sampleRate);
        int detectedPitch = notes.empty() ? -1 : notes[0].midiPitch;
        
        float timeInSeconds = static_cast<float>(i) / sampleRate;

        // Если высота ноты изменилась (или началась/закончилась)
        if (detectedPitch != currentMidiPitch) {
            // Сохраняем предыдущую ноту, если она длилась достаточно долго (отсекаем короткие шумы)
            if (currentMidiPitch != -1 && consecutiveFrames > 3) {
                NoteData note;
                note.midiPitch = currentMidiPitch;
                note.startTime = currentNoteStart;
                note.duration = timeInSeconds - currentNoteStart;
                float rms = buffer.getRMSLevel(0, 0, numSamples); // громкость текущего буфера
                note.velocity = juce::jlimit(0.0f, 1.0f, rms * 2.0f);
                detectedNotes.push_back(note);
            }
            
            // Начинаем отслеживать новую ноту
            currentMidiPitch = detectedPitch;
            currentNoteStart = timeInSeconds;
            consecutiveFrames = 1;
        }
        else {
            consecutiveFrames++;
        }
    }

    // Не забываем сохранить последнюю ноту, если запись оборвалась на ней
    if (currentMidiPitch != -1 && consecutiveFrames > 3) {
        NoteData note;
        note.midiPitch = currentMidiPitch;
        note.startTime = currentNoteStart;
        note.duration = static_cast<float>(numSamples / sampleRate) - currentNoteStart;
        float rms = buffer.getRMSLevel(0, 0, numSamples); // громкость текущего буфера
        note.velocity = juce::jlimit(0.0f, 1.0f, rms * 2.0f);
        detectedNotes.push_back(note);
    }

    DBG("PitchDetector: Анализ завершен. Найдено нот: " << detectedNotes.size());
    return detectedNotes;
}


std::vector<NoteData> PitchDetector::analyze(const juce::AudioBuffer<float>& buffer
                                             , double sampleRate) {
    if (buffer.getNumChannels() == 0) return {};
    int numSamples = buffer.getNumSamples();
    int fftSize = fft.getSize();

    if (numSamples < fftSize) return {};

    std::vector<NoteData> notes;
    const float* input = buffer.getReadPointer(0);

    // Подготовка данных + окно
    float* fftReal = fftData;
    for (int i = 0; i < fftSize; ++i) {
        fftReal[i] = (i < numSamples ? input[i] : 0.0f) * hannWindow.getSample(0, i);
    }

    // FFT (возвращает спектр)
    fft.performFrequencyOnlyForwardTransform(fftReal);

    // Поиск пика
    float freq = findPitchFreq(fftReal, fftSize, sampleRate);
    if (freq > 50.0f && freq < 2000.0f) {
        NoteData note;
        note.midiPitch = juce::jlimit(0, 127, freqToMidi(freq));
        note.startTime = 0.0f;
        note.duration = static_cast<float>(numSamples / sampleRate);
        float rms = buffer.getRMSLevel(0, 0, numSamples); // громкость текущего буфера
        note.velocity = juce::jlimit(0.0f, 1.0f, rms * 2.0f);
        notes.emplace_back(note);
    }

    return notes;
}


float PitchDetector::findPitchFreq(const float* spectrum, int fftSize, double sampleRate) const {
    float maxMag = 0.0f;
    int maxBin = 0;
    for (int i = 2; i < fftSize / 2; i++) { // fftSize / 2 потому что вторая часть fft симетрична
        if (spectrum[i] > maxMag) {
            maxMag = spectrum[i];
            maxBin = i;
        }
    }

    // Параболическая интерполяция
    if (maxBin > 1 && maxBin < fftSize / 2 - 1) {
        float a = spectrum[maxBin - 1];
        float b = spectrum[maxBin];
        float c = spectrum[maxBin + 1];
        float denom = a - 2.0f * b + c;
        if (std::abs(denom) > 1e-6f) {
            float delta = 0.5f * (a - c) / denom;
            return (maxBin + delta) * sampleRate / fftSize;
        }
    }
    return static_cast<float>(maxBin) * sampleRate / fftSize;
}

int PitchDetector::freqToMidi(float freq) const {
    return static_cast<int>(69.0f + 12.0f * std::log2f(freq / 440.0f) + 0.5f);
}
