#pragma once

#include <JuceHeader.h>

class AppState {
  public:
    static AppState& getInstance();

    // Добавляет новый трек в конец списка
    void addTrack(const juce::String& trackName);

    // Удаляет трек по индексу.
    void removeTrack(int index);

    // Возвращает узел "Tracks", содержащий все дорожки
    juce::ValueTree getTrackList();
    
    // Возвращает корневой узел для привязки глобальных слушателей (Listeners)
    juce::ValueTree& getRootNode() { return rootNode; }

    juce::CriticalSection& getLock() { return dataLock; }

 private:
    AppState();
    ~AppState() = default;
    
    juce::ValueTree rootNode;

    // Мьютекс для синхронизации доступа (UI vs Audio Analysis Thread)
    juce::CriticalSection dataLock;

    // Константы для имен узлов
    static const juce::Identifier projectNodeId;
    static const juce::Identifier tracksNodeId;
    static const juce::Identifier trackNodeId;

    JUCE_DECLARE_NON_COPYABLE(AppState)
};
