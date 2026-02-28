#pragma once

#include <JuceHeader.h>
#include "NoteData.h"

/**
 * Модель одного аудиотрека. Владеет своими нотами и параметрами.
 * Обертка над ValueTree с удобным API.
 */
class TrackModel
{
public:
    // Конструктор принимает узел ValueTree типа "Track"
    TrackModel(juce::ValueTree trackTreeToUse);
    ~TrackModel() = default;

    // Параметры трека
    
    //Имя трека (по умолчанию "Track 1")
    juce::String getName() const;
    void setName(const juce::String& newName);

    // Цвет трека для UI
    juce::Colour getColour() const;
    void setColour(const juce::Colour& newColour);

    // Мьют трека
    bool isMuted() const;
    void setMuted(bool shouldMute);

    // Соло трека
    bool isSoloed() const;
    void setSoloed(bool shouldSolo);

    // Уникальный ID трека
    juce::Uuid getUuid() const;


    // Работа с нотами
    
    // Возвращает список всех нот в треке копирует данные, поэтому потокобезопасно для UI.
    juce::Array<NoteData> getNoteList() const;

    // Добавляет ноту в конец списка
    void addNote(const NoteData& note);

    // Удаляет ноту по индексу
    void removeNote(int noteIndex);

    // Количество нот в треке
    int getNumNotes() const;

    // ValueTree доступ
    
    juce::ValueTree getTrackTree() const { return trackTree; }

private:
    juce::ValueTree trackTree;  // Узел "Track" из AppState

    // Константы идентификаторов
    static const juce::Identifier notesNodeId;
    static const juce::Identifier noteNodeId;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackModel)
};
