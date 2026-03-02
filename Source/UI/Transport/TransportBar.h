#pragma once
#include <JuceHeader.h>
#include "../../Audio/AudioEngine.h"

class TransportBar : public juce::Component {
 public:
    // Внедряем зависимость: передаем ссылку на движок
    explicit TransportBar(AudioEngine& engineToUse);
    ~TransportBar() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

 private:
    AudioEngine& engine;

    // UI элементы
    juce::TextButton loadButton   { juce::String::fromUTF8("⏏") }; // Load
    juce::TextButton saveButton   { juce::String::fromUTF8("⤓") }; // Save
    juce::TextButton playButton   { juce::String::fromUTF8("▶") };
    juce::TextButton pauseButton  { juce::String::fromUTF8("⏸") };
    juce::TextButton stopButton   { juce::String::fromUTF8("■") };
    juce::TextButton recordButton { juce::String::fromUTF8("⏺") };
    
    std::unique_ptr<juce::FileChooser> fileChooser;
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportBar)
};
