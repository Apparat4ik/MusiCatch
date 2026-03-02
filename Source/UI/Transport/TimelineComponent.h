#pragma once

#include <JuceHeader.h>
#include "../../Audio/AudioEngine.h"

class TimelineComponent : public juce::Component {
public:
    explicit TimelineComponent(AudioEngine& engineToUse);
    ~TimelineComponent() override = default;

    void paint(juce::Graphics& g) override;
    
    // Обработка перемотки кликом и перетаскиванием
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;

    // Синхронизация масштаба и прокрутки с PianoRoll
    void setPixelsPerSecond(double newPixelsPerSecond);
    void setXOffset(int newOffset);

private:
    AudioEngine& engine;
    
    int xOffset = 0;
    double pixelsPerSecond = 100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimelineComponent)
};
