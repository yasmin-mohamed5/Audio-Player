#include "MainComponent.h"
#include "Player.h"



MainComponent::MainComponent()
{
    
    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(0.5f); // start from 50% of the value
    volumeSlider.addListener(this);

    timeSlider.addListener(this);
    timeSlider.setRange(0.0, 1.0, 0.01);
    timeSlider.setValue(0.0f); //start from time: 0.0 sec

    speedSlider.addListener(this);
    speedSlider.setRange(0.25, 2.0, 0.01);
    speedSlider.setValue(1.0f);

    speedLabel.setText("Speed", juce::dontSendNotification);
    speedLabel.attachToComponent(&speedSlider, true);

    volumeLabel.setText("Volume", juce::dontSendNotification);
    volumeLabel.attachToComponent(&volumeSlider, true);

    positionLabel.setText("Time", juce::dontSendNotification);
    positionLabel.attachToComponent(&timeSlider, true);


    addAndMakeVisible(playlistBox);
    addAndMakeVisible(volumeSlider);
    addAndMakeVisible(timeSlider);
    addAndMakeVisible(speedSlider);
    addAndMakeVisible(speedLabel);
    addAndMakeVisible(positionLabel);
    addAndMakeVisible(volumeLabel);
    addAndMakeVisible(metadataLable);


    addAndMakeVisible(setMarker);
    addAndMakeVisible(setStart);
    addAndMakeVisible(setEnd);
    addAndMakeVisible(repeat_times);

    setMarker.setTextToShowWhenEmpty("order", juce::Colours::grey);
    setStart.setTextToShowWhenEmpty("Start", juce::Colours::grey);
    setEnd.setTextToShowWhenEmpty("End", juce::Colours::grey);
    repeat_times.setTextToShowWhenEmpty("Repeat", juce::Colours::grey);

    metadataLable.setJustificationType(juce::Justification::centred);
    speedLabel.setJustificationType(juce::Justification::centred);
    volumeLabel.setJustificationType(juce::Justification::centred);
    positionLabel.setJustificationType(juce::Justification::centred);

    metadataLable.setColour(juce::Label::textColourId, juce::Colours::white);
    speedLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    volumeLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    positionLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    theme = false;
    currentTrackIndex = -1;
    lastStartClickTime = 0;
    lastEndClickTime = 0;
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}
 

void MainComponent::paint(juce::Graphics& g)
{
    // background color
    if (theme) {
        g.fillAll(juce::Colours::lightgrey);
    }
    else {
        g.fillAll(juce::Colours::darkgrey);
    }
    // song name color & text label color
    if (theme) {
        metadataLable.setColour(juce::Label::textColourId, juce::Colours::black);
        speedLabel.setColour(juce::Label::textColourId, juce::Colours::black);
        volumeLabel.setColour(juce::Label::textColourId, juce::Colours::black);
        positionLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    }
    else {
        metadataLable.setColour(juce::Label::textColourId, juce::Colours::white);
        speedLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        volumeLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        positionLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    }

    double total = thumbnail.getTotalLength();

    juce::Rectangle<int> waveformArea(20, 340, getWidth() - 40, 100);

    if (total > 0.0)
    {
        // waveform color
        if (theme) {
            g.setColour(juce::Colours::deepskyblue);
        }
        else {
            g.setColour(juce::Colours::darkblue);
        }

        // draw only channel 0 (single waveform)
        thumbnail.drawChannel(g, waveformArea, 0.0, total, 0, 0.8f);

        double currentTime = transportSource.getCurrentPosition();
        float x = waveformArea.getX() + (float)((currentTime / total) * waveformArea.getWidth());

        g.setColour(juce::Colours::black); // playhead color
        g.drawLine(x, waveformArea.getY(), x, waveformArea.getBottom(), 3.0f);

        double Time = transportSource.getCurrentPosition();
        int currentMinutes = (int)(Time / 60);
        int currentSeconds = (int)std::fmod(Time, 60.0);


        double total = thumbnail.getTotalLength();
        int totalMinutes = (int)(total / 60);
        int totalSeconds = (int)std::fmod(total, 60.0);


        juce::String timeText = juce::String(currentMinutes) + ":" +
            juce::String(currentSeconds).paddedLeft('0', 2) +
            " / " +
            juce::String(totalMinutes) + ":" +
            juce::String(totalSeconds).paddedLeft('0', 2);

        int textX = waveformArea.getX();
        int textY = waveformArea.getBottom() + 5;

        // time text color in min:sec
        if (theme) {
            g.setColour(juce::Colours::black);
        }
        else {
            g.setColour(juce::Colours::white);
        }
        g.setFont(14.0f);
        g.drawText(timeText, textX, textY, 80, 20, juce::Justification::left);


    }
    else
    {
        // song name color & text label color
        if (theme) {
            metadataLable.setColour(juce::Label::textColourId, juce::Colours::black);
            speedLabel.setColour(juce::Label::textColourId, juce::Colours::black);
            volumeLabel.setColour(juce::Label::textColourId, juce::Colours::black);
            positionLabel.setColour(juce::Label::textColourId, juce::Colours::black);
        }
        else {
            metadataLable.setColour(juce::Label::textColourId, juce::Colours::white);
            speedLabel.setColour(juce::Label::textColourId, juce::Colours::white);
            volumeLabel.setColour(juce::Label::textColourId, juce::Colours::white);
            positionLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        }
        // waveform area color
        if (theme) {
            g.setColour(juce::Colours::deepskyblue);
        }
        else {
            g.setColour(juce::Colours::blue);
        }
        g.drawRect(waveformArea, 2);

        // text color in waveform area before loading sound
        if (theme) {
            g.setColour(juce::Colours::black);
        }
        else {
            g.setColour(juce::Colours::white);
        }
        g.drawText("Load the sound", waveformArea, juce::Justification::centred, false);
    }
    // draw markers
    if (theme) { //marker color
        g.setColour(juce::Colours::black);
    }
    else {
        g.setColour(juce::Colours::white);
    }
    double lengthInSeconds = transportSource.getLengthInSeconds();
    for (auto markTime : marks)
    {

        float x = waveformArea.getX() + (float)((markTime / lengthInSeconds) * waveformArea.getWidth());
        // startX , startY , endX , endY , thickness
        g.drawLine(x, (float)waveformArea.getY(), x, (float)waveformArea.getBottom(), 2.0f);
    }
}

void MainComponent::resized()
{
    // y-> y-axis // x-> x-axis // z-> width // h-> height
    int y = 20;
    int x = 20;
    int z = 80;
    int h = 40;
    int gap = 10;

    loadButton.setBounds(x, y, z, h); x += z + gap;
    loadPlaylistButton.setBounds(x, y, z, h); x += z + gap;
    playlistBox.setBounds(x, y, 200, h); x += 200 + gap;
    playButton.setBounds(x, y, z, h); x += z + gap;
    pauseButton.setBounds(x, y, z, h); x += z + gap;
    stopButton.setBounds(x, y, z, h); x += z + gap;
    startButton.setBounds(x, y, z, h); x += z + gap;
    endButton.setBounds(x, y, z, h); y += h + gap; x = 20;
    restartButton.setBounds(x, y, z, h);  x += z + gap;
    backwardButton.setBounds(x, y, z, h);  x += z + gap;
    forwardButton.setBounds(x, y, z, h); x += z + gap;
    muteButton.setBounds(x, y, z, h); x += z + gap;
    loopButton.setBounds(x, y, z, h); x += z + gap;
    loopStartEndButton.setBounds(x, y, z + 20, h); x += z + (3 * gap);
    setStart.setBounds(x, y, 40, h); x += 45;
    setEnd.setBounds(x, y, 40, h); x += 45;
    repeat_times.setBounds(x, y, z, h); x += z + gap;
    favoriteButton.setBounds(x, y, z, h); y += h + gap; x = 20;
    markerButton.setBounds(x, y, z, h); x += z + gap;
    getmarkerButton.setBounds(x, y, z, h); x += z + gap;
    setMarker.setBounds(x, y, 40, h); x += z + gap - 40;
    themeButton.setBounds(x, y, z, h); x += z + gap;
    pinnButton.setBounds(x, y, z, h); x += z + gap;
    clearButton.setBounds(x, y, z, h); x += z + gap;

    volumeSlider.setBounds(50, 180, getWidth() - 60, 30);
    timeSlider.setBounds(50, 220, getWidth() - 60, 30);
    speedSlider.setBounds(50, 260, getWidth() - 60, 30);
    metadataLable.setBounds(20, 300, getWidth() - 40, 30);

}




