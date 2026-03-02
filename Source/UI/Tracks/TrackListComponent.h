#pragma once

#include <JuceHeader.h>
#include "TrackHeaderComponent.h"
#include "../../Core/AppState.h"
#include "../../Model/TrackModel.h"


class TrackListComponent : public juce::Component,
                           private juce::ValueTree::Listener {
 public:
    TrackListComponent();
    ~TrackListComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

 private:
    void rebuildTrackList();
    void updateSelection();
    void setSelectedTrackId(const juce::String& trackId);
    void valueTreeChildAdded(juce::ValueTree& parentTree,
                             juce::ValueTree& childWhichHasBeenAdded) override;
                               
    void valueTreeChildRemoved(juce::ValueTree& parentTree,
                               juce::ValueTree& childWhichHasBeenRemoved,
                               int indexFromWhichChildWasRemoved) override;
                               
    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged,
                                  const juce::Identifier& property) override;

    juce::OwnedArray<TrackModel> trackModels;
    juce::OwnedArray<TrackHeaderComponent> trackHeaders;
    

    // Узел Tracks из AppState
    juce::ValueTree tracksTree;
    
    // Корневой узел Project для отслеживания selectedTrackId
    juce::ValueTree rootTree;

    // Высота одного трека
    static constexpr int kTrackHeight = 60;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackListComponent)
};
