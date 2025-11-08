#pragma once

#include <JuceHeader.h>
#include <vector>


class MainComponent : public juce::Button::Listener,
    public juce::Slider::Listener,
    public juce::AudioAppComponent
{
public:
    MainComponent();
    ~MainComponent() override;



   
    // GUI
    void paint(juce::Graphics& g) override;
    void resized() override;


protected:
    // GUI
    juce::TextButton loadButton{ "Load" };
    juce::TextButton playButton{ "Play |>" };
    juce::TextButton pauseButton{ "Pause ||" };
    juce::TextButton restartButton{ "Restart" };
    juce::TextButton startButton{ "|<|" };
    juce::TextButton endButton{ "|>|" };
    juce::TextButton stopButton{ "Stop" };
    juce::TextButton loopButton{ "Loop" };
    juce::TextButton muteButton{ "Mute" };
    juce::TextButton loopStartEndButton{ "Values to loop" };
    juce::TextButton loadPlaylistButton{ "Load playlist" };
    juce::TextButton markerButton{ "Add Marker" };
    juce::TextButton getmarkerButton{ "Get Marker" };
    juce::TextButton forwardButton{ " +10s" };
    juce::TextButton backwardButton{ " -10s" };
    juce::TextButton pinnButton{ "pin" };
    juce::TextButton favoriteButton{ "Add to Fav" };
    juce::TextButton themeButton{ "Dark" };
    juce::TextButton clearButton{ " Clear" };

    juce::ComboBox playlistBox;

    juce::TextEditor setMarker;
    juce::TextEditor setStart;
    juce::TextEditor setEnd;
    juce::TextEditor repeat_times;

    juce::Slider volumeSlider;
    juce::Slider timeSlider;
    juce::Slider speedSlider;
    juce::Label metadataLable;

    juce::Label speedLabel;
    juce::Label positionLabel;
    juce::Label volumeLabel;

  

    juce::AudioTransportSource transportSource;
    juce::ResamplingAudioSource resampleSource{ &transportSource, false, 2 };

    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;

    juce::AudioThumbnailCache thumbnailCache{ 10 };
    juce::AudioThumbnail thumbnail{ 512, formatManager, thumbnailCache };

    juce::Array<juce::File> playlistFiles;
    int currentTrackIndex;
    int lastStartClickTime;
    int lastEndClickTime;

    std::vector<double> marks; // marker times used by paint()
    bool theme;        // UI theme flag used by paint()


private:

  //Player player;



    //void loadTrack(const juce::File& file);
    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};