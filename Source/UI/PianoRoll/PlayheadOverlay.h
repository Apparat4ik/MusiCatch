#pragma once

#include <JuceHeader.h>
#include "../../Audio/AudioEngine.h" // Путь согласно вашей файловой структуре

class PlayheadOverlay : public juce::Component,
                        private juce::Timer {
 public:
    // Внедрение зависимости (DI). Компонент UI не управляет временем жизни движка.
    explicit PlayheadOverlay(AudioEngine& engineToUse);
    ~PlayheadOverlay() override;

    void paint(juce::Graphics& g) override;
    
    // Переопределяем hitTest для избирательного пропуска кликов.
    bool hitTest(int x, int y) override;

    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;

    // Установка масштаба времени (чтобы переводить секунды движка в пиксели экрана)
    void setPixelsPerSecond(double newPixelsPerSecond);
    void setXOffset(int newOffset);

 private:
    void timerCallback() override;

    AudioEngine& engine;
                            
    int xOffset = 0;
    double pixelsPerSecond = 100.0; // Дефолтный масштаб по X
    int rulerHeight = 30;           // Высота верхней области (линейки)
    float currentPositionX = 0.0f;  // Кешируем позицию для оптимизации перерисовок

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayheadOverlay)
};
