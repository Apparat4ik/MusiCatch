#pragma once
#include <JuceHeader.h>
#include <memory>

class AudioImporter {
public:
    // Если формат не поддерживается или файл поврежден, возвращает nullptr.
    // Важно: deleteReaderWhenThisIsDeleted установлено в true, поэтому
    // возвращаемый объект сам удалит reader при уничтожении.
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
    // Приватный конструктор, так как класс содержит только статические методы
    AudioImporter() = delete;
};
