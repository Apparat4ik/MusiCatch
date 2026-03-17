#pragma once
#include <JuceHeader.h>

class CustomLookAndFeel : public juce::LookAndFeel_V4 {
 public:
    CustomLookAndFeel();
    ~CustomLookAndFeel() override = default;

    // Переопределение отрисовки крутилок (Rotary Knobs)
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, const float rotaryStartAngle,
                           const float rotaryEndAngle, juce::Slider& slider) override;

    // Переопределение отрисовки линейных ползунков (Volume Faders)
    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           const juce::Slider::SliderStyle style, juce::Slider& slider) override;

    // Переопределение кнопок (Mute, Solo, Record, Transport)
    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;
                               
    // Отрисовка скроллбаров (важно для Piano roll)
    void drawScrollbar (juce::Graphics& g, juce::ScrollBar& scrollbar,
                        int x, int y, int width, int height,
                        bool isScrollbarVertical, int thumbStartPosition,
                        int thumbSize, bool isMouseOver, bool isMouseDown) override;
};
