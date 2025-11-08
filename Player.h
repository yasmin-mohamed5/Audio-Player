#pragma once
#include <JuceHeader.h>
#include"MainComponent.h"
#include<vector>
using namespace std;


class Player : public MainComponent,
    public juce::Timer,
    public juce::ChangeListener,
    public juce::ComboBox::Listener
    
{
public:



    Player();
    ~Player() override;
	// made functions public for easier access in MainComponent

     // Event handlers
    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* comboBox) override;


    // Audio control
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
    void releaseResources();


    // Time
    void timerCallback() override;


    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void loadPlaylistFiles(const juce::Array<juce::File>& files);

    void saveLast();
    void loadLast();

    juce::AudioTransportSource& getTransportSource(); //
    juce::AudioThumbnail& getThumbnail(); //
    juce::AudioFormatManager& getFormatManager();
    juce::AudioSource* getAudioSource();

    double getStartPoint() const;
    double getEndPoint() const;
    bool isLoopingEnabled() const;
    int getRepeatedTimes() const;
    void resetLoop();


private:
    

    //juce::AudioThumbnail thumbnail{ 512, formatManager, thumbnailCache };
    // Audio
    /*juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    juce::ResamplingAudioSource resampleSource{ &transportSource, false, 2 };
    juce::AudioThumbnailCache thumbnailCache{ 10 };
    juce::AudioThumbnail thumbnail{ 512, formatManager, thumbnailCache };

    juce::Array<juce::File> playlistFiles;
    int currentTrackIndex = -1;
    int lastStartClickTime = 0;
    int lastEndClickTime = 0;*/

    //vector <double> marks; //
    int order;
    double startPoint, endPoint;
    bool isLooping; //theme
    bool is_restartLoop;
    int repeatedTimes;
    bool isMuted;
    float previousGain;


    /*juce::Array<juce::File> playlistFiles;*/

    int currentTrackIndex = -1;

    void selectTrack(int index);
    bool pinned;
    bool cleared;

    friend class MainComponent;

    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Player)
};
