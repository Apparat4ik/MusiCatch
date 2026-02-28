#pragma once

#include <JuceHeader.h>

// Core & Audio
#include "Core/AppState.h"
#include "Audio/AudioEngine.h"

// UI
#include "UI/Styles/CustomLookAndFeel.h"
#include "UI/Transport/TransportBar.h"
#include "UI/Tracks/TrackListComponent.h"
#include "UI/PianoRoll/PianoRollComponent.h"
#include "UI/PianoRoll/PianoKeyboardComponent.h"
#include "UI/PianoRoll/PlayheadOverlay.h"

/**
 * Главный компонент приложения (Hub).
 * Владеет основными UI-модулями и координирует внедрение зависимостей (DI).
 */
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

    // --- Backend & State ---
    // Получаем ссылку на Singleton-состояние приложения
    AppState& appState;
    
    // Главный аудио-движок (он сам является AudioAppComponent и управляет потоком)
    AudioEngine audioEngine;

    // --- UI Components ---
    TransportBar transportBar;
    TrackListComponent trackList;
    
    PianoRollComponent pianoRoll;
    juce::Viewport pianoRollViewport; // Контейнер для скроллинга PianoRoll
    PlayheadOverlay playhead { audioEngine };
                          
    void scrollBarMoved(juce::ScrollBar* scrollBarThatHasMoved, double newRangeStart) override;

    PianoKeyboardComponent pianoKeyboard {pianoRoll};
    juce::Viewport keyboardViewport;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
