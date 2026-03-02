#pragma once
#include <JuceHeader.h>

class RecordingBuffer {
 public:
    RecordingBuffer(int numChannels, int maxSamples = 10 * 60 * 44100);

    void prepareToRecord(double sampleRate);
    void pushBlock(const juce::AudioBuffer<float>& buffer, int startSample, int numSamples);
    void clear();
    int getTotalSamples() const;
    
    juce::AudioBuffer<float> getRecordedData();

private:
    juce::AbstractFifo recordingFifo;
    juce::AudioBuffer<float> fifoData;
    double sampleRate = 44100.0;
    int maxSamples;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RecordingBuffer)
};
