#include "AppState.h"


const juce::Identifier AppState::projectNodeId ("Project");
const juce::Identifier AppState::tracksNodeId  ("Tracks");
const juce::Identifier AppState::trackNodeId   ("Track");

AppState& AppState::getInstance() {
    static AppState instance;
    return instance;
}

AppState::AppState() : rootNode (projectNodeId) {
    // Инициализируем структуру: Project -> Tracks
    auto tracks = juce::ValueTree(tracksNodeId);
    rootNode.addChild(tracks, -1, nullptr);
}

void AppState::addTrack(const juce::String& trackName) {
    // Блокируем доступ на время записи
    const juce::ScopedLock sl (dataLock);

    // Создаем новый узел трека
    juce::ValueTree newTrack (trackNodeId);
    
    // Устанавливаем свойства (имя, цвет, ID)
    newTrack.setProperty("name", trackName, nullptr);
    newTrack.setProperty("id", juce::Uuid().toString(), nullptr);
    newTrack.setProperty("color", juce::Colours::cornflowerblue.toString(), nullptr);
    newTrack.setProperty("isMuted", false, nullptr);
    newTrack.setProperty("isSoloed", false, nullptr);

    // Создаем под-узел для клипов/нот
    newTrack.addChild(juce::ValueTree("Notes"), -1, nullptr);

    // Добавляем в список треков
    // getChildWithName возвращает 'Tracks' узел
    auto tracksNode = rootNode.getChildWithName(tracksNodeId);
    if (tracksNode.isValid()) {
        tracksNode.addChild(newTrack, -1, nullptr);
    }
    DBG("AppState: Добавлен новый трек: " << trackName);
}

void AppState::removeTrack(int index) {
    const juce::ScopedLock sl (dataLock);

    auto tracksNode = rootNode.getChildWithName(tracksNodeId);
    if (tracksNode.isValid()
        && index >= 0
        && index < tracksNode.getNumChildren()) {
        
        tracksNode.removeChild(index, nullptr);
    }
}

juce::ValueTree AppState::getTrackList() {
    // Возвращаем копию ссылки на узел Tracks.
    const juce::ScopedLock sl (dataLock);
    return rootNode.getChildWithName(tracksNodeId);
}
