#include "MainComponent.h"

MainComponent::MainComponent()
    : appState(AppState::getInstance()),
      transportBar(audioEngine),
      pianoRoll(appState, audioEngine) {
    // 1. Применяем глобальный стиль ко всем дочерним компонентам
    setLookAndFeel(&customLookAndFeel);

    // 2. Настраиваем Viewport'ы (контейнеры для прокрутки)
    pianoRollViewport.setViewedComponent(&pianoRoll, false);
    pianoRollViewport.setScrollBarsShown(true, true);
    
    keyboardViewport.setViewedComponent(&pianoKeyboard, false);
    keyboardViewport.setScrollBarsShown(false, false);

    // 3. Добавляем основные панели на экран
    addAndMakeVisible(transportBar);
    addAndMakeVisible(timeline);
    addAndMakeVisible(trackList);
    addAndMakeVisible(keyboardViewport);
    addAndMakeVisible(pianoRollViewport);

    // 4. Добавляем Playhead последним
    // В JUCE компонент, который добавляется позже всех, отрисовывается на самом верхнем слое
    // Линия воспроизведения должна перекрывать и PianoRoll, и Timeline.
    addAndMakeVisible(playhead);

    // 5. Подписываемся на события скроллинга (чтобы синхронизировать клавиатуру и таймлайн)
    pianoRollViewport.getVerticalScrollBar().addListener(this);
    pianoRollViewport.getHorizontalScrollBar().addListener(this);

    if (appState.getTrackList().getNumChildren() == 0) {
        appState.addTrack("Vocal Track");
        appState.addTrack("Guitar Track");
        
        audioEngine.addTrack();
        audioEngine.addTrack();
    }

    // 7. Устанавливаем дефолтный размер окна приложения
    setSize(1000, 700);
}

MainComponent::~MainComponent() {
    setLookAndFeel(nullptr);
    pianoRollViewport.getVerticalScrollBar().removeListener(this);
    pianoRollViewport.getHorizontalScrollBar().removeListener(this);
    appState.getRootNode().removeAllChildren(nullptr);
}

void MainComponent::paint(juce::Graphics& g) {
    // Заливаем фон самым темным цветом (если между компонентами будут щели)
    g.fillAll(juce::Colour(0xFF151515));
}

void MainComponent::resized() {
    auto bounds = getLocalBounds();

    // верхняя панель
    auto topBarArea = bounds.removeFromTop(85);
    bounds.removeFromTop(1); // Разделительная линия

    // Левая часть: Транспорт (250px, совпадает с шириной списка треков)
    transportBar.setBounds(topBarArea.removeFromLeft(250));
    topBarArea.removeFromLeft(1);
    
    // Отступ над пианино-клавиатурой (пустое место)
    int keyboardWidth = 50;
    topBarArea.removeFromLeft(keyboardWidth);
    topBarArea.removeFromLeft(1);

    // Правая часть: Таймлайн (всё оставшееся место)
    timeline.setBounds(topBarArea);

    // Нижняя панель (Треки и Сетка)
    trackList.setBounds(bounds.removeFromLeft(250));
    bounds.removeFromLeft(1);
    
    // Клавиатура
    keyboardViewport.setBounds(bounds.removeFromLeft(keyboardWidth));
    bounds.removeFromLeft(1);
    
    // Сетка Piano Roll
    pianoRollViewport.setBounds(bounds);

    // настройка внутренних размеров Piano Roll
    int pianoRollWidth = juce::jmax(bounds.getWidth(), 3000);
    int numNotes = pianoRoll.getMaxMidiNote() - pianoRoll.getMinMidiNote() + 1;
    int pianoRollHeight = static_cast<int>(numNotes * pianoRoll.getNoteHeight());

    pianoRoll.setBounds(0, 0, pianoRollWidth, pianoRollHeight);
    pianoKeyboard.setBounds(0, 0, keyboardWidth, pianoRollHeight);

    // PLAYHEAD OVERLAY
    // Playhead должен перекрывать И Таймлайн, И Piano Roll по ширине.
    // Задаем ему координаты так, чтобы он начинался там же, где Timeline и PianoRoll
    auto playheadBounds = getLocalBounds();
    playheadBounds.removeFromLeft(250 + 1 + keyboardWidth + 1); // Пропускаем левое меню
    playhead.setBounds(playheadBounds);
}


void MainComponent::scrollBarMoved(juce::ScrollBar* scrollBarThatHasMoved, double newRangeStart) {
    if (scrollBarThatHasMoved == &pianoRollViewport.getVerticalScrollBar()) {
        keyboardViewport.setViewPosition(0, static_cast<int>(newRangeStart));
    }
    else if (scrollBarThatHasMoved == &pianoRollViewport.getHorizontalScrollBar()) {
        int newOffset = static_cast<int>(newRangeStart);
        playhead.setXOffset(newOffset);
        timeline.setXOffset(newOffset);
    }
}
