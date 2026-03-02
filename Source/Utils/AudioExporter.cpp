#include "AudioExporter.h"

bool AudioExporter::exportProjectToWav(AudioEngine& engine, const juce::File& targetFile) {
    auto& mixer = engine.getTrackMixer();
    
    // Получаем длину самого длинного трека
    juce::int64 totalLengthInSamples = mixer.getTotalLength();
    if (totalLengthInSamples <= 0) {
        DBG("Нет данных для экспорта.");
        return false;
    }

    double sampleRate = engine.getCurrentSampleRate();
    if (sampleRate <= 0) sampleRate = 44100.0;

    juce::WavAudioFormat wavFormat;
    

    std::unique_ptr<juce::AudioFormatWriter> writer(
        wavFormat.createWriterFor(new juce::FileOutputStream(targetFile),
                                  sampleRate,
                                  2,  // Стерео
                                  16, // Битность
                                  {},
                                  0) // Quality (0 = best)
    );

    if (writer == nullptr) {
        DBG("Ошибка: не удалось создать AudioFormatWriter для MP3. Проверьте LAME энкодер.");
        return false;
    }


    // Временно останавливаем транспорт, чтобы избежать гонки данных
    bool wasPlaying = engine.isPlaying();
    if (wasPlaying) {
        engine.stop();
    }

    // Сохраняем текущую позицию, чтобы вернуть её после экспорта
    juce::int64 originalPosition = mixer.getNextReadPosition();
    mixer.setNextReadPosition(0);

    // Подготавливаем буфер для покадрового чтения (размер блока 2048 сэмплов)
    const int blockSize = 2048;
    juce::AudioBuffer<float> renderBuffer(2, blockSize);
    juce::AudioSourceChannelInfo channelInfo(&renderBuffer, 0, blockSize);

    juce::int64 samplesProcessed = 0;

    while (samplesProcessed < totalLengthInSamples) {
        int samplesToProcess = static_cast<int>(juce::jmin(static_cast<juce::int64>(blockSize),
                                                           totalLengthInSamples - samplesProcessed));
        channelInfo.numSamples = samplesToProcess;
        renderBuffer.clear();

        // Просим микшер отрендерить блок
        mixer.getNextAudioBlock(channelInfo);

        // Записываем блок в файл
        writer->writeFromAudioSampleBuffer(renderBuffer, 0, samplesToProcess);

        samplesProcessed += samplesToProcess;
    }

    // Возвращаем микшер в исходное состояние
    mixer.setNextReadPosition(originalPosition);
    if (wasPlaying) engine.play();

    DBG("Экспорт успешно завершен: " << targetFile.getFullPathName());
    return true;
}


bool AudioExporter::exportTrackToMidi(const TrackModel& track, const juce::File& targetFile) {
    juce::MidiMessageSequence sequence;

    // Стандартное разрешение MIDI: 960 тиков на четвертную ноту
    short ticksPerQuarterNote = 960;
    
    // При темпе 120 BPM (2 четвертные ноты в секунду) получаем:
    const double ticksPerSecond = ticksPerQuarterNote * 2.0;

    // Получаем массив нот из трека
    auto notes = track.getNoteList();

    if (notes.isEmpty()) {
        DBG("Трек пуст, нет нот для экспорта в MIDI.");
        return false;
    }

    // Проходим по всем распознанным нотам и создаем MIDI-события
    for (const auto& note : notes) {
        // Конвертируем секунды в тики
        auto startTick = static_cast<double>(note.startTime * ticksPerSecond);
        auto endTick = static_cast<double>((note.startTime + note.duration) * ticksPerSecond);

        // Событие Note On
        // Канал 1, высота тона (0-127), velocity (0.0 - 1.0)
        auto noteOn = juce::MidiMessage::noteOn(1, note.midiPitch, note.velocity);
        noteOn.setTimeStamp(startTick);

        // Событие Note Off
        auto noteOff = juce::MidiMessage::noteOff(1, note.midiPitch, 0.0f);
        noteOff.setTimeStamp(endTick);

        sequence.addEvent(noteOn);
        sequence.addEvent(noteOff);
    }

    sequence.updateMatchedPairs();

    // Создаем объект MIDI-файла и записываем в него нашу последовательность
    juce::MidiFile midiFile;
    midiFile.setTicksPerQuarterNote(ticksPerQuarterNote);
    midiFile.addTrack(sequence);

    // Записываем файл на диск
    juce::FileOutputStream stream(targetFile);
    if (stream.openedOk()) {
        stream.setPosition(0);
        stream.truncate(); // Очищаем файл, если он уже существовал
        
        bool success = midiFile.writeTo(stream);
        if (success) {
            DBG("MIDI успешно сохранен: " << targetFile.getFullPathName());
        }
        return success;
    }

    DBG("Ошибка: не удалось открыть файл для записи MIDI.");
    return false;
}
