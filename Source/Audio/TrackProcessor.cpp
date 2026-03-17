#include "TrackProcessor.h"

TrackProcessor::TrackProcessor() = default;

TrackProcessor::~TrackProcessor() {
    setSource(nullptr);
}

void TrackProcessor::setSource(std::unique_ptr<juce::PositionableAudioSource> newSource) {
    // Блокируем доступ для audio thread, пока меняем указатель
    const juce::ScopedLock sl(sourceMutex);

    if (currentSource != nullptr)
        currentSource->releaseResources();

    currentSource = std::move(newSource);

    // Если движок уже работает, нужно подготовить новый сурс сразу же
    if (currentSource != nullptr && currentSampleRate > 0.0) {
        currentSource->prepareToPlay(currentBlockSize, currentSampleRate);
    }
}

void TrackProcessor::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
    const juce::ScopedLock sl(sourceMutex);
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlockExpected;

    if (currentSource != nullptr)
        currentSource->prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void TrackProcessor::releaseResources() {
    const juce::ScopedLock sl(sourceMutex);
    if (currentSource != nullptr)
        currentSource->releaseResources();
}

void TrackProcessor::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) {
    bool muteActive = isMuted.load(std::memory_order_relaxed) ||
                      mutedByOtherSolo.load(std::memory_order_relaxed);

    if (sourceMutex.tryEnter()) {
        // сначала читаем следующий блок, чтобы внутренний таймер
        // звуковой дорожки не остановился
        if (currentSource != nullptr) {
            currentSource->getNextAudioBlock(bufferToFill);

            if (muteActive) {
                bufferToFill.clearActiveBufferRegion();
            }
            
        } else {
            bufferToFill.clearActiveBufferRegion(); // Нет источника - тишина
        }
        sourceMutex.exit();
    }
    else {
        // Мьютекс занят (идет загрузка файла), выдаем тишину
        bufferToFill.clearActiveBufferRegion();
    }
}


void TrackProcessor::setNextReadPosition(juce::int64 newPosition) {
    const juce::ScopedLock sl(sourceMutex);
    if (currentSource != nullptr)
        currentSource->setNextReadPosition(newPosition);
}

juce::int64 TrackProcessor::getNextReadPosition() const {
    const juce::ScopedLock sl(sourceMutex);
    return currentSource != nullptr ? currentSource->getNextReadPosition() : 0;
}

juce::int64 TrackProcessor::getTotalLength() const {
    const juce::ScopedLock sl(sourceMutex);
    return currentSource != nullptr ? currentSource->getTotalLength() : 0;
}

bool TrackProcessor::isLooping() const {
    const juce::ScopedLock sl(sourceMutex);
    return currentSource != nullptr && currentSource->isLooping();
}

void TrackProcessor::setLooping(bool shouldLoop) {
    const juce::ScopedLock sl(sourceMutex);
    if (currentSource != nullptr)
        currentSource->setLooping(shouldLoop);
}

// Управление состояниями
void TrackProcessor::setMute(bool shouldMute) { isMuted.store(shouldMute, std::memory_order_relaxed); }
bool TrackProcessor::getMute() const { return isMuted.load(std::memory_order_relaxed); }

void TrackProcessor::setSolo(bool shouldSolo) { isSoloed.store(shouldSolo, std::memory_order_relaxed); }
bool TrackProcessor::getSolo() const { return isSoloed.load(std::memory_order_relaxed); }

void TrackProcessor::setMutedByOtherSolo(bool isMutedBySolo) { mutedByOtherSolo.store(isMutedBySolo, std::memory_order_relaxed); }
