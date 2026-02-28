#pragma once

#include <JuceHeader.h>
#include "NoteComponent.h"
#include "../../Audio/AudioEngine.h"
#include "../../Core/AppState.h"


class PianoRollComponent : public juce::Component,
                           private juce::ValueTree::Listener {
 public:
    // Внедряем зависимости: глобальное состояние и аудиодвижок
    PianoRollComponent(AppState& state, AudioEngine& engine);
    ~PianoRollComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // --- Функции конвертации координат ---
    float timeToX(float timeSeconds) const;
    float xToTime(float x) const;
    float pitchToY(int midiNote) const;
    int yToPitch(float y) const;

    // Обновление дочерних компонентов-нот для активного трека
    void updateNotes();

    // Настройка масштаба
    void setZoom(float newPixelsPerSecond, float newNoteHeight);
                               
    // PianoRollComponent.h (добавить в public секцию)
    float getNoteHeight() const { return noteHeight; }
    int getMinMidiNote() const { return minMidiNote; }
    int getMaxMidiNote() const { return maxMidiNote; }

 private:
    // Отрисовка
    void drawGrid(juce::Graphics& g);
    void drawInactiveTracks(juce::Graphics& g);

    // Автоматически обновляем UI при изменении данных в AppState
    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
    void valueTreeChildAdded(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded) override;
    void valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved) override;
    void valueTreeChildOrderChanged(juce::ValueTree& parentTreeWhoseChildrenHaveMoved, int oldIndex, int newIndex) override {}
    void valueTreeParentChanged(juce::ValueTree& treeWhoseParentHasChanged) override {}

    // Ссылки на внешние сервисы
    AppState& appState;

    // Хранилище дочерних UI-компонентов (активных нот)
    juce::OwnedArray<NoteComponent> activeNoteComponents;

    // --- Параметры сетки и масштаба ---
    float pixelsPerSecond = 100.0f;
    float noteHeight = 20.0f;
    
    // Диапазон отображаемых MIDI нот (например, от C1 до C7)
    int maxMidiNote = 108;
    int minMidiNote = 24;

    // Идентификатор активного трека (которому принадлежат кликабельные ноты)
    juce::String activeTrackId;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRollComponent)
};
