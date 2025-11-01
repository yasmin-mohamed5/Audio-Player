#pragma once
#include <JuceHeader.h>

class Player : public juce::AudioAppComponent,
    public juce::Button::Listener,
    public juce::Slider::Listener,
    public juce::Timer
{
public:
    Player();
    ~Player() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Event handlers
    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;

    // Audio control
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
    void releaseResources();
    
    // Time
    void timerCallback() override;

private:
    // Audio
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    juce::ResamplingAudioSource resampleSource{ &transportSource, false, 2 };


    // GUI
    juce::TextButton loadButton{ "Load" };
    juce::TextButton playButton{ "Play ►" };
    juce::TextButton pauseButton{ "Pause ||" };
    juce::TextButton restartButton{ "Restart" };
    juce::TextButton startButton{ "|◄" };
    juce::TextButton endButton{ "►|" };
    juce::TextButton stopButton{ "Stop" };
    juce::TextButton loopButton{ "Loop" };
    juce::TextButton muteButton{ "Mute" };
    juce::TextButton loopStartEndButton{ "values to loop" };
    juce::TextEditor setStart;
    juce::TextEditor setEnd;
    double startPoint, endPoint;
	bool isLooping;
    bool is_restartLoop;
    bool isMuted = false;
    float previousGain = 0.5f;
    juce::Slider volumeSlider;
    juce::Slider timeSlider;
    juce::Slider speedSlider;
    juce::Label metadataLable;

    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Player)
};
