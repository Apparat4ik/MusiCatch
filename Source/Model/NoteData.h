#pragma once

#include <JuceHeader.h>

/**
 * Структура данных, представляющая одну ноту.
 * Используется как DTO (Data Transfer Object) для ValueTree.
 */
struct NoteData {
    float startTime = 0.0f;  // Время начала в секундах
    float duration = 0.0f;   // Длительность в секундах
    int midiPitch = 60;      // Высота тона (0-127, 60 = C4)
    float velocity = 0.8f;   // Громкость/сила нажатия (0.0 - 1.0)

    // Оператор сравнения для тестов и удобства
    bool operator== (const NoteData& other) const
    {
        return startTime == other.startTime &&
               duration == other.duration &&
               midiPitch == other.midiPitch &&
               velocity == other.velocity;
    }
};


 // Специализация VariantConverter для автоматической сериализации NoteData <-> juce::var.
template<>
struct juce::VariantConverter<NoteData> {
    static NoteData fromVar (const juce::var& v) {
        NoteData n;
        // Если var - это DynamicObject (JSON-like), читаем свойства
        if (auto* obj = v.getDynamicObject())
        {
            n.startTime = (float) obj->getProperty ("s");
            n.duration  = (float) obj->getProperty ("d");
            n.midiPitch = (int)   obj->getProperty ("p");
            n.velocity  = (float) obj->getProperty ("v");
        }
        return n;
    }

    static juce::var toVar (const NoteData& n) {
        auto* obj = new juce::DynamicObject();
        obj->setProperty ("s", n.startTime);
        obj->setProperty ("d", n.duration);
        obj->setProperty ("p", n.midiPitch);
        obj->setProperty ("v", n.velocity);
        
        return juce::var (obj);
    }
};
