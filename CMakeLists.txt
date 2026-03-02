cmake_minimum_required(VERSION 3.20)
project(MusiCatch VERSION 1.0.0)

# 1. Автоматически скачиваем JUCE 
include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4 # Используем актуальную версию
)
FetchContent_MakeAvailable(juce)

# 2. Создаем GUI-приложение
juce_add_gui_app(MusiCatch
    PRODUCT_NAME "MusiCatch"
    VERSION 1.0.0)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-x11-3.0)
    target_include_directories(MusiCatch PRIVATE ${GTK3_INCLUDE_DIRS})
    target_link_libraries(MusiCatch PRIVATE ${GTK3_LIBRARIES})
endif()

# Отключаем встроенный браузер JUCE (он тянет WebKit и усложняет сборку)
target_compile_definitions(MusiCatch PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)


# 3. Добавляем все исходные файлы
target_sources(MusiCatch PRIVATE
    Source/Main.cpp
    Source/MainComponent.cpp
    Source/Core/AppState.cpp
    Source/Audio/AudioEngine.cpp
    Source/Audio/TrackProcessor.cpp
    Source/Audio/RecordingBuffer.cpp
    Source/Audio/Analysis/PitchDetector.cpp
    Source/Model/TrackModel.cpp
    Source/UI/Styles/CustomLookAndFeel.cpp
    Source/UI/Tracks/TrackListComponent.cpp
    Source/UI/Tracks/TrackHeaderComponent.cpp
    Source/UI/PianoRoll/PianoRollComponent.cpp
    Source/UI/PianoRoll/PianoKeyboardComponent.cpp
    Source/UI/PianoRoll/PlayheadOverlay.cpp
    Source/UI/PianoRoll/NoteComponent.cpp
    Source/UI/Transport/TransportBar.cpp
    Source/UI/Transport/TimelineComponent.cpp
    Source/Utils/AudioImporter.cpp
    Source/Utils/AudioExporter.cpp
)

# 4. Подключаем необходимые модули JUCE
target_link_libraries(MusiCatch PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_dsp
)

# 5. Указываем стандарт C++
juce_generate_juce_header(MusiCatch)
target_compile_features(MusiCatch PUBLIC cxx_std_17)
