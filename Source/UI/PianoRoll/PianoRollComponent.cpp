#include "PianoRollComponent.h"

PianoRollComponent::PianoRollComponent(AppState& state, AudioEngine& engine)
    : appState(state) {
    setInterceptsMouseClicks(true, true);
    
    // Подписываемся на изменения в глобальном дереве
    appState.getRootNode().addListener(this);

    // Временно выберем первый трек как активный, если он есть
    auto tracksNode = appState.getTrackList();
    if (tracksNode.getNumChildren() > 0) {
        activeTrackId = tracksNode.getChild(0).getProperty("id").toString();
    }
    
    updateNotes();
}

PianoRollComponent::~PianoRollComponent() {
    appState.getRootNode().removeListener(this);
    activeNoteComponents.clear();
}

// Функции конвертации
float PianoRollComponent::timeToX(float timeSeconds) const { return timeSeconds * pixelsPerSecond; }
float PianoRollComponent::xToTime(float x) const { return x / pixelsPerSecond; }
float PianoRollComponent::pitchToY(int midiNote) const { return static_cast<float>(maxMidiNote - midiNote) * noteHeight; }
int PianoRollComponent::yToPitch(float y) const { return maxMidiNote - static_cast<int>(y / noteHeight); }


void PianoRollComponent::setZoom(float newPixelsPerSecond, float newNoteHeight) {
    pixelsPerSecond = juce::jmax(10.0f, newPixelsPerSecond);
    noteHeight = juce::jmax(5.0f, newNoteHeight);
    
    
    updateNotes();
    repaint();
}

// Отрисовка
void PianoRollComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff2d2d2d));

    drawGrid(g);
    drawInactiveTracks(g); // Рисуем силуэты
}

void PianoRollComponent::drawGrid(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    
    for (int note = minMidiNote; note <= maxMidiNote; ++note) {
        float y = pitchToY(note);
        int noteInOctave = note % 12;
        
        bool isBlackKey = (noteInOctave == 1 || noteInOctave == 3 || noteInOctave == 6 || noteInOctave == 8 || noteInOctave == 10);
                           
        g.setColour(isBlackKey ? juce::Colour(0xff202020) : juce::Colour(0xff333333));
        g.fillRect(0.0f, y, bounds.getWidth(), noteHeight);
        
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.drawHorizontalLine(juce::roundToInt(y + noteHeight), 0.0f, bounds.getWidth());
    }

    g.setColour(juce::Colours::white.withAlpha(0.1f));
    
    double bpm = 120.0;
    double beatsPerSecond = bpm / 60.0;
    double pixelsPerBeat = pixelsPerSecond / beatsPerSecond;
    
    int visibleBeats = bounds.getWidth() / pixelsPerBeat;
        
    for (int i = 0; i <= visibleBeats; ++i) {
        float x = static_cast<float>(i * pixelsPerBeat);
        bool isMeasure = (i % 4 == 0); // Начало такта
        
        // Линии тактов делаем чуть ярче, линии долей — совсем тусклыми
        g.setColour(juce::Colours::white.withAlpha(isMeasure ? 0.15f : 0.05f));
        g.drawVerticalLine(juce::roundToInt(x), 0.0f, bounds.getHeight());
    }
}

void PianoRollComponent::drawInactiveTracks(juce::Graphics& g) {
    const juce::ScopedLock sl(appState.getLock());

    juce::ValueTree tracksNode = appState.getTrackList();
    if (!tracksNode.isValid()) return;

    for (int i = 0; i < tracksNode.getNumChildren(); ++i) {
        juce::ValueTree trackNode = tracksNode.getChild(i);
        
        if (trackNode.getProperty("id").toString() == activeTrackId) continue;
        if (static_cast<bool>(trackNode.getProperty("isMuted", false))) continue;

        juce::String colorStr = trackNode.getProperty("color", juce::Colours::grey.toString());
        juce::Colour trackColor = juce::Colour::fromString(colorStr).withAlpha(0.3f);
        g.setColour(trackColor);

        juce::ValueTree notesNode = trackNode.getChildWithName("Notes");
        if (notesNode.isValid()) {
            for (int j = 0; j < notesNode.getNumChildren(); ++j) {
                juce::ValueTree noteNode = notesNode.getChild(j);
                
                // Распаковываем данные
                NoteData noteData = juce::VariantConverter<NoteData>::fromVar(noteNode.getProperty("data"));

                float x = timeToX(noteData.startTime);
                float y = pitchToY(noteData.midiPitch);
                float width = juce::jmax(3.0f, timeToX(noteData.duration));
                
                g.fillRect(x, y, width, noteHeight);
            }
        }
    }
}

void PianoRollComponent::resized() {

   // updateNotes();
}

void PianoRollComponent::updateNotes() {
    activeNoteComponents.clear();

    const juce::ScopedLock sl(appState.getLock());
    juce::ValueTree tracksNode = appState.getTrackList();
    
    // Ищем активный трек
    juce::ValueTree activeTrackNode = tracksNode.getChildWithProperty("id", activeTrackId);
    if (!activeTrackNode.isValid()) return;

    juce::String colorStr = activeTrackNode.getProperty("color", juce::Colours::deepskyblue.toString());
    juce::Colour activeColor = juce::Colour::fromString(colorStr);

    // Создаем локальную модель для безопасной передачи в NoteComponent
    TrackModel trackModel(activeTrackNode);

    juce::ValueTree notesNode = activeTrackNode.getChildWithName("Notes");
    if (notesNode.isValid()) {
        for (int j = 0; j < notesNode.getNumChildren(); ++j) {
            juce::ValueTree noteNode = notesNode.getChild(j);
            
            // 1. Десериализуем данные ноты через твой конвертер!
            NoteData noteData = juce::VariantConverter<NoteData>::fromVar(noteNode.getProperty("data"));

            juce::String uuidStr = noteNode.getProperty("id", juce::Uuid().toString());
            juce::Uuid noteId(uuidStr);

            // 2. Передаем реальную модель
            auto* noteComp = activeNoteComponents.add(new NoteComponent(noteId, trackModel, activeColor));
            addAndMakeVisible(noteComp);

            // 3. Берем данные из структуры NoteData
            float x = timeToX(noteData.startTime);
            float y = pitchToY(noteData.midiPitch);
            float width = timeToX(noteData.duration);
            
            // Защита: даже очень короткий звук должен быть виден (минимум 3 пикселя)
            width = juce::jmax(3.0f, width);

            noteComp->setBounds(juce::roundToInt(x), juce::roundToInt(y), juce::roundToInt(width), juce::roundToInt(noteHeight));
        }
    }
    
    repaint();
}

// ValueTree Listeners
void PianoRollComponent::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) {
    if (tree == appState.getRootNode() && property == juce::Identifier("selectedTrackId")) {
        activeTrackId = tree.getProperty("selectedTrackId").toString();
    }
    // Если поменялась нота, цвет трека или активный статус — перерисовываем
    juce::MessageManager::callAsync([this]() {
        updateNotes();
    });
}

void PianoRollComponent::valueTreeChildAdded(juce::ValueTree& parentTree, juce::ValueTree& child) {
    juce::MessageManager::callAsync([this]() { updateNotes(); });
}

void PianoRollComponent::valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& child, int index) {
    juce::MessageManager::callAsync([this]() { updateNotes(); });
}
