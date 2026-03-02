#include "TimelineComponent.h"

TimelineComponent::TimelineComponent(AudioEngine& engineToUse) : engine(engineToUse) {
    // Компонент должен перехватывать клики мыши
    setInterceptsMouseClicks(true, false);
}

void TimelineComponent::paint(juce::Graphics& g) {
    // 1. Фон таймлайна
    g.fillAll(juce::Colour(0xFF252525));
    
    // Нижняя граница
    g.setColour(juce::Colours::black);
    g.drawHorizontalLine(getHeight() - 1, 0, getWidth());
    
    // 2. Смещение сетки (синхронизация со скроллом)
    g.saveState();
    g.addTransform(juce::AffineTransform::translation(static_cast<float>(-xOffset), 0.0f));

    // --- Отрисовка музыкальной сетки ---
    double bpm = 120.0;
    double beatsPerSecond = bpm / 60.0;
    double pixelsPerBeat = pixelsPerSecond / beatsPerSecond;
    
    int startBeat = static_cast<int>(xOffset / pixelsPerBeat);
    int endBeat = static_cast<int>((xOffset + getWidth()) / pixelsPerBeat) + 1;
    
    g.setFont(juce::Font(11.0f));
    
    for (int i = startBeat; i <= endBeat; ++i) {
        float x = static_cast<float>(i * pixelsPerBeat);
        bool isMeasure = (i % 4 == 0);
        
        if (isMeasure) {
            g.setColour(juce::Colours::white.withAlpha(0.6f));
            g.drawVerticalLine(juce::roundToInt(x), 0.0f, static_cast<float>(getHeight()));
            int measureNumber = (i / 4) + 1;
            g.drawText(juce::String(measureNumber), juce::roundToInt(x) + 4, 2, 40, 14, juce::Justification::topLeft, false);
        } else {
            g.setColour(juce::Colours::white.withAlpha(0.2f));
            // Маленькие рисочки для долей
            g.drawVerticalLine(juce::roundToInt(x), static_cast<float>(getHeight()) - 8.0f, static_cast<float>(getHeight()));
        }
    }

    g.restoreState();
}

void TimelineComponent::mouseDown(const juce::MouseEvent& event) {
    double timeInSeconds = (event.position.x + xOffset) / pixelsPerSecond;
    engine.setPosition(juce::jmax(0.0, timeInSeconds));
}

void TimelineComponent::mouseDrag(const juce::MouseEvent& event) {
    double timeInSeconds = (event.position.x + xOffset) / pixelsPerSecond;
    engine.setPosition(juce::jmax(0.0, timeInSeconds));
}

void TimelineComponent::setPixelsPerSecond(double newPixels) {
    if (newPixels > 0.0) {
        pixelsPerSecond = newPixels;
        repaint();
    }
}

void TimelineComponent::setXOffset(int newOffset) {
    if (xOffset != newOffset) {
        xOffset = newOffset;
        repaint();
    }
}
