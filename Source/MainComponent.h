#pragma once

#include <JuceHeader.h>

#include "Core/AppState.h"
#include "Audio/AudioEngine.h"

#include "UI/Styles/CustomLookAndFeel.h"
#include "UI/Transport/TransportBar.h"
#include "UI/Tracks/TrackListComponent.h"
#include "UI/PianoRoll/PianoRollComponent.h"
#include "UI/PianoRoll/PianoKeyboardComponent.h"
#include "UI/PianoRoll/PlayheadOverlay.h"
#include "UI/Transport/TimelineComponent.h"


class MainComponent : public juce::Component,
                      private juce::ScrollBar::Listener {
 public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

 private:
    // Кастомный стиль (LookAndFeel) должен быть объявлен до UI-компонентов,
    // чтобы при разрушении (деструкторе) он удалялся последним.
    CustomLookAndFeel customLookAndFeel;
                          
    AppState& appState;
    AudioEngine audioEngine;

    // UI
    TransportBar transportBar;
    TrackListComponent trackList;
    
    PianoRollComponent pianoRoll;
    juce::Viewport pianoRollViewport; // Контейнер для скроллинга PianoRoll
    PlayheadOverlay playhead { audioEngine };
                          
    void scrollBarMoved(juce::ScrollBar* scrollBarThatHasMoved, double newRangeStart) override;

    PianoKeyboardComponent pianoKeyboard {pianoRoll};
    TimelineComponent timeline { audioEngine };
    juce::Viewport keyboardViewport;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
