#pragma once
#include <JuceHeader.h>
#include "PianoRollComponent.h"

class PianoKeyboardComponent : public juce::Component {
 public:
    explicit PianoKeyboardComponent(PianoRollComponent& pr);
    ~PianoKeyboardComponent() override = default;

    void paint(juce::Graphics& g) override;

 private:
    PianoRollComponent& pianoRoll;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoKeyboardComponent)
};
