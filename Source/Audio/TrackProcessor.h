#pragma once
#include <JuceHeader.h>
#include <atomic>

class TrackProcessor : public juce::PositionableAudioSource
{
public:
    TrackProcessor();
    ~TrackProcessor() override;

    // --- Методы PositionableAudioSource ---
    void setNextReadPosition(juce::int64 newPosition) override;
    juce::int64 getNextReadPosition() const override;
    juce::int64 getTotalLength() const override;
    bool isLooping() const override;
    void setLooping(bool shouldLoop) override;

    // --- Методы AudioSource ---
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    
    // ВАЖНО: Вызывается в RT-потоке. Никаких new/delete/printf/std::vector!
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

    // --- Управление треком ---
    // Установка нового аудио источника "на лету"
    void setSource(std::unique_ptr<juce::PositionableAudioSource> newSource);

    // Управление состояниями (потокобезопасно через std::atomic)
    void setMute(bool shouldMute);
    bool getMute() const;

    void setSolo(bool shouldSolo);
    bool getSolo() const;

    // Для глобальной логики микшера: если другой трек в Solo, этот должен молчать
    void setMutedByOtherSolo(bool isMutedBySolo);

private:
    // Текущий источник аудио (например, AudioFormatReaderSource или RecordingBufferSource)
    std::unique_ptr<juce::PositionableAudioSource> currentSource;

    // Мьютекс для защиты замены currentSource прямо во время воспроизведения
    juce::CriticalSection sourceMutex;

    // Атомарные флаги для lock-free чтения в getNextAudioBlock
    std::atomic<bool> isMuted { false };
    std::atomic<bool> isSoloed { false };
    std::atomic<bool> mutedByOtherSolo { false };

    // Кэшируем параметры для инициализации новых source'ов "на лету"
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackProcessor)
};
