#include "AudioEngine.h"


AudioEngine::AudioEngine() {
    formatManager.registerBasicFormats();

    // Цепочка: Микшер треков вставляется в Транспорт.
    transportSource.setSource(&trackMixer, 0, nullptr, 44100.0); // Последний параметр
                                                                 // для корректировки
                                                                 // частоты дискретизации

    // Для безопасного вывода в систему
    resampler = std::make_unique<juce::ResamplingAudioSource>(&transportSource, false);

    setAudioChannels(2, 2); // Инициализация аудио (2 in, 2 out)
    startTimerHz(30);
}

AudioEngine::~AudioEngine() {
    stopTimer();
    cancelPendingUpdate();
    
    shutdownAudio();
    trackMixer.removeAllTracks();
    trackProcessors.clear();
}


void AudioEngine::handleAsyncUpdate() {}


void AudioEngine::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlockExpected; // Сохраняем размер блока
    DBG("Инициализация аудио. Буфер: " << samplesPerBlockExpected << " сэмплов.");
    
    recordingBuffer.prepareToRecord(sampleRate);
    
    trackMixer.prepareToPlay(samplesPerBlockExpected, sampleRate);
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    
    if (resampler != nullptr)
        resampler->prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void AudioEngine::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) {
    // В буфере содержится аудио с входных каналов
    // забираем эти данные до того, как микшер и ресемплер их перезапишут
    if (recording.load()) {
        if (bufferToFill.buffer->getNumChannels() > 0) {
            recordingBuffer.pushBlock(*bufferToFill.buffer, bufferToFill.startSample, bufferToFill.numSamples);
        }
    }
    juce::ScopedNoDenormals noDenormals; // Защита от денормализованных чисел
    
    if (resampler != nullptr)
        resampler->getNextAudioBlock(bufferToFill);
    else
        bufferToFill.clearActiveBufferRegion();
}

void AudioEngine::releaseResources() {
    if (resampler != nullptr)
        resampler->releaseResources();
        
    transportSource.releaseResources();
    trackMixer.releaseResources();
}


void AudioEngine::timerCallback() {
    triggerAsyncUpdate(); // Безопасный вызов обновления UI из потока таймера
}

TrackProcessor* AudioEngine::addTrack() {
    auto* newTrack = trackProcessors.add(new TrackProcessor());
    
    if (currentSampleRate > 0.0)
        newTrack->prepareToPlay(512, currentSampleRate); // Безопасный дефолт блока
        
    trackMixer.addTrack(newTrack);
    return newTrack;
}

void AudioEngine::removeTrack(TrackProcessor* trackToRemove) {
    trackMixer.removeTrack(trackToRemove);
    trackProcessors.removeObject(trackToRemove);
}

void AudioEngine::loadFileIntoTrack(TrackProcessor* track, const juce::File& file) {
    if (track == nullptr) return;

    auto* reader = formatManager.createReaderFor(file);
    if (reader != nullptr) {
        // Создаем Positionable Source (true = source сам удалит reader)
        auto fileSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
        
        // Передаем владение источнику в TrackProcessor
        track->setSource(std::move(fileSource));
        
        DBG("Loaded into track: " << file.getFullPathName() << " (" << reader->sampleRate << "Hz)");
    }
    else {
        DBG("Failed to load: " << file.getFullPathName());
    }
}

void AudioEngine::setRecording(bool shouldRecord) {
    if (recording.load() == shouldRecord) return;

    if (shouldRecord) {
        recordingBuffer.clear();
        recording.store(true);
        DBG("Запись начата (SampleRate: " << currentSampleRate << ")");
    } else {
        recording.store(false);
        juce::Thread::sleep(20); // Микропауза для завершения аудиопотока

        int totalSamples = recordingBuffer.getTotalSamples();
        if (totalSamples > 0) {
            juce::AudioBuffer<float> finalBuffer = recordingBuffer.getRecordedData();
            
            PitchDetector detector(11); // 11 = 2048 точек для чуть лучшей точности на низах
            std::vector<NoteData> detectedNotes = detector.analyzeTrack(finalBuffer, currentSampleRate);

            auto memorySource = std::make_unique<juce::MemoryAudioSource>(finalBuffer, true, false);
            
            auto* newTrack = addTrack();
            newTrack->setSource(std::move(memorySource));
            
            juce::MessageManager::callAsync([notes = std::move(detectedNotes)]() {
                auto& appState = AppState::getInstance();
                appState.addTrack("Vocal Take");
                
                // Получаем свежесозданный узел трека (он последний в списке)
                auto trackList = appState.getTrackList();
                auto newTrackNode = trackList.getChild(trackList.getNumChildren() - 1);
                
                // Оборачиваем его в TrackModel и накидываем ноты
                TrackModel model(newTrackNode);
                for (const auto& note : notes) {
                    model.addNote(note);
                }
                
                // Делаем этот трек активным, чтобы PianoRoll мгновенно переключился на него
                appState.getRootNode().setProperty("selectedTrackId", model.getUuid().toString(), nullptr);
            });
        }
    }
}

void AudioEngine::loadAndAnalyzeFile(const juce::File& file) {
    if (!file.existsAsFile()) return;

    juce::AudioBuffer<float> analysisBuffer;
    double fileSampleRate = 0.0;
    
    if (!AudioImporter::loadIntoBuffer(file, formatManager, analysisBuffer, fileSampleRate)) {
        DBG("Не удалось загрузить файл в буфер для анализа");
        return;
    }

    PitchDetector detector(11);
    std::vector<NoteData> detectedNotes = detector.analyzeTrack(analysisBuffer, fileSampleRate);

    // Создаем источник воспроизведения (стриминг с диска)
    auto fileSource = AudioImporter::loadAudioFile(file, formatManager);
    if (fileSource == nullptr) return;

    // Создаем трек и привязываем к нему файл
    auto* newTrack = addTrack();
    newTrack->setSource(std::move(fileSource));

    // Отправляем данные в интерфейс
    juce::String trackName = file.getFileNameWithoutExtension();
    
    juce::MessageManager::callAsync([notes = std::move(detectedNotes), trackName]() {
        auto& appState = AppState::getInstance();
        appState.addTrack(trackName);
        
        // Получаем свежесозданный узел трека
        auto trackList = appState.getTrackList();
        auto newTrackNode = trackList.getChild(trackList.getNumChildren() - 1);
        
        TrackModel model(newTrackNode);
        for (const auto& note : notes) {
            model.addNote(note);
        }
        
        // Делаем трек активным
        appState.getRootNode().setProperty("selectedTrackId", model.getUuid().toString(), nullptr);
    });
}
