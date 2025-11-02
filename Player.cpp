#include"Player.h"

Player::Player()
{
    formatManager.registerBasicFormats();

    for (auto* btn : { &loadButton, &restartButton, &stopButton, &playButton, &pauseButton, &startButton, &endButton, &muteButton, &loopButton, &loopStartEndButton, &loadPlaylistButton, &forwardButton, &backwardButton })
    {
        btn->addListener(this);
        addAndMakeVisible(btn);
    } previousGain = 0.5f;

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

    playlistBox.addListener(this); // listen for selection change
    addAndMakeVisible(playlistBox);
    addAndMakeVisible(volumeSlider);
    addAndMakeVisible(timeSlider);
    addAndMakeVisible(speedSlider);
    addAndMakeVisible(speedLabel);
    addAndMakeVisible(positionLabel);
    addAndMakeVisible( volumeLabel);
    addAndMakeVisible(metadataLable);
    thumbnail.addChangeListener(this);
    addAndMakeVisible(setStart);
    addAndMakeVisible(setEnd);
    addAndMakeVisible(repeat_times);

    setStart.setTextToShowWhenEmpty("Start", juce::Colours::grey);
    setEnd.setTextToShowWhenEmpty("End", juce::Colours::grey);
    repeat_times.setTextToShowWhenEmpty("Repeat", juce::Colours::grey);

    metadataLable.setJustificationType(juce::Justification::centred);
    metadataLable.setColour(juce::Label::textColourId, juce::Colours::white);

    startTimer(200);//each 200 ms
    setAudioChannels(0, 2);

    is_restartLoop = false;
    isLooping = false;
    startPoint = 0.0;
    endPoint = 0.0;
    repeatedTimes = -1;

    loadLast();
}

Player::~Player() {
    stopTimer();
    shutdownAudio();
    saveLast();
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

    double total = thumbnail.getTotalLength();

    juce::Rectangle<int> waveformArea(20, 340, getWidth() - 40, 100);

    if (total > 0.0)
    {
        g.setColour(juce::Colours::deepskyblue);
        // draw only channel 0 (single waveform)
        thumbnail.drawChannel(g, waveformArea, 0.0, total, 0, 0.8f);

        double currentTime = transportSource.getCurrentPosition();
        float x = waveformArea.getX() + (float)((currentTime / total) * waveformArea.getWidth());

        g.setColour(juce::Colours::black);
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

        g.setColour(juce::Colours::white);
        g.setFont(14.0f);
        g.drawText(timeText, textX, textY, 80, 20, juce::Justification::left);

    }
    else
    {
        g.setColour(juce::Colours::deepskyblue);
        g.drawRect(waveformArea, 2);
        g.setColour(juce::Colours::white);
        g.drawText("Load the sound", waveformArea, juce::Justification::centred, false);
    }
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
    loadPlaylistButton.setBounds(x, y, z, h); x += z + gap;
    playlistBox.setBounds(x, y, 200, h); x += 200 + gap;
    playButton.setBounds(x, y, z, h); x += z + gap;
    pauseButton.setBounds(x, y, z, h); x += z + gap;
    stopButton.setBounds(x, y, z, h); y += h + gap; x = 20;
    restartButton.setBounds(x, y, z, h);  x += z + gap;
    startButton.setBounds(x, y, z, h); x += z + gap;
    endButton.setBounds(x, y, z, h); x += z + gap;
    backwardButton.setBounds(x, y, z, h);  x += z + gap;
    forwardButton.setBounds(x, y, z, h); x += z + gap;
    loopButton.setBounds(x, y, z, h); x += z + gap;
    muteButton.setBounds(x, y, z, h);  y += h + gap; x = 20;
    loopStartEndButton.setBounds(x, y, z + 20, h); x += z + (3 * gap);
    setStart.setBounds(x, y, 40, h); x += 45;
    setEnd.setBounds(x, y, 40, h); x += 45;
    repeat_times.setBounds(x, y, z, h);



    volumeSlider.setBounds(50, 180, getWidth() - 60, 30);
    timeSlider.setBounds(50, 220, getWidth() - 60, 30);
    speedSlider.setBounds(50, 260, getWidth() - 60, 30);
    metadataLable.setBounds(20, 300, getWidth() - 40, 30);
}

void Player::buttonClicked(juce::Button* button)
{
    if (button == &loadPlaylistButton)
    {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Select audio files...", juce::File{}, "*.wav;*.mp3");

        // NEW: Allow multiple file selection
        fileChooser->launchAsync(
            juce::FileBrowserComponent::openMode
            | juce::FileBrowserComponent::canSelectFiles
            | juce::FileBrowserComponent::canSelectMultipleItems,
            [this](const juce::FileChooser& fc)
            {
                auto files = fc.getResults(); //  returns an Array<File>
                if (files.isEmpty())
                    return;

                loadPlaylistFiles(files);
            });
        return;
    }
    if (button == &loadButton) {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Select an audio file...", juce::File{},"*.wav;*.mp3");

        fileChooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file.existsAsFile()){
                juce::Array<juce::File> single{ file };
                loadPlaylistFiles(single);

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

                        thumbnail.setSource(new juce::FileInputSource(file));
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
    else if (button == &muteButton){
        if (!isMuted)
        {
            previousGain = (float)volumeSlider.getValue();
            transportSource.setGain(0.0f);
            volumeSlider.setValue(0.0, juce::dontSendNotification);
            isMuted = true;
            muteButton.setButtonText("Unmute");
            DBG("Muted, previousGain=" << previousGain);
        }
        else{
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
        repeatedTimes = repeat_times.getText().getIntValue();
        if (repeat_times.isEmpty()) {
            repeatedTimes = -1;
        }
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
            repeatedTimes = -1;
            startPoint = 0.0;
            endPoint = 0.0;
            setStart.clear();
            setEnd.clear();
            repeat_times.clear();
            loopStartEndButton.setButtonText("values to loop");
        }
    }
    else if (button == &forwardButton){
        double current = transportSource.getCurrentPosition();
        double total = transportSource.getLengthInSeconds();
        transportSource.setPosition(std::min(current + 10.0, total));
    }
    else if (button == &backwardButton){
        double current = transportSource.getCurrentPosition();
        transportSource.setPosition(std::max(current - 10.0, 0.0));
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
    }
    else if (slider == &speedSlider) {
        float speed = (float)speedSlider.getValue();
        resampleSource.setResamplingRatio(speed);
    }
}

void Player::loadPlaylistFiles(const juce::Array<juce::File>& files)
{
    // Clear previous playlist and UI
    playlistFiles.clear();
    playlistBox.clear();

    // Keep only existing files
    for (auto& f : files)
        if (f.existsAsFile())
            playlistFiles.add(f);

    if (playlistFiles.isEmpty())
        return;

    // Populate ComboBox with names, IDs starting from 1
    int id = 1;
    for (auto& f : playlistFiles)
        playlistBox.addItem(f.getFileNameWithoutExtension(), id++);

    playlistBox.setSelectedId(1, juce::dontSendNotification);  // select first item
    currentTrackIndex = 0;

    // Switch to first track
    selectTrack(currentTrackIndex);
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
        if (isLooping && repeatedTimes && (pos >= endPoint - 0.001 || pos <= startPoint)) { // to avoid cut the last second

            transportSource.setPosition(startPoint);
            transportSource.start();
            pos = transportSource.getCurrentPosition();
            repeatedTimes--;
        }
        // update the GUI time slider
        timeSlider.setValue(pos, juce::dontSendNotification);
        repaint();
    }
}
void Player::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &thumbnail)
    {
        repaint();
    }
}
void Player::comboBoxChanged(juce::ComboBox* comboBox)
{
    if (comboBox == &playlistBox)
    {
        int id = playlistBox.getSelectedId();
        if (id > 0)
            selectTrack(id - 1); // map ComboBox ID to zero-based index
    }
}
void Player::selectTrack(int index)
{
    if (index < 0 || index >= playlistFiles.size())
        return;

    auto file = playlistFiles[index];

    // Stop and detach previous source
    transportSource.stop();
    transportSource.setSource(nullptr);
    readerSource.reset();

    // Create a new reader for the selected file
    if (auto* reader = formatManager.createReaderFor(file))
    {
        readerSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
        transportSource.setSource(readerSource.get(), 0, nullptr, reader->sampleRate);

        // Update thumbnail to show the selected file
        thumbnail.setSource(new juce::FileInputSource(file));

        // Update time slider range
        double lengthInSeconds = transportSource.getLengthInSeconds();
        timeSlider.setRange(lengthInSeconds > 0.0 ? 0.0 : 0.0,
            lengthInSeconds > 0.0 ? lengthInSeconds : 1.0,
            0.01);
        timeSlider.setValue(0.0, juce::dontSendNotification);

        // Update metadata label (title/artist or filename + duration)
        juce::String displayText;
        auto metadata = reader->metadataValues;
        if (metadata.containsKey("title"))  displayText = metadata["title"];
        if (metadata.containsKey("artist")) displayText += " - " + metadata["artist"];
        if (displayText.isEmpty())          displayText = file.getFileNameWithoutExtension();

        if (lengthInSeconds > 0.0)
        {
            int minutes = (int)(lengthInSeconds / 60);
            int seconds = (int)std::fmod(lengthInSeconds, 60.0);
            displayText += "  [" + juce::String(minutes) + ":" +
                juce::String(seconds).paddedLeft('0', 2) + "]";
        }

        metadataLable.setText(displayText, juce::dontSendNotification);

        // Optionally auto-start on selection:
        transportSource.start();
    }
    currentTrackIndex = index;
}

void Player::saveLast()
{
    if (!readerSource) return;

    juce::File sessionFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                             .getChildFile("last_session.txt");

    juce::String filePath = playlistFiles.isEmpty() ? "" : playlistFiles[currentTrackIndex].getFullPathName();
    double position = transportSource.getCurrentPosition();

    juce::String content = filePath + "\n" + juce::String(position);
    sessionFile.replaceWithText(content);
}
void Player::loadLast()
{
    juce::File sessionFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                             .getChildFile("last_session.txt");

    if (!sessionFile.existsAsFile()) return;

    juce::StringArray lines = juce::StringArray::fromLines(sessionFile.loadFileAsString());
    if (lines.size() < 2) return;

    juce::File lastFile(lines[0]);
    double lastPosition = lines[1].getDoubleValue();

    if (lastFile.existsAsFile())
    {
        juce::Array<juce::File> single{ lastFile };
        loadPlaylistFiles(single);
        transportSource.setPosition(lastPosition);
    }
}
