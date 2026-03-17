#include "TrackHeaderComponent.h"
#include "../../Utils/AudioExporter.h"

TrackHeaderComponent::TrackHeaderComponent(TrackModel& model) : trackModel(model) {

    nameLabel.setText(trackModel.getName(), juce::dontSendNotification);
    nameLabel.setFont(juce::Font(14.0f, juce::Font::plain));
    nameLabel.setJustificationType(juce::Justification::centredLeft);
    nameLabel.setEditable(/*single-click*/ false, /*double-click*/ true);
    nameLabel.onTextChange = [this] {
        trackModel.setName(nameLabel.getText());
    };
    addAndMakeVisible(nameLabel);
    
    exportMidiButton.setTooltip("Export track notes to MIDI");
    exportMidiButton.onClick = [this] {
        
        // Подставляем имя трека как название файла по умолчанию
        juce::String defaultFileName = trackModel.getName() + ".mid";
        
        fileChooser = std::make_unique<juce::FileChooser>(
            "Save Track as MIDI",
            juce::File::getSpecialLocation(juce::File::userMusicDirectory).getChildFile(defaultFileName),
            "*.mid");

        auto chooserFlags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles;
        
        // Используем асинхронный запуск окна ОС
        fileChooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (file != juce::File{}) {
                // Передаем модель текущего трека в статический метод экспортера
                AudioExporter::exportTrackToMidi(trackModel, file);
            }
        });
    };
    addAndMakeVisible(exportMidiButton);

    // Mute button
    muteButton.setClickingTogglesState(true);
    muteButton.setTooltip("Mute track");
    muteButton.onClick = [this] {
        trackModel.setMuted(muteButton.getToggleState());
    };
    addAndMakeVisible(muteButton);

    // Solo button
    soloButton.setClickingTogglesState(true);
    soloButton.setTooltip("Solo track");
    soloButton.onClick = [this] {
        trackModel.setSoloed(soloButton.getToggleState());
    };
    addAndMakeVisible(soloButton);

    updateButtonStates();

    // Подписываемся на изменения ValueTree
    // любое внешнее изменение (Undo/Redo, другой UI) вызовет repaint().
    trackModel.getTrackTree().addListener(this);
}

TrackHeaderComponent::~TrackHeaderComponent() {
    // Обязательно отписываемся, чтобы избежать dangling pointer.
    trackModel.getTrackTree().removeListener(this);
}

void TrackHeaderComponent::setSelected(bool shouldBeSelected) {
    if (selected == shouldBeSelected)
        return;

    selected = shouldBeSelected;
    repaint();
}

void TrackHeaderComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds();

    // синий если выбран, иначе тёмно-серый
    const juce::Colour bgColour = selected
        ? juce::Colour(0xFF1A6EBF)   // активный трек — синий
        : juce::Colour(0xFF2D2D2D);  // неактивный    — тёмно-серый
    g.fillAll(bgColour);

    // Цветной маркер слева
    g.setColour(trackModel.getColour());
    g.fillRect(bounds.removeFromLeft(kColorMarkerWidth));

    // Нижняя разделительная линия
    g.setColour(juce::Colour(0xFF1A1A1A));
    g.drawLine(0.0f,
               static_cast<float>(getHeight() - 1),
               static_cast<float>(getWidth()),
               static_cast<float>(getHeight() - 1),
               1.0f);
}


void TrackHeaderComponent::resized() {
    auto area = getLocalBounds()
                    .reduced(kPadding)
                    .withTrimmedLeft(kColorMarkerWidth);

    // Размещаем кнопки справа налево: Solo -> Mute -> MIDI
    soloButton.setBounds(area.removeFromRight(kButtonSize)
                             .withSizeKeepingCentre(kButtonSize, kButtonSize));
    area.removeFromRight(kPadding / 2);

    muteButton.setBounds(area.removeFromRight(kButtonSize)
                             .withSizeKeepingCentre(kButtonSize, kButtonSize));
    area.removeFromRight(kPadding / 2);

    // Кнопку MIDI делаем чуть шире
    int midiButtonWidth = 36;
    exportMidiButton.setBounds(area.removeFromRight(midiButtonWidth)
                                   .withSizeKeepingCentre(midiButtonWidth, kButtonSize));
    area.removeFromRight(kPadding);

    // Оставшееся слева место — лейбл с именем
    nameLabel.setBounds(area);
}


void TrackHeaderComponent::mouseDown(const juce::MouseEvent&) {
    if (onSelected)
        onSelected();
}

// ValueTree::Listener
void TrackHeaderComponent::valueTreePropertyChanged(juce::ValueTree&,
                                                    const juce::Identifier& property) {
    
    if (property == juce::Identifier("name")) {
        nameLabel.setText(trackModel.getName(), juce::dontSendNotification);
    }

    // Для mute/solo/color достаточно repaint() + обновить состояние кнопок.
    updateButtonStates();
    repaint();
}


void TrackHeaderComponent::updateButtonStates() {
    muteButton.setToggleState(trackModel.isMuted(),
                              juce::dontSendNotification);
    soloButton.setToggleState(trackModel.isSoloed(),
                              juce::dontSendNotification);
}
