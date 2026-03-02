#include "TransportBar.h"
#include "../../Utils/AudioExporter.h"

TransportBar::TransportBar(AudioEngine& engineToUse)
    : engine(engineToUse) {
    
    addAndMakeVisible(loadButton);
    addAndMakeVisible(saveButton);
    addAndMakeVisible(playButton);
    addAndMakeVisible(pauseButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(recordButton);

    playButton.onClick = [this] {
        engine.play();
        playButton.setToggleState(true, juce::dontSendNotification);
    };
    
    pauseButton.onClick = [this] {
        engine.stop();
        playButton.setToggleState(false, juce::dontSendNotification);
    };
    
    stopButton.onClick = [this] {
        engine.stop();
        engine.rewind();
        playButton.setToggleState(false, juce::dontSendNotification);
    };
    
    recordButton.setClickingTogglesState(true);
    recordButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFFD0021B));
    recordButton.onClick = [this] { engine.setRecording(recordButton.getToggleState()); };
        
    loadButton.onClick = [this] {
        fileChooser = std::make_unique<juce::FileChooser>("Select Audio File",
                                                          juce::File::getSpecialLocation(juce::File::userMusicDirectory),
                                                          "*.wav;*.mp3;*.aiff;*.flac");
        
        auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
        fileChooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (file != juce::File{}) {
                engine.loadAndAnalyzeFile(file);
            }
        });
    };

    saveButton.onClick = [this] {
        fileChooser = std::make_unique<juce::FileChooser>("Save Project as Wav",
                                                          juce::File::getSpecialLocation(juce::File::userMusicDirectory).getChildFile("MyProject.wav"),
                                                          "*.wav");
                                                          
        auto chooserFlags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles;
        fileChooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (file != juce::File{}) {
                AudioExporter::exportProjectToWav(engine, file);
            }
        });
    };
}

TransportBar::~TransportBar() {}

void TransportBar::paint(juce::Graphics& g) {
    auto area = getLocalBounds().toFloat();
    g.fillAll(juce::Colour(0xFF1E1E1E));
    g.setColour(juce::Colour(0xFF111111));
    g.drawLine(0.0f, 0.0f, area.getWidth(), 0.0f, 2.0f);
}

void TransportBar::resized() {
    auto area = getLocalBounds().reduced(8);
    
    juce::Grid grid;
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;

    grid.templateRows    = { Track(Fr(1)), Track(Fr(1)) };
    grid.templateColumns = { Track(Fr(1)), Track(Fr(1)), Track(Fr(1)) };
    
    grid.columnGap = juce::Grid::Px(4);
    grid.rowGap    = juce::Grid::Px(4);

    grid.items = {
        juce::GridItem(loadButton),
        juce::GridItem(saveButton),
        juce::GridItem(recordButton),
        juce::GridItem(playButton),
        juce::GridItem(pauseButton),
        juce::GridItem(stopButton)
    };

    grid.performLayout(area);
}
