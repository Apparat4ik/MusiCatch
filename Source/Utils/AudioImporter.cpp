#include "AudioImporter.h"

std::unique_ptr<juce::AudioFormatReaderSource> AudioImporter::loadAudioFile(
    const juce::File& file,
    juce::AudioFormatManager& formatManager) {
    if (!file.existsAsFile()) {
        DBG("AudioImporter::loadAudioFile - Файла не существует: " + file.getFullPathName());
        return nullptr;
    }

    auto* reader = formatManager.createReaderFor(file);
    
    if (reader == nullptr) {
        DBG("AudioImporter::loadAudioFile - Неподдерживаемый формат файла: " + file.getFullPathName());
        return nullptr;
    }

    // Обертываем reader в AudioFormatReaderSource.
    return std::make_unique<juce::AudioFormatReaderSource>(reader, true);
}

bool AudioImporter::loadIntoBuffer(
    const juce::File& file,
    juce::AudioFormatManager& formatManager,
    juce::AudioBuffer<float>& targetBuffer,
    double& outSampleRate) {
    
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    
    if (reader == nullptr) {
        DBG("AudioImporter::loadIntoBuffer - Ошибка загрузки файла: " + file.getFullPathName());
        return false;
    }

    outSampleRate = reader->sampleRate;
    auto lengthInSamples = static_cast<int>(reader->lengthInSamples);
    

    targetBuffer.setSize(reader->numChannels, lengthInSamples);
    

    reader->read(&targetBuffer, 0, lengthInSamples, 0, true, true);
    
    DBG("AudioImporter: Файл загружен. Длина: " << targetBuffer.getNumSamples() << " сэмплов.");
    return true;
}
