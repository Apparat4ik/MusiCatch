#include "AudioImporter.h"

std::unique_ptr<juce::AudioFormatReaderSource> AudioImporter::loadAudioFile(
    const juce::File& file,
    juce::AudioFormatManager& formatManager)
{
    if (!file.existsAsFile())
    {
        DBG("AudioImporter::loadAudioFile - File does not exist: " + file.getFullPathName());
        return nullptr;
    }

    // formatManager.createReaderFor создает новый экземпляр AudioFormatReader.
    // Вызывающий код (или класс-обертка) берет на себя ответственность за его удаление [web:41][web:45].
    auto* reader = formatManager.createReaderFor(file);
    
    if (reader == nullptr)
    {
        DBG("AudioImporter::loadAudioFile - Unsupported format or corrupted file: " + file.getFullPathName());
        return nullptr;
    }

    // Обертываем reader в AudioFormatReaderSource.
    // Второй аргумент (true) указывает, что Source возьмет на себя владение reader-ом
    // и удалит его при своем уничтожении, предотвращая утечки памяти
    return std::make_unique<juce::AudioFormatReaderSource>(reader, true);
}

bool AudioImporter::loadIntoBuffer(
    const juce::File& file,
    juce::AudioFormatManager& formatManager,
    juce::AudioBuffer<float>& targetBuffer,
    double& outSampleRate)
{
    // Оборачиваем голый указатель в std::unique_ptr для автоматического удаления
    // в случае досрочного выхода из функции
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    
    if (reader == nullptr)
    {
        DBG("AudioImporter::loadIntoBuffer - Failed to load file: " + file.getFullPathName());
        return false;
    }

    outSampleRate = reader->sampleRate;
    auto lengthInSamples = static_cast<int>(reader->lengthInSamples);
    
    // Подготавливаем буфер нужного размера
    targetBuffer.setSize(reader->numChannels, lengthInSamples);
    
    // Считываем данные напрямую в буфер
    reader->read(&targetBuffer, 0, lengthInSamples, 0, true, true);
    
    return true;
}
