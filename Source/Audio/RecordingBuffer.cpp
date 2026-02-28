#include "RecordingBuffer.h"

RecordingBuffer::RecordingBuffer (int numChannels, int maxS)
    : recordingFifo (maxS)           // FIFO теперь считает сэмплы
    , fifoData (numChannels, maxS)   // Сразу выделяем память на 10 минут!
    , maxSamples (maxS)
{
}

void RecordingBuffer::prepareToRecord (double sampleRate_)
{
    sampleRate = sampleRate_;
    clear();
}

void RecordingBuffer::pushBlock (const juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    // Защита от падений, если микрофон не дал каналов
    if (buffer.getNumChannels() == 0 || fifoData.getNumChannels() == 0) return;

    // Запрашиваем место под numSamples сэмплов
    auto writer = recordingFifo.write (numSamples);

    if (writer.blockSize1 > 0)
    {
        for (int ch = 0; ch < fifoData.getNumChannels(); ++ch)
            fifoData.copyFrom (ch, writer.startIndex1, buffer, ch, startSample, writer.blockSize1);
    }
    
    // Если буфер FIFO закольцевался и нужно дописать остаток в начало
    if (writer.blockSize2 > 0)
    {
        for (int ch = 0; ch < fifoData.getNumChannels(); ++ch)
            fifoData.copyFrom (ch, writer.startIndex2, buffer, ch, startSample + writer.blockSize1, writer.blockSize2);
    }
}

void RecordingBuffer::clear()
{
    recordingFifo.reset();
}

int RecordingBuffer::getTotalSamples() const
{
    return recordingFifo.getNumReady();
}

juce::AudioBuffer<float> RecordingBuffer::getRecordedData()
{
    int numReady = recordingFifo.getNumReady();
    juce::AudioBuffer<float> result (fifoData.getNumChannels(), numReady);

    if (numReady > 0)
    {
        auto reader = recordingFifo.read (numReady);

        if (reader.blockSize1 > 0)
        {
            for (int ch = 0; ch < fifoData.getNumChannels(); ++ch)
                result.copyFrom (ch, 0, fifoData, ch, reader.startIndex1, reader.blockSize1);
        }

        if (reader.blockSize2 > 0)
        {
            for (int ch = 0; ch < fifoData.getNumChannels(); ++ch)
                result.copyFrom (ch, reader.blockSize1, fifoData, ch, reader.startIndex2, reader.blockSize2);
        }
    }

    return result;
}
