#include "TrackListComponent.h"


TrackListComponent::TrackListComponent() {
    tracksTree = AppState::getInstance().getTrackList();
    rootTree   = AppState::getInstance().getRootNode();

    // 1. tracksTree: чтобы ловить добавление/удаление треков
    tracksTree.addListener(this);
    // 2. rootTree: чтобы ловить изменение свойства "selectedTrackId" (например, от Undo/Redo)
    rootTree.addListener(this);

    // Первичная сборка списка, если в AppState уже есть треки при загрузке
    rebuildTrackList();
}

TrackListComponent::~TrackListComponent() {
    tracksTree.removeListener(this);
    rootTree.removeListener(this);
}


void TrackListComponent::paint(juce::Graphics& g) {
    // Фон под списком треков
    g.fillAll(juce::Colour(0xFF1E1E1E));
}

void TrackListComponent::resized() {
    auto area = getLocalBounds();

    for (auto* header : trackHeaders) {
        header->setBounds(area.removeFromTop(kTrackHeight));
    }
}

void TrackListComponent::rebuildTrackList() {
    trackHeaders.clear();
    trackModels.clear();

    const juce::ScopedLock sl(AppState::getInstance().getLock());

    // Проходим по всем дочерним узлам <Track> внутри <Tracks>
    for (int i = 0; i < tracksTree.getNumChildren(); ++i) {
        juce::ValueTree childNode = tracksTree.getChild(i);

        // Создаем модель для узла
        auto* model = trackModels.add(new TrackModel(childNode));
        
        // Создаем UI компонент для модели
        auto* header = trackHeaders.add(new TrackHeaderComponent(*model));
        addAndMakeVisible(header);

        // Назначаем реакцию на клик: когда пользователь выбирает этот трек,
        // мы сохраняем его ID в глобальный AppState.
        header->onSelected = [this, model]() {
            setSelectedTrackId(model->getUuid().toString());
        };
    }

    // Обновляем выделение
    updateSelection();

    // Пересчитываем геометрию
    resized();
}


void TrackListComponent::setSelectedTrackId(const juce::String& trackId) {
    // Пишем ID выбранного трека в корень AppState.
    // Это автоматически вызовет valueTreePropertyChanged,
    // который обновит интерфейс через updateSelection().
    
    const juce::ScopedLock sl(AppState::getInstance().getLock());
    rootTree.setProperty("selectedTrackId", trackId, nullptr);
}

void TrackListComponent::updateSelection() {
    juce::String currentSelectedId;
    
    const juce::ScopedLock sl(AppState::getInstance().getLock());
    currentSelectedId = rootTree.getProperty("selectedTrackId", "").toString();
    

    // Раздаем команду выделения всем хедерам
    for (int i = 0; i < trackHeaders.size(); ++i) {
        auto* header = trackHeaders[i];
        auto* model  = trackModels[i];
        
        bool isThisTrackSelected = (model->getUuid().toString() == currentSelectedId);
        header->setSelected(isThisTrackSelected);
    }
}


void TrackListComponent::valueTreeChildAdded(juce::ValueTree& parentTree,
                                             juce::ValueTree&) {
    if (parentTree == tracksTree) {
        rebuildTrackList();
    }
}

void TrackListComponent::valueTreeChildRemoved(juce::ValueTree& parentTree,
                                               juce::ValueTree& childWhichHasBeenRemoved,
                                               int indexFromWhichChildWasRemoved) {
    if (parentTree == tracksTree) {
        rebuildTrackList();
    }
}

void TrackListComponent::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged,
                                                  const juce::Identifier& property) {
    // Если где-то (в UI или из UndoManager) изменили selectedTrackId — перерисовываем выделение.
    if (treeWhosePropertyHasChanged == rootTree && property == juce::Identifier("selectedTrackId")) {
        updateSelection();
    }
}
