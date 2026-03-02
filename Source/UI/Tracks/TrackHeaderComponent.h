#pragma once

#include <JuceHeader.h>
#include "../../Model/TrackModel.h"


class TrackHeaderComponent : public juce::Component,
                             private juce::ValueTree::Listener {
 public:
    std::function<void()> onSelected;

    explicit TrackHeaderComponent(TrackModel& model);
    ~TrackHeaderComponent() override;

    // Выделение трека (активный = синий, остальные = серые).
    void setSelected(bool shouldBeSelected);
    bool isSelected() const noexcept { return selected; }

    // juce::Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;

private:
    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged,
                                  const juce::Identifier& property) override;

    void updateButtonStates();


    TrackModel& trackModel;
    bool selected = false;

    juce::Label  nameLabel;
                                 
    juce::TextButton exportMidiButton { "MIDI" };
    juce::TextButton muteButton  { "M" };
    juce::TextButton soloButton  { "S" };
                                 
    std::unique_ptr<juce::FileChooser> fileChooser;

    // Цветной прямоугольник-маркер (левый край панели)
    static constexpr int kColorMarkerWidth = 4;
    static constexpr int kButtonSize       = 24;
    static constexpr int kPadding          = 6;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackHeaderComponent)
};
