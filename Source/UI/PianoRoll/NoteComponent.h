#pragma once

#include <JuceHeader.h>
#include "../../Model/TrackModel.h"


class NoteComponent : public juce::Component {
public:
    NoteComponent(const juce::Uuid& noteId, TrackModel& track, juce::Colour colour);
    ~NoteComponent() override;

    void paint(juce::Graphics& g) override;
    
    // События мыши (подготовка для редактирования)
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp  (const juce::MouseEvent& e) override;
    
    // При наведении мыши меняем курсор
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit (const juce::MouseEvent& e) override;

    const juce::Uuid& getNoteId() const noexcept { return noteUuid; }
    
    // Обновление цвета
    void setNoteColour(juce::Colour newColour);

private:
    juce::Uuid noteUuid;
    TrackModel& trackModel;
    juce::Colour noteColour;
    
    // Состояния для отрисовки и взаимодействия
    bool isHovered = false;
    bool isDragging = false;
    
    // Координаты начала перетаскивания (для вычисления смещения)
    juce::Point<int> dragStartPos;

    // Параметры отрисовки
    static constexpr float kCornerSize = 4.0f;
    static constexpr float kBorderWidth = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoteComponent)
};
