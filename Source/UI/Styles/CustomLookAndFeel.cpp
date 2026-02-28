#include "CustomLookAndFeel.h"

CustomLookAndFeel::CustomLookAndFeel()
{
    // Инициализация глобальной палитры (позже можно вынести в Constants.h)
    // Темная тема в стиле Melodyne/Ableton
    const juce::Colour bgDark      = juce::Colour(0xFF1E1E1E); // Глубокий серый
    const juce::Colour panelGray   = juce::Colour(0xFF2D2D2D); // Фон панелей
    const juce::Colour accentBlue  = juce::Colour(0xFF4A90E2); // Тот самый синий для активных элементов
    const juce::Colour textLight   = juce::Colour(0xFFDCDCDC);

    // Применяем цвета к базовым элементам JUCE
    setColour (juce::ResizableWindow::backgroundColourId, bgDark);
    setColour (juce::Slider::thumbColourId, accentBlue);
    setColour (juce::Slider::rotarySliderFillColourId, accentBlue);
    setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xFF111111));
    setColour (juce::Slider::trackColourId, panelGray);
    setColour (juce::TextButton::buttonColourId, panelGray);
    setColour (juce::TextButton::textColourOffId, textLight);
}

void CustomLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPos, const float rotaryStartAngle,
                                         const float rotaryEndAngle, juce::Slider& slider)
{
    // Современная отрисовка крутилок в виде дуги (Arc), а не 3D-ручки
    auto radius = (float) juce::jmin (width / 2, height / 2) - 4.0f;
    auto centreX = (float) x + (float) width  * 0.5f;
    auto centreY = (float) y + (float) height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Толщина линии
    float lineThickness = 3.0f;

    // Фон крутилки (темный трек)
    juce::Path backgroundArc;
    backgroundArc.addCentredArc (centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (slider.findColour (juce::Slider::rotarySliderOutlineColourId));
    g.strokePath (backgroundArc, juce::PathStrokeType (lineThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Заполненная часть крутилки (акцентный цвет)
    if (slider.isEnabled())
    {
        juce::Path valueArc;
        valueArc.addCentredArc (centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, angle, true);
        g.setColour (slider.findColour (juce::Slider::rotarySliderFillColourId));
        g.strokePath (valueArc, juce::PathStrokeType (lineThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Индикатор (точка или линия на ручке)
    juce::Path p;
    auto pointerLength = radius * 0.33f;
    p.addRectangle (-lineThickness * 0.5f, -radius, lineThickness, pointerLength);
    p.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));
    g.setColour (slider.isEnabled() ? juce::Colours::white : juce::Colours::grey);
    g.fillPath (p);
}

void CustomLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPos, float minSliderPos, float maxSliderPos,
                                         const juce::Slider::SliderStyle style, juce::Slider& slider)
{
    // Плоский минималистичный фейдер
    g.setColour (slider.findColour (juce::Slider::trackColourId));
    
    if (slider.isVertical())
    {
        float trackWidth = 4.0f;
        g.fillRoundedRectangle (x + (width - trackWidth) * 0.5f, y, trackWidth, height, 2.0f);
        
        g.setColour (slider.findColour (juce::Slider::thumbColourId));
        g.fillRoundedRectangle (x + width * 0.2f, sliderPos - 8.0f, width * 0.6f, 16.0f, 3.0f);
    }
    // Добавь else if (slider.isHorizontal()) по аналогии, если будут горизонтальные ползунки
}

void CustomLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                             const juce::Colour& backgroundColour,
                                             bool shouldDrawButtonAsHighlighted,
                                             bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
    auto baseColour = backgroundColour.withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f);

    // Логика нажатия и наведения (Flat UI: просто меняем яркость, никаких градиентов)
    if (shouldDrawButtonAsDown || button.getToggleState())
        baseColour = baseColour.brighter (0.2f); // Включенная кнопка светлее (например, Solo или выбранный трек)
    else if (shouldDrawButtonAsHighlighted)
        baseColour = baseColour.brighter (0.1f); // Hover-эффект

    g.setColour (baseColour);
    g.fillRoundedRectangle (bounds, 4.0f); // Скругленные углы (4px)

    // Тонкая обводка для контраста
    g.setColour (juce::Colour (0xFF111111).withAlpha (0.3f));
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
}

void CustomLookAndFeel::drawScrollbar (juce::Graphics& g, juce::ScrollBar& scrollbar,
                                       int x, int y, int width, int height,
                                       bool isScrollbarVertical, int thumbStartPosition,
                                       int thumbSize, bool isMouseOver, bool isMouseDown)
{
    // Минималистичный скроллбар (как в macOS/современных браузерах)
    g.setColour (juce::Colour(0xFF202020)); // Фон скролла
    g.fillRect (x, y, width, height);

    juce::Rectangle<int> thumbBounds = isScrollbarVertical
                                     ? juce::Rectangle<int> (x + 2, thumbStartPosition + 2, width - 4, thumbSize - 4)
                                     : juce::Rectangle<int> (thumbStartPosition + 2, y + 2, thumbSize - 4, height - 4);

    auto thumbColour = isMouseDown ? juce::Colour(0xFF666666) :
                       isMouseOver ? juce::Colour(0xFF555555) : juce::Colour(0xFF444444);
                       
    g.setColour (thumbColour);
    g.fillRoundedRectangle (thumbBounds.toFloat(), 4.0f);
}
