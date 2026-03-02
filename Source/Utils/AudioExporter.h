#pragma once

#include <JuceHeader.h>
#include "../Audio/AudioEngine.h"
#include "../Model/TrackModel.h"

class AudioExporter {
public:
    // Экспорт всего микса в WAV
    static bool exportProjectToWav(AudioEngine& engine, const juce::File& targetFile);

    // Экспорт распознанных нот выбранного трека в MIDI
    static bool exportTrackToMidi(const TrackModel& track, const juce::File& targetFile);
private:
    AudioExporter() = delete; // Запрещаем создание экземпляров
};
