#pragma once
#include <JuceHeader.h>

class RecordingBuffer
{
public:
    // Убрали параметр bufferSize, он больше не нужен
    RecordingBuffer(int numChannels, int maxSamples = 10 * 60 * 44100);

    void prepareToRecord(double sampleRate);
    
    // Передаем исходный буфер, стартовый сэмпл и количество сэмплов
    void pushBlock(const juce::AudioBuffer<float>& buffer, int startSample, int numSamples);
    
    void clear();

    int getTotalSamples() const;
    
    // Удобный метод: отдает все записанные данные одним куском!
    juce::AudioBuffer<float> getRecordedData();

private:
    juce::AbstractFifo recordingFifo;
    juce::AudioBuffer<float> fifoData;
    double sampleRate = 44100.0;
    int maxSamples;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RecordingBuffer)
};
