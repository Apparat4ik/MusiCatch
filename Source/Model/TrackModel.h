#pragma once

#include <JuceHeader.h>
#include "NoteData.h"



class TrackModel {
 public:
    // Конструктор принимает узел ValueTree типа "Track"
    TrackModel() = default;
    TrackModel(juce::ValueTree trackTreeToUse);
    ~TrackModel() = default;

    // Параметры трека
    juce::String getName() const;
    void setName(const juce::String& newName);

    juce::Colour getColour() const;
    void setColour(const juce::Colour& newColour);

    bool isMuted() const;
    void setMuted(bool shouldMute);

    bool isSoloed() const;
    void setSoloed(bool shouldSolo);


    juce::Uuid getUuid() const;
    
    // Возвращает список всех нот в треке копирует данные, поэтому потокобезопасно для UI.
    juce::Array<NoteData> getNoteList() const;

    void addNote(const NoteData& note);
    void removeNote(int noteIndex);
    int getNumNotes() const;
    
    juce::ValueTree getTrackTree() const { return trackTree; }

 private:
    juce::ValueTree trackTree;

    static const juce::Identifier notesNodeId;
    static const juce::Identifier noteNodeId;
};
