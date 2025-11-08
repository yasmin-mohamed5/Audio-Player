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

    void buildShuffle();        // create shuffled order for current playlist
    int nextShuffledIndex();    // returns the next index from shuffleOrder or -1
    void startShuffle(int index);// align shufflePosition with a given currently playing track
    void saveLast();
    void loadLast();

    int nextShuffled();

    juce::AudioTransportSource& getTransportSource(); //
    juce::AudioThumbnail& getThumbnail(); //
    juce::AudioFormatManager& getFormatManager();
    juce::AudioSource* getAudioSource();

    double getStartPoint() const;
    double getEndPoint() const;
    bool isLoopingEnabled() const;
    int getRepeatedTimes() const;
    void resetLoop();
    bool hasTriggeredNext = false;

    // in Player.h
    uint32 lastManualJumpTime = 0; // new



private:

    int order;
    double startPoint, endPoint;
    bool isLooping; //theme
    bool is_restartLoop;
    int repeatedTimes;
    bool isMuted;
    float previousGain;

    int currentTrackIndex = -1;
    void selectTrack(int index);
    bool pinned;
    bool cleared;

    friend class MainComponent;

    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Player)
};
