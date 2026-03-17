#include "TrackModel.h"

const juce::Identifier TrackModel::notesNodeId ("Notes");
const juce::Identifier TrackModel::noteNodeId  ("Note");
// juce::Identifier позволяет сравнивать строки по их хэшу

TrackModel::TrackModel(juce::ValueTree trackTreeToUse)
    : trackTree (trackTreeToUse) {
    juce::ignoreUnused(trackTreeToUse);
}

juce::String TrackModel::getName() const {
    return trackTree.getProperty("name", "Unnamed Track");
}

void TrackModel::setName(const juce::String& newName) {
    trackTree.setProperty("name", newName, nullptr);
}

juce::Colour TrackModel::getColour() const {
    return juce::Colour::fromString(trackTree.getProperty("color", juce::Colours::cornflowerblue.toString()).toString());
}

void TrackModel::setColour(const juce::Colour& newColour) {
    trackTree.setProperty("color", newColour.toString(), nullptr);
}

bool TrackModel::isMuted() const {
    return trackTree.getProperty("isMuted", false);
}

void TrackModel::setMuted(bool shouldMute) {
    trackTree.setProperty("isMuted", shouldMute, nullptr);
}

bool TrackModel::isSoloed() const {
    return trackTree.getProperty("isSoloed", false);
}

void TrackModel::setSoloed(bool shouldSolo) {
    trackTree.setProperty("isSoloed", shouldSolo, nullptr);
}

juce::Uuid TrackModel::getUuid() const {
    return juce::Uuid(trackTree.getProperty("id", "").toString());
}

juce::Array<NoteData> TrackModel::getNoteList() const {
    juce::Array<NoteData> notes;

    // Находим узел "Notes"
    auto notesNode = trackTree.getChildWithName(notesNodeId);
    if (!notesNode.isValid())
        return notes;

    // Читаем все дочерние "Note"
    for (int i = 0; i < notesNode.getNumChildren(); ++i) {
        auto noteNode = notesNode.getChild(i);
        if (noteNode.hasType(noteNodeId)) {
            NoteData note = juce::VariantConverter<NoteData>::fromVar(
                            noteNode.getProperty("data", juce::var())
                        );
            notes.add(note);
        }
    }

    return notes;
}

void TrackModel::addNote(const NoteData& note) {
    auto notesNode = trackTree.getChildWithName(notesNodeId);
    if (!notesNode.isValid()) {
        notesNode = juce::ValueTree(notesNodeId);
        trackTree.addChild(notesNode, -1, nullptr);
    }

    // Создаем узел ноты и сохраняем данные
    juce::ValueTree newNote (noteNodeId);
    newNote.setProperty("id", juce::Uuid().toString(), nullptr);
    newNote.setProperty("data", juce::VariantConverter<NoteData>::toVar(note), nullptr);
    notesNode.addChild(newNote, -1, nullptr);
}

void TrackModel::removeNote(int noteIndex) {
    auto notesNode = trackTree.getChildWithName(notesNodeId);
    if (notesNode.isValid() && noteIndex >= 0 && noteIndex < notesNode.getNumChildren()) {
        notesNode.removeChild(noteIndex, nullptr);
    }
}

int TrackModel::getNumNotes() const {
    auto notesNode = trackTree.getChildWithName(notesNodeId);
    return notesNode.isValid() ? notesNode.getNumChildren() : 0;
}
