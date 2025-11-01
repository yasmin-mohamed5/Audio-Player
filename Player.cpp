#include"Player.h"

Player::Player()
{
    formatManager.registerBasicFormats();

    for (auto* btn : { &loadButton, &restartButton, &stopButton, &playButton, &pauseButton, &startButton, &endButton, &muteButton, &loopButton, &loopStartEndButton})
    {
        btn->addListener(this);
        addAndMakeVisible(btn);
    }previousGain = 0.5f;

    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(0.5); // start from 50% of the value
    volumeSlider.addListener(this);

    timeSlider.addListener(this);
    timeSlider.setRange(0.0, 1.0, 0.01);
    timeSlider.setValue(0.0); //start from time: 0.0 sec

    speedSlider.addListener(this);
    speedSlider.setRange(0.05, 2.0, 0.01);
    speedSlider.setValue(1.0); 

    addAndMakeVisible(volumeSlider);
    addAndMakeVisible(timeSlider);
    addAndMakeVisible(speedSlider);
    addAndMakeVisible(metadataLable);
    addAndMakeVisible(setStart);
    addAndMakeVisible(setEnd);

    setStart.setTextToShowWhenEmpty("start", juce::Colours::grey);
    setEnd.setTextToShowWhenEmpty("start", juce::Colours::grey);

    metadataLable.setJustificationType(juce::Justification::centred);
    metadataLable.setColour(juce::Label::textColourId, juce::Colours::white);

    startTimer(200);//each 200 ms
    setAudioChannels(0, 2);
    is_restartLoop = false;
    isLooping = false;
    startPoint = 0.0;
	endPoint = 0.0;
}

Player::~Player() {
    stopTimer();
    shutdownAudio();
}

void Player::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    resampleSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void Player::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    resampleSource.getNextAudioBlock(bufferToFill);
}

void Player::releaseResources()
{
    transportSource.releaseResources();
}

void Player::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

void Player::resized()
{
    // y-> y-axis // x-> x-axis // z-> width // h-> height
    int y = 20;
    int x = 20;
    int z = 80;
    int h = 40;
    int gap = 10;

    loadButton.setBounds(x, y, z, h); x += z + gap;
    playButton.setBounds(x, y, z, h); x += z + gap;
    pauseButton.setBounds(x, y, z, h); x += z + gap;
    restartButton.setBounds(x, y, z, h); x += z + gap;
    stopButton.setBounds(x, y, z, h); x += z + gap;
    startButton.setBounds(x, y, z, h); x += z + gap;
    muteButton.setBounds(x, y, z, h);  y += 60; x = 20;
    loopButton.setBounds(x, y, z, h); x += z + gap;
    endButton.setBounds(x, y, z, h); x += z + gap;
	loopStartEndButton.setBounds(x, y, z+20, h); x += z + (3*gap);
	setStart.setBounds(x, y, 40, h); x += 45;
	setEnd.setBounds(x, y, 40, h); x += z + gap;
    
    volumeSlider.setBounds(20, 160, getWidth() - 40, 30);
    timeSlider.setBounds(20, 200, getWidth() - 40, 30); 
    speedSlider.setBounds(20, 240, getWidth() - 40, 30);
    metadataLable.setBounds(20, 280, getWidth() - 40, 30);
}

void Player::buttonClicked(juce::Button* button)
{
    if (button == &loadButton) {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Select an audio file...", \

            juce::File{},
            "*.wav;*.mp3");
        fileChooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file.existsAsFile())
                {
                    if (auto* reader = formatManager.createReaderFor(file))
                    {
                        // 🔑 Disconnect old source first
                        transportSource.stop();
                        transportSource.setSource(nullptr);
                        readerSource.reset();

                        // Create new reader source
                        readerSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);

                        // Attach safely
                        transportSource.setSource(readerSource.get(),
                            0,
                            nullptr,
                            reader->sampleRate);
                        double lengthInSeconds = transportSource.getLengthInSeconds();
                        if (lengthInSeconds > 0.0) {
                            timeSlider.setRange(0.0, lengthInSeconds, 0.01); // set rage in seconds to the slider
                        }
                        else {
                            timeSlider.setRange(0.0, 1.0, 0.01);
                        }
                        timeSlider.setValue(0.0, juce::dontSendNotification);
                        juce::String displayText;

                        // Try to read metadata
                        auto metadata = reader->metadataValues;
                        if (metadata.containsKey("title"))
                            displayText = metadata["title"];
                        if (metadata.containsKey("artist"))
                            displayText += " - " + metadata["artist"];

                        // If nothing found, fallback to filename
                        if (displayText.isEmpty())
                            displayText = file.getFileNameWithoutExtension();

                        // Add duration
                        if (lengthInSeconds > 0.0)
                        {
                            int minutes = (int)(lengthInSeconds / 60);
                            int seconds = (int)std::fmod(lengthInSeconds, 60.0);
                            displayText += "  [" + juce::String(minutes) + ":" +
                                juce::String(seconds).paddedLeft('0', 2) + "]";
                        }

                        // Update label
                        metadataLable.setText(displayText, juce::dontSendNotification);
                        transportSource.start();
                    }
                }
            });
    }
    else if (button == &playButton) {
        transportSource.start();
    }
    else if (button == &pauseButton) {
        transportSource.stop();
    }
    else if (button == &restartButton) {
        transportSource.setPosition(0.0);
        transportSource.start();
    }
    else if (button == &stopButton) {
        transportSource.stop();
        transportSource.setPosition(0.0);
    }
    else if (button == &startButton) {
        transportSource.setPosition(0.0);
    }
    else if (button == &endButton) {
        if (readerSource != nullptr) {
            double lengthInSeconds = transportSource.getLengthInSeconds();
            transportSource.setPosition(lengthInSeconds); // jump to end
        }
    }
    else if (button == &muteButton)
    {
        if (!isMuted)
        {
            previousGain = (float)volumeSlider.getValue();
            transportSource.setGain(0.0f);
            volumeSlider.setValue(0.0, juce::dontSendNotification);
            isMuted = true;
            muteButton.setButtonText("Unmute");
            DBG("Muted, previousGain=" << previousGain);
        }
        else
        {
            transportSource.setGain(previousGain);
            volumeSlider.setValue(previousGain, juce::dontSendNotification);
            isMuted = false;
            muteButton.setButtonText("Mute");
        }
    }
    else if (button == &loopButton) {
        is_restartLoop = !is_restartLoop;
        if (is_restartLoop) {
            loopButton.setButtonText("Looping");
        }
        else {
            loopButton.setButtonText("Loop");
        }
    }
    else if (button == &loopStartEndButton) {
        isLooping = !isLooping;
        startPoint = setStart.getText().getDoubleValue();
        endPoint = setEnd.getText().getDoubleValue();
        if (startPoint == endPoint) {
			isLooping = false;
        }
        double lengthInSeconds = transportSource.getLengthInSeconds();
        if (startPoint < 0) {
            startPoint = 0;
        }
        if (endPoint > lengthInSeconds) {
            endPoint = lengthInSeconds;
        }
        if (startPoint > endPoint) {
            double x = startPoint;
            startPoint = endPoint;
            endPoint = x;
        }
        if (isLooping) {
            loopStartEndButton.setButtonText("Looping");
        }
        else {
            startPoint = 0.0;
            endPoint = 0.0;
            setStart.clear();
            setEnd.clear();
            loopStartEndButton.setButtonText("values to loop");
        }
    }


}

void Player::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &volumeSlider) {
        float v = (float)slider->getValue();
        transportSource.setGain(v);

        if (v > 0.001f) {
            previousGain = v;
            isMuted = false;
            muteButton.setButtonText("Mute");
        }
        else {
            isMuted = true;
            muteButton.setButtonText("Unmute");
        }
    }
    else if (slider == &timeSlider) {
        transportSource.setPosition((float)slider->getValue());
    } else if (slider == &speedSlider) {
        float speed = (float)speedSlider.getValue();
        resampleSource.setResamplingRatio(speed);
    }
}


void Player::timerCallback()
{
    // update the time slider to reflect current playback position
    if (readerSource != nullptr) {
        double pos = transportSource.getCurrentPosition();
        double lengthInSeconds = transportSource.getLengthInSeconds();
        if (is_restartLoop && pos >= lengthInSeconds - 0.001) { // to avoid cut the last second 
            transportSource.setPosition(0.0);
            transportSource.start();
            pos = transportSource.getCurrentPosition();
        }
        if (isLooping && (pos >= endPoint - 0.001 || pos <= startPoint)) { // to avoid cut the last second 
            
            transportSource.setPosition(startPoint);
            transportSource.start();
            pos = transportSource.getCurrentPosition();
        }
        // update the GUI time slider
        timeSlider.setValue(pos, juce::dontSendNotification);
    }
}
