#include "PlayheadOverlay.h"

PlayheadOverlay::PlayheadOverlay(AudioEngine& engineToUse)
    : engine(engineToUse) {
    setInterceptsMouseClicks(true, false);
    
    startTimerHz(60);
}

PlayheadOverlay::~PlayheadOverlay() {
    stopTimer();
}

void PlayheadOverlay::paint(juce::Graphics& g) {
    // 1. Рисуем фон линейки
    g.setColour(juce::Colour(0xFF252525)); // Темно-серый фон
    g.fillRect(0, 0, getWidth(), rulerHeight);
    
    // Нижняя граница линейки
    g.setColour(juce::Colours::black);
    g.drawHorizontalLine(rulerHeight, 0, getWidth());
    
    g.saveState();
    g.addTransform(juce::AffineTransform::translation(static_cast<float>(-xOffset), 0.0f));

    // --- Отрисовка музыкальной сетки (120 BPM, размер 4/4) ---
    double bpm = 120.0;
    double beatsPerSecond = bpm / 60.0; // 2 доли в секунду
    double pixelsPerBeat = pixelsPerSecond / beatsPerSecond; // Пикселей на одну долю
    
    int startBeat = static_cast<int>(xOffset / pixelsPerBeat);
    int endBeat = static_cast<int>((xOffset + getWidth()) / pixelsPerBeat) + 1;
    
    g.setFont(juce::Font(11.0f));
    
    for (int i = startBeat; i <= endBeat; ++i) {
        float x = static_cast<float>(i * pixelsPerBeat);
        bool isMeasure = (i % 4 == 0);
        
        if (isMeasure) {
            g.setColour(juce::Colours::white.withAlpha(0.6f));
            g.drawVerticalLine(juce::roundToInt(x), 0.0f, static_cast<float>(rulerHeight));
            int measureNumber = (i / 4) + 1;
            g.drawText(juce::String(measureNumber), juce::roundToInt(x) + 4, 2, 40, 14, juce::Justification::topLeft, false);
        } else {
            g.setColour(juce::Colours::white.withAlpha(0.2f));
            g.drawVerticalLine(juce::roundToInt(x), static_cast<float>(rulerHeight) - 6.0f, static_cast<float>(rulerHeight));
        }
    }

    // 2. Линия Playhead через весь Piano Roll
    juce::Colour playheadColor = juce::Colour(0xFF4A90E2); // Тот самый синий цвет
    g.setColour(playheadColor.withAlpha(0.8f));
    g.drawVerticalLine(juce::roundToInt(currentPositionX), 0.0f, static_cast<float>(getHeight()));
    
    // 3. Красивая каретка (флажок) на самой линейке
    juce::Path head;
    float px = currentPositionX;
    head.startNewSubPath(px - 6.0f, 0.0f);
    head.lineTo(px + 6.0f, 0.0f);
    head.lineTo(px + 6.0f, static_cast<float>(rulerHeight) - 6.0f);
    head.lineTo(px, static_cast<float>(rulerHeight));
    head.lineTo(px - 6.0f, static_cast<float>(rulerHeight) - 6.0f);
    head.closeSubPath();

    g.setColour(playheadColor);
    g.fillPath(head);
    
    // Легкая белая обводка для контраста
    g.setColour(juce::Colours::white.withAlpha(0.5f));
    g.strokePath(head, juce::PathStrokeType(1.0f));
    
    g.restoreState();
}

bool PlayheadOverlay::hitTest(int x, int y) {
    juce::ignoreUnused(x);
    return y <= rulerHeight;
}

void PlayheadOverlay::mouseDown(const juce::MouseEvent& event) {
    double timeInSeconds = (event.position.x + xOffset) / pixelsPerSecond;
    engine.setPosition(juce::jmax(0.0, timeInSeconds));
}

void PlayheadOverlay::mouseDrag(const juce::MouseEvent& event) {
    double timeInSeconds = (event.position.x + xOffset) / pixelsPerSecond;
    engine.setPosition(juce::jmax(0.0, timeInSeconds));
}

void PlayheadOverlay::setPixelsPerSecond(double newPixelsPerSecond) {
    if (newPixelsPerSecond > 0.0) {
        pixelsPerSecond = newPixelsPerSecond;
        repaint();
    }
}


void PlayheadOverlay::timerCallback() {
    // 1. Берем текущее время из движка
    double timeInSeconds = engine.getCurrentPosition();
    float newPositionX = static_cast<float>(timeInSeconds * pixelsPerSecond);
    
    // 2. Оптимизация: перерисовываем только если позиция реально изменилась
    if (std::abs(currentPositionX - newPositionX) > 0.5f) {
        currentPositionX = newPositionX;
        repaint();
    }
}


void PlayheadOverlay::setXOffset(int newOffset) {
    if (xOffset != newOffset) {
        xOffset = newOffset;
        repaint();
    }
}
