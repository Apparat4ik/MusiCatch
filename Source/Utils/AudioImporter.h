#pragma once
#include <JuceHeader.h>
#include <memory>

class AudioImporter {
 public:

    static std::unique_ptr<juce::AudioFormatReaderSource> loadAudioFile(
        const juce::File& file,
        juce::AudioFormatManager& formatManager);

    // Вспомогательный метод для прямой загрузки в AudioBuffer
    static bool loadIntoBuffer(
        const juce::File& file,
        juce::AudioFormatManager& formatManager,
        juce::AudioBuffer<float>& targetBuffer,
        double& outSampleRate);
    
 private:
    AudioImporter() = delete;
};
