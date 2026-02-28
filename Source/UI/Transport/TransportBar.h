#pragma once
#include <JuceHeader.h>
#include "../../Audio/AudioEngine.h"

class TransportBar : public juce::Component,
                     private juce::Timer {
 public:
    // Внедряем зависимость: передаем ссылку на движок
    explicit TransportBar(AudioEngine& engineToUse);
    ~TransportBar() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

 private:
    void timerCallback() override;

    AudioEngine& engine;

    // UI элементы
    juce::TextButton loadButton { "Load" };
    juce::TextButton playButton   { "Play" };
    juce::TextButton stopButton   { "Stop" };
    juce::TextButton pauseButton  { "Pause" };
    juce::TextButton recordButton { "Record" };
    
    std::unique_ptr<juce::FileChooser> fileChooser;
    
    juce::Label timecodeLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportBar)
};
