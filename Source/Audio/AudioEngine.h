#pragma once

#include <JuceHeader.h>
#include "TrackProcessor.h"
#include "RecordingBuffer.h"
#include "Analysis/PitchDetector.h"
#include "../Model/TrackModel.h"
#include "../Core/AppState.h"
#include "../Utils/AudioImporter.h"

// Вспомогательный класс для микширования нескольких PositionableAudioSource
class PositionableMixerSource : public juce::PositionableAudioSource {
 public:
    void addTrack(TrackProcessor* tp) {
        mixer.addInputSource(tp, false);
        tracks.add(tp);
    }
    
    void removeTrack(TrackProcessor* tp) {
        mixer.removeInputSource(tp);
        tracks.removeAllInstancesOf(tp);
    }
    
    void removeAllTracks() {
        mixer.removeAllInputs();
        tracks.clear();
    }

    // --- AudioSource методы ---
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override {
        mixer.prepareToPlay(samplesPerBlockExpected, sampleRate);
    }
    
    void releaseResources() override { mixer.releaseResources(); }
    
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override {
        mixer.getNextAudioBlock(bufferToFill);
    }

    // --- PositionableAudioSource методы ---
    void setNextReadPosition(juce::int64 newPosition) override {
        for (auto* t : tracks) t->setNextReadPosition(newPosition);
    }
    
    juce::int64 getNextReadPosition() const override {
        return tracks.isEmpty() ? 0 : tracks.getFirst()->getNextReadPosition();
    }
    
    juce::int64 getTotalLength() const override {
        juce::int64 maxLen = 0;
        for (auto* t : tracks)
            maxLen = juce::jmax(maxLen, t->getTotalLength());
        return maxLen;
    }
    
    bool isLooping() const override { return false; }
    void setLooping(bool) override {}

 private:
    juce::MixerAudioSource mixer;
    juce::Array<TrackProcessor*> tracks;
};

// --- Основной движок ---
class AudioEngine : public juce::AudioAppComponent,
                    private juce::Timer,
                    private juce::AsyncUpdater {
 public:
    AudioEngine();
    ~AudioEngine() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    void handleAsyncUpdate() override;

    // Управление транспортом (глобально для всех треков)
    void play() { transportSource.start(); }
    void stop() { transportSource.stop(); }
    void rewind() { transportSource.setPosition(0.0); }
    bool isPlaying() const { return transportSource.isPlaying(); }
    double getCurrentPosition() const { return transportSource.getCurrentPosition(); }
    void setPosition(double pos) { transportSource.setPosition(pos); }

    // Управление треками
    // Возвращает указатель на созданный процессор, чтобы UI мог к нему привязаться (Mute/Solo)
    TrackProcessor* addTrack();
    void removeTrack(TrackProcessor* trackToRemove);
    void loadFileIntoTrack(TrackProcessor* track, const juce::File& file);

    juce::AudioDeviceManager& getDeviceManager() { return deviceManager; }
                        
    void setRecording(bool isRecording);
    bool isRecording() const { return recording.load(); }
    
    void loadAndAnalyzeFile(const juce::File& file);

 private:
    void timerCallback() override;

    juce::AudioDeviceManager deviceManager;
    juce::AudioFormatManager formatManager;
                        
    std::atomic<bool> recording { false };
    RecordingBuffer recordingBuffer { 1, 10 * 60 * 44100 };
    int currentBlockSize = 512;

    // Хранилище всех треков
    juce::OwnedArray<TrackProcessor> trackProcessors;

    // Аудио-цепь: TrackProcessors -> PositionableMixer -> Transport -> Resampler(Опционально) -> Device
    PositionableMixerSource trackMixer;
    juce::AudioTransportSource transportSource;
    std::unique_ptr<juce::ResamplingAudioSource> resampler;
    
    double currentSampleRate = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioEngine)
};
