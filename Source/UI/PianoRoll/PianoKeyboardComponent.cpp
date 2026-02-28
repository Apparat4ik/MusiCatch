#include "PianoKeyboardComponent.h"

PianoKeyboardComponent::PianoKeyboardComponent(PianoRollComponent& pr)
    : pianoRoll(pr) {
}

void PianoKeyboardComponent::paint(juce::Graphics& g) {
    // Темный фон по умолчанию
    g.fillAll(juce::Colour(0xFF1E1E1E));

    int minNote = pianoRoll.getMinMidiNote();
    int maxNote = pianoRoll.getMaxMidiNote();
    float noteHeight = pianoRoll.getNoteHeight();
    float width = static_cast<float>(getWidth());

    // Отрисовываем клавиши сверху вниз
    for (int note = minNote; note <= maxNote; ++note) {
        float y = pianoRoll.pitchToY(note);
        int noteInOctave = note % 12;
        
        // Определяем, черная ли это клавиша
        bool isBlackKey = (noteInOctave == 1 || noteInOctave == 3 || noteInOctave == 6 ||
                           noteInOctave == 8 || noteInOctave == 10);

        juce::Rectangle<float> keyBounds(0.0f, y, width, noteHeight);

        if (isBlackKey) {
            g.setColour(juce::Colour(0xFF111111)); // Почти черный
            g.fillRect(keyBounds);
        } else {
            g.setColour(juce::Colour(0xFFDCDCDC)); // Светло-серый (белые клавиши)
            g.fillRect(keyBounds);
            
            // Тонкая линия между белыми клавишами
            g.setColour(juce::Colour(0xFF888888));
            g.drawRect(keyBounds, 1.0f);

            // Подписываем ноты 'До' (C)
            if (noteInOctave == 0) {
                g.setColour(juce::Colour(0xFF111111));
                g.setFont(juce::Font(12.0f, juce::Font::bold));
                // JUCE обычно считает C3 = 60
                int octave = (note / 12) - 2;
                g.drawText("C" + juce::String(octave), keyBounds.reduced(4.0f, 0.0f),
                           juce::Justification::centredRight, false);
            }
        }
    }
}
