#include "PlayheadOverlay.h"

PlayheadOverlay::PlayheadOverlay(AudioEngine& engineToUse)
    : engine(engineToUse) {
    // Пропускаем клики сквозь ползунок, чтобы они доходили до Таймлайна и PianoRoll
    setInterceptsMouseClicks(false, false);
    
    // ВАЖНО: Запускаем таймер перерисовки (60 кадров в секунду)
    startTimerHz(60);
}

PlayheadOverlay::~PlayheadOverlay() {
    stopTimer();
}

void PlayheadOverlay::paint(juce::Graphics& g) {
    g.saveState();
    
    // Сдвигаем координаты рисунка в зависимости от прокрутки скроллбара
    g.addTransform(juce::AffineTransform::translation(static_cast<float>(-xOffset), 0.0f));

    juce::Colour playheadColor = juce::Colour(0xFF4A90E2);
    float px = currentPositionX;

    // 1. Вертикальная линия через весь экран
    g.setColour(playheadColor.withAlpha(0.8f));
    g.drawVerticalLine(juce::roundToInt(px), 0.0f, static_cast<float>(getHeight()));
    
    // 2. Красивая каретка (флажок) в самом верху
    float flagHeight = 20.0f;
    juce::Path head;
    head.startNewSubPath(px - 7.0f, 0.0f);
    head.lineTo(px + 7.0f, 0.0f);
    head.lineTo(px + 7.0f, flagHeight - 7.0f);
    head.lineTo(px, flagHeight);
    head.lineTo(px - 7.0f, flagHeight - 7.0f);
    head.closeSubPath();

    g.setColour(playheadColor);
    g.fillPath(head);
    
    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.strokePath(head, juce::PathStrokeType(1.0f));
    
    g.restoreState();
}

bool PlayheadOverlay::hitTest(int x, int y) {
    juce::ignoreUnused(x, y);
    return false; // Полностью игнорируем мышь
}

void PlayheadOverlay::timerCallback() {
    // Запрашиваем актуальное время у движка
    double timeInSeconds = engine.getCurrentPosition();
    float newPositionX = static_cast<float>(timeInSeconds * pixelsPerSecond);
    
    // Если позиция изменилась хотя бы на долю пикселя — принудительно перерисовываем
    if (currentPositionX != newPositionX) {
        currentPositionX = newPositionX;
        repaint();
        
        // Раскомментируй строку ниже, если ползунок всё равно стоит на месте.
        // Если в консоль побегут нули, значит движок почему-то не отдает время UI.
        DBG("Playhead time: " << timeInSeconds << " | px: " << currentPositionX);
    }
}

void PlayheadOverlay::setPixelsPerSecond(double newPixelsPerSecond) {
    if (newPixelsPerSecond > 0.0) {
        pixelsPerSecond = newPixelsPerSecond;
        repaint();
    }
}

void PlayheadOverlay::setXOffset(int newOffset) {
    if (xOffset != newOffset) {
        xOffset = newOffset;
        repaint();
    }
}
