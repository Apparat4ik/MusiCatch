#pragma once
#include <JuceHeader.h>
#include <vector>
#include "../../Model/NoteData.h"

class PitchDetector
{
public:
    PitchDetector(int fftOrder = 10);  // 1024 точек
    ~PitchDetector();

    std::vector<NoteData> analyze(const juce::AudioBuffer<float>& buffer, double sampleRate);
    
    std::vector<NoteData> analyzeTrack(const juce::AudioBuffer<float>& buffer, double sampleRate);

private:
    juce::dsp::FFT fft;
    juce::AudioBuffer<float> hannWindow;
    juce::HeapBlock<float> fftData;  // float* для legacy FFT (magnitude)

    float findPitchFreq(const float* spectrum, int fftSize, double sampleRate) const;
    int freqToMidi(float freq) const;
};
