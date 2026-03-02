#include "NoteComponent.h"

NoteComponent::NoteComponent(const juce::Uuid& id, TrackModel track, juce::Colour colour)
    : noteUuid(id)
    , trackModel(track)
    , noteColour(colour) {
    // Компонент не "невидимый" для мыши
    setInterceptsMouseClicks(true, false);
}

NoteComponent::~NoteComponent() {}

void NoteComponent::setNoteColour(juce::Colour newColour) {
    if (noteColour != newColour) {
        noteColour = newColour;
        repaint();
    }
}


void NoteComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    
    // Если нота маленькая, делаем небольшие отступы, чтобы ноты не сливались
    bounds.reduce(0.5f, 0.5f);

    // Цвет заливки (немного светлее, если навели мышь)
    juce::Colour fillColour = isHovered ? noteColour.brighter(0.2f) : noteColour;
    
    // Полупрозрачность при перетаскивании (Feedback для пользователя)
    if (isDragging)
        fillColour = fillColour.withAlpha(0.7f);

    // Заливка
    g.setColour(fillColour);
    g.fillRoundedRectangle(bounds, kCornerSize);

    // Обводка
    g.setColour(noteColour.darker(0.8f));
    g.drawRoundedRectangle(bounds, kCornerSize, kBorderWidth);
    
    // Небольшой градиент или "блик" для эффекта объема
    juce::ColourGradient gradient(juce::Colour(0x22FFFFFF), bounds.getX(), bounds.getY(),
                                  juce::Colour(0x00FFFFFF), bounds.getX(), bounds.getBottom(),
                                  false);
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds, kCornerSize);
}


void NoteComponent::mouseEnter(const juce::MouseEvent& e) {
    isHovered = true;
    setMouseCursor(juce::MouseCursor::NormalCursor);
    // В будущем тут можно менять курсор на LeftRight или UpDown
    // в зависимости от того, навели на край ноты или в центр.
    repaint();
}

void NoteComponent::mouseExit(const juce::MouseEvent& e) {
    isHovered = false;
    repaint();
}


void NoteComponent::mouseDown(const juce::MouseEvent& e) {
    isDragging = true;
    
    // Запоминаем точку, в которой кликнули относительно самого компонента
    dragStartPos = e.getPosition();
    
    // Выводим ноту на передний план (z-index), чтобы при перетаскивании
    // она рисовалась поверх остальных нот
    toFront(true);
    
    repaint();
}

void NoteComponent::mouseDrag(const juce::MouseEvent& e) {
    if (!isDragging) return;

    // Вычисляем, на сколько пикселей сдвинулась мышь с момента mouseDown.
    int deltaX = e.x - dragStartPos.getX();
    int deltaY = e.y - dragStartPos.getY();

    setTopLeftPosition(getX() + deltaX, getY() + deltaY);
}

void NoteComponent::mouseUp(const juce::MouseEvent& e) {
    if (isDragging) {
        isDragging = false;
        // позже сделать отправку данных
        // конвертировать новые пиксельные координаты (getX(), getY())
        // обратно во время (start, duration) и высоту тона (pitch).
        repaint();
    }
}
