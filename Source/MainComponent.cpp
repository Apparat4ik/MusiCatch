#include "MainComponent.h"

MainComponent::MainComponent()
    : appState(AppState::getInstance()),
      transportBar(audioEngine),         // Передаем ссылку на движок
      pianoRoll(appState, audioEngine) { // Передаем ссылки на состояние и движок

    // 1. Применяем глобальный стиль ко всем дочерним компонентам
    setLookAndFeel(&customLookAndFeel);

    // 2. Добавляем и делаем видимыми основные панели
    addAndMakeVisible(transportBar);
    addAndMakeVisible(trackList);

    // 3. Настраиваем Viewport для PianoRoll
    // false = Viewport не берет на себя владение памятью pianoRoll (не сделает delete)
    pianoRollViewport.setViewedComponent(&pianoRoll, false);
    pianoRollViewport.setScrollBarsShown(true, true); // Включаем вертикальный и горизонтальный скролл
    addAndMakeVisible(pianoRollViewport);
          
    addAndMakeVisible(playhead);
    pianoRollViewport.getVerticalScrollBar().addListener(this);
    pianoRollViewport.getHorizontalScrollBar().addListener(this);
          
    keyboardViewport.setViewedComponent(&pianoKeyboard, false);
    keyboardViewport.setScrollBarsShown(false, false); // Прячем скроллбары клавиатуры
    addAndMakeVisible(keyboardViewport);

    pianoRollViewport.getVerticalScrollBar().addListener(this);

    // 4. Демо-инициализация (создаем первый трек, если проект пуст)
    if (appState.getTrackList().getNumChildren() == 0) {
        appState.addTrack("Vocal Track");
        appState.addTrack("Guitar Track");
        
        // TODO: В будущем здесь нужно будет связать добавление трека в AppState
        // с созданием TrackProcessor внутри AudioEngine.
        audioEngine.addTrack();
        audioEngine.addTrack();
    }

    // Устанавливаем дефолтный размер окна приложения
    setSize(1000, 700);
}

MainComponent::~MainComponent() {
    setLookAndFeel(nullptr);
    pianoRollViewport.getVerticalScrollBar().removeListener(this);
    pianoRollViewport.getHorizontalScrollBar().removeListener(this);
}

void MainComponent::paint(juce::Graphics& g) {
    // Заливаем фон самым темным цветом (если между компонентами будут щели)
    g.fillAll(juce::Colour(0xFF151515));
}

void MainComponent::resized() {
    auto bounds = getLocalBounds();
    transportBar.setBounds(bounds.removeFromTop(60));
    bounds.removeFromTop(1);

    trackList.setBounds(bounds.removeFromLeft(250));
    bounds.removeFromLeft(1);
    
    int keyboardWidth = 50;
    int rulerHeight = 30;
    
    auto keyboardArea = bounds.removeFromLeft(keyboardWidth);
    keyboardArea.removeFromTop(rulerHeight);
    keyboardViewport.setBounds(keyboardArea);
    
    bounds.removeFromLeft(1);
    
    playhead.setBounds(bounds);

    auto gridArea = bounds;
    gridArea.removeFromTop(rulerHeight);
    pianoRollViewport.setBounds(gridArea);

    int pianoRollWidth = juce::jmax(bounds.getWidth(), 3000);
    int numNotes = pianoRoll.getMaxMidiNote() - pianoRoll.getMinMidiNote() + 1;
    int pianoRollHeight = static_cast<int>(numNotes * pianoRoll.getNoteHeight());

    pianoRoll.setBounds(0, 0, pianoRollWidth, pianoRollHeight);
    pianoKeyboard.setBounds(0, 0, keyboardWidth, pianoRollHeight);
}

void MainComponent::scrollBarMoved(juce::ScrollBar* scrollBarThatHasMoved, double newRangeStart) {
    if (scrollBarThatHasMoved == &pianoRollViewport.getVerticalScrollBar()) {
        keyboardViewport.setViewPosition(0, static_cast<int>(newRangeStart));
    }
    else if (scrollBarThatHasMoved == &pianoRollViewport.getHorizontalScrollBar()) {
        // Передаем горизонтальный скролл в PlayheadOverlay
        playhead.setXOffset(static_cast<int>(newRangeStart));
    }
}
