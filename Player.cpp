#include "Player.h"

Player::Player()
{
    formatManager.registerBasicFormats();

    for (auto* btn : { &loadButton, &restartButton, &stopButton, &playButton, &pauseButton, &startButton, &endButton})
    {
        btn->addListener(this);
        addAndMakeVisible(btn);
    }

    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(0.5);
    volumeSlider.addListener(this);
    addAndMakeVisible(volumeSlider);

    setAudioChannels(0, 2);
}

Player::~Player() {
    shutdownAudio();
}

void Player::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void Player::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    transportSource.getNextAudioBlock(bufferToFill);
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
    endButton.setBounds(x, y, z, h);

    volumeSlider.setBounds(20, 100, getWidth() - 40, 30);
}

void Player::buttonClicked(juce::Button* button)
{
    if (button == &loadButton){
        fileChooser = std::make_unique<juce::FileChooser>(
            "Select an audio file...",
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
}

void Player::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &volumeSlider)
        transportSource.setGain((float)slider->getValue());
}