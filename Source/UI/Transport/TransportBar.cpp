#include "TransportBar.h"

TransportBar::TransportBar(AudioEngine& engineToUse)
    : engine(engineToUse) {
    // кнопка Play
    addAndMakeVisible(playButton);
    playButton.onClick = [this] { engine.play(); };
        
    // кнопка Pause
    addAndMakeVisible(pauseButton);
    pauseButton.onClick = [this] {
        engine.stop();
    };

    // кнопка Stop
    addAndMakeVisible(stopButton);
    stopButton.onClick = [this] {
        engine.stop();
        engine.rewind(); // Возвращаем ползунок в начало при остановке
    };

    // кнопка Record
    addAndMakeVisible(recordButton);
    // Делаем ее визуально "залипающей" и назначаем красный цвет активного состояния
    recordButton.setClickingTogglesState(true);
    recordButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFFD0021B));
    recordButton.onClick = [this] {
        bool isRecording = recordButton.getToggleState();
        engine.setRecording(isRecording);
    };
        
    // кнопка Load
    addAndMakeVisible(loadButton);
    loadButton.onClick = [this] {
        // Создаем диалоговое окно (ищем только аудиофайлы)
        fileChooser = std::make_unique<juce::FileChooser>("Select Audio File",
                                                          juce::File::getSpecialLocation(juce::File::userMusicDirectory),
                                                          "*.wav;*.mp3;*.aiff;*.flac");
        
        auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
        
        // Запускаем окно асинхронно (современный подход JUCE)
        fileChooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (file != juce::File{}) {
                engine.loadAndAnalyzeFile(file);
            }
        });
    };

    // Настраиваем дисплей таймкода
    addAndMakeVisible(timecodeLabel);
    timecodeLabel.setFont(juce::Font(22.0f, juce::Font::bold));
    timecodeLabel.setJustificationType(juce::Justification::centredRight);
    timecodeLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF4A90E2));
    timecodeLabel.setText("00:00.000", juce::dontSendNotification);

    // Запускаем опрос AudioEngine на 30 кадрах в секунду
    startTimerHz(30);
}

TransportBar::~TransportBar() {
    stopTimer();
}

void TransportBar::paint(juce::Graphics& g) {
    // Отрисовка фона панели в стиле Flat DAW (цвета из CustomLookAndFeel)
    auto area = getLocalBounds().toFloat();
    g.fillAll(juce::Colour(0xFF1E1E1E));

    // Тонкая линия разделителя сверху
    g.setColour(juce::Colour(0xFF111111));
    g.drawLine(0.0f, 0.0f, area.getWidth(), 0.0f, 2.0f);
}

void TransportBar::resized() {
    auto area = getLocalBounds().reduced(8); // Отступы от краев окна
    
    // Используем FlexBox для современного адаптивного выравнивания
    juce::FlexBox fb;
    fb.justifyContent = juce::FlexBox::JustifyContent::flexStart;
    fb.alignContent   = juce::FlexBox::AlignContent::center;
    
    auto margin = juce::FlexItem::Margin(0, 8, 0, 0); // Отступ справа от каждой кнопки
    
    int btnW = 70;
    fb.items.add(juce::FlexItem(loadButton).withWidth(btnW).withHeight(area.getHeight()).withMargin(margin));
    fb.items.add(juce::FlexItem(playButton).withWidth(btnW).withHeight(area.getHeight()).withMargin(margin));
    fb.items.add(juce::FlexItem(pauseButton).withWidth(btnW).withHeight(area.getHeight()).withMargin(margin));
    fb.items.add(juce::FlexItem(stopButton).withWidth(btnW).withHeight(area.getHeight()).withMargin(margin));
    fb.items.add(juce::FlexItem(recordButton).withWidth(btnW).withHeight(area.getHeight()).withMargin(margin));
    
    // Элемент-распорка (Spacer), чтобы отодвинуть таймкод к правому краю
    fb.items.add(juce::FlexItem().withFlex(1.0f));

    // Добавляем таймкод
    fb.items.add(juce::FlexItem(timecodeLabel).withWidth(120).withHeight(area.getHeight()));

    // Выполняем расстановку
    fb.performLayout(area);
}

void TransportBar::timerCallback()
{
    // 1. Опрашиваем текущее время и форматируем таймкод
    double posSeconds = engine.getCurrentPosition();
    
    int minutes = static_cast<int>(posSeconds) / 60;
    int seconds = static_cast<int>(posSeconds) % 60;
    int millis  = static_cast<int>((posSeconds - static_cast<int>(posSeconds)) * 1000.0);

    // %02d обеспечивает нули в начале, если число меньше 10 (например, "03" вместо "3")
    juce::String timeStr = juce::String::formatted("%02d:%02d.%03d", minutes, seconds, millis);
    
    // Обновляем текст только если он изменился (оптимизация перерисовки)
    if (timecodeLabel.getText() != timeStr)
        timecodeLabel.setText(timeStr, juce::dontSendNotification);

    // 2. Обновляем статус кнопки Play (чтобы она "горела", если воспроизведение идёт, или выключалась, если дошли до конца файла)
    bool isEnginePlaying = engine.isPlaying();
    if (playButton.getToggleState() != isEnginePlaying)
        playButton.setToggleState(isEnginePlaying, juce::dontSendNotification);
}
