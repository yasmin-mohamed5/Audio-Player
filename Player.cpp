#include"Player.h"
#include <random>
#include <algorithm>


Player::Player()
{
    for (auto* btn : { &loadButton, &restartButton, &stopButton, &playButton, &pauseButton, &startButton, &endButton, &muteButton, &loopButton, &loopStartEndButton, &loadPlaylistButton, &markerButton, &getmarkerButton, &forwardButton, &backwardButton, &favoriteButton, &themeButton, &pinnButton, &clearButton, &shuffleButton })
    {
        btn->addListener(this);
        addAndMakeVisible(btn);
    }
    playlistBox.addListener(this); // listen for selection change
    thumbnail.addChangeListener(this);
    formatManager.registerBasicFormats();
    previousGain = 0.5f;
    startTimer(200);//each 200 ms
    setAudioChannels(0, 2);

    is_restartLoop = false;
    isLooping = false;
    startPoint = 0.0;
    endPoint = 0.0;
    repeatedTimes = -1;
    order = -1;
    
    cleared = false;
    isMuted = false;
    pinned = true;
    loadLast();
}

Player::~Player() {
    stopTimer();
    shutdownAudio();
    saveLast();
}

#include <random> // at top of Player.cpp

void Player::buildShuffle()
{
    shuffleOrder.clear();
    int n = playlistFiles.size();
    if (n == 0) return;
    shuffleOrder.reserve(n);
    for (int i = 0; i < n; ++i) shuffleOrder.push_back(i);

    // Shuffle using std::shuffle + random_device
    random_device rd;
    mt19937 g(rd());
    shuffle(shuffleOrder.begin(), shuffleOrder.end(), g);

    // Ensure the currently playing track is placed at current position
    if (currentTrackIndex >= 0)
        startShuffle(currentTrackIndex);
}

void Player::startShuffle(int index)
{
    // Find current track in shuffleOrder and set shufflePosition to it
    for (int i = 0; i < shuffleOrder.size(); ++i)
    {
        if (shuffleOrder[i] == index)
        {
            shufflePosition = i;
            return;
        }
    }
    // If not found (rare), set position to 0
    shufflePosition = 0;
}

int Player::nextShuffledIndex()
{
    if (shuffleOrder.empty()) return -1;
    // advance position (wrap)
    shufflePosition = (shufflePosition + 1) % shuffleOrder.size();
    return shuffleOrder[shufflePosition];
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
                buildShuffle();
            });
        return;
    }
    if (button == &loadButton) {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Select an audio file...", juce::File{}, "*.wav;*.mp3");

        fileChooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file.existsAsFile()) {
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
        auto now = juce::Time::getMillisecondCounter();

        if (now - lastStartClickTime < 1000) // double press within 1s
        {
            // Previous track
            if (currentTrackIndex > 0)
                selectTrack(currentTrackIndex - 1);
        }
        else
        {
            // Normal single press: jump to start of current track
            transportSource.setPosition(0.0);
        }
        lastStartClickTime = now;
    }
    else if (button == &endButton) {
        auto now = juce::Time::getMillisecondCounter();

        if (now - lastEndClickTime < 500) // double click: next track
        {
            int nextIndex = -1;
            if (isShuffling && playlistFiles.size() > 1)
                nextIndex = nextShuffledIndex();
            else if (currentTrackIndex + 1 < playlistFiles.size())
                nextIndex = currentTrackIndex + 1;

            if (nextIndex >= 0)
                selectTrack(nextIndex);
        }
        else // single click: go to end of current track
        {
            if (readerSource != nullptr)
            {
                double lengthInSeconds = transportSource.getLengthInSeconds();
                transportSource.setPosition(lengthInSeconds - 0.01);
                lastManualJumpTime = now; // remember when we jumped
            }
        }

        lastEndClickTime = now;
    }
    else if (button == &muteButton) {
        if (!isMuted)
        {
            previousGain = (float)volumeSlider.getValue();
            transportSource.setGain(0.0f);
            volumeSlider.setValue(0.0, juce::dontSendNotification);
            isMuted = true;
            muteButton.setButtonText("Unmute");
            DBG("Muted, previousGain=" << previousGain);
        }
        else {
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
        if (repeat_times.isEmpty() || repeatedTimes <= 0) {
            repeatedTimes = -1;
            repeat_times.clear();
        }
        if (startPoint == endPoint) {
            isLooping = false;
        }
        double lengthInSeconds = transportSource.getLengthInSeconds();
        if (startPoint < 0 || startPoint >= lengthInSeconds ){
            startPoint = 0;
            setStart.setText("0", juce::dontSendNotification);
        }
        if (endPoint > lengthInSeconds || endPoint <= 0) {
            endPoint = lengthInSeconds;
            int currentMinutes = (int)endPoint / 60;
            int currentSeconds = ((int)endPoint) % 60;
			setEnd.setText(juce::String(currentMinutes) + ":" +
                juce::String(currentSeconds), juce::dontSendNotification);
        }
        if (startPoint > endPoint) {
            double x = startPoint;
            startPoint = endPoint;
            endPoint = x;
			setStart.setText(juce::String(startPoint), juce::dontSendNotification);
			setEnd.setText(juce::String(endPoint), juce::dontSendNotification);
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
    else if (button == &markerButton) {
        marks.push_back(transportSource.getCurrentPosition());
        repaint();
    }
    else if (button == &getmarkerButton) {
        order = setMarker.getText().getIntValue();
        if (!setMarker.isEmpty()) {
            if (order > 0 && order <= marks.size()) {
                transportSource.setPosition(marks[order - 1]);
                transportSource.start();
            }
            else {
                setMarker.clear();
            }
        }

    }
    else if (button == &forwardButton) {
        double current = transportSource.getCurrentPosition();
        double total = transportSource.getLengthInSeconds();
        transportSource.setPosition(std::min(current + 10.0, total));
    }
    else if (button == &backwardButton) {
        double current = transportSource.getCurrentPosition();
        transportSource.setPosition(std::max(current - 10.0, 0.0));
    }
    else if (button == &favoriteButton)
    {
        if (currentTrackIndex >= 0 && currentTrackIndex < playlistFiles.size())
        {
            auto file = playlistFiles[currentTrackIndex];

            // Define the favorites folder (inside user’s Documents, for example)
            juce::File favoritesFolder = juce::File::getSpecialLocation(
                juce::File::userDocumentsDirectory).getChildFile("Favorites");

            if (!favoritesFolder.exists())
                favoritesFolder.createDirectory();  // make sure folder exists

            // Copy the file into Favorites
            juce::File destFile = favoritesFolder.getChildFile(file.getFileName());

            auto result = file.copyFileTo(destFile);

            if (result)
            {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::InfoIcon,
                    "Favorite Added",
                    "The song has been added to Favorites successfully!");
            }
            else
            {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::WarningIcon,
                    "Error",
                    "Failed to add the song to Favorites.");
            }

        }
    }
    else if (button == &themeButton) {
        theme = !theme;
        if (theme) {
            themeButton.setButtonText("light");
            // value text color
            volumeSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
            timeSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
            speedSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
        }
        else {
            themeButton.setButtonText("Dark");
            // value text color
            volumeSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
            timeSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
            speedSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        }
        repaint();
    }
    else if (button == &pinnButton)
    {
        pinned = false;
        if (currentTrackIndex >= 0 && currentTrackIndex < playlistFiles.size())
        {
            juce::File currentFile = playlistFiles[currentTrackIndex];
            double pos = transportSource.getCurrentPosition();

            // write session file
            juce::File sessionFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                .getChildFile("last_session.txt");
            juce::String content = currentFile.getFullPathName() + "\n" + juce::String(pos);
            sessionFile.replaceWithText(content);

            pinnButton.setButtonText("Pinned");

        }
    }
    else if (button == &clearButton)
    {
        // Stop playback
        transportSource.stop();
        transportSource.setSource(nullptr);
        readerSource.reset();

        // Clear playlist and UI
        playlistFiles.clear();
        playlistBox.clear();
        metadataLable.setText("", juce::dontSendNotification);
        thumbnail.clear();
        timeSlider.setValue(0.0);
        volumeSlider.setValue(0.5, juce::dontSendNotification);
        transportSource.setGain(0.5f);
        speedSlider.setValue(1.0, juce::dontSendNotification);
        resampleSource.setResamplingRatio(1.0f);

        // Reset state
        currentTrackIndex = -1;
        cleared = true;
        clearButton.setButtonText("Cleared");

        juce::File sessionFile = juce::File::getSpecialLocation(
            juce::File::userApplicationDataDirectory).getChildFile("last_session.txt");
        sessionFile.replaceWithText("CLEARED");
    }else if (button == &shuffleButton)
    {
        isShuffling =  !isShuffling;
        shuffleButton.setButtonText(isShuffling ? "Shuffled" : "Shuffle");
        if (isShuffling && playlistFiles.size() > 1)
            buildShuffle();// fresh shuffle when turning on
            startShuffle(currentTrackIndex);
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
    cleared = false;
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
    if (isShuffling && playlistFiles.size() > 1) {
        startShuffle(currentTrackIndex);
        buildShuffle();
    }

}
void Player::timerCallback()
{
    // update the time slider to reflect current playback position
    if (readerSource != nullptr) {
        double pos = transportSource.getCurrentPosition();
        double lengthInSeconds = transportSource.getLengthInSeconds();
        // loop
        if (is_restartLoop && pos >= lengthInSeconds - 0.001) { // to avoid cut the last second
            transportSource.setPosition(0.0);
            transportSource.start();
            pos = transportSource.getCurrentPosition();
        }
        // start end loop
        if (isLooping && repeatedTimes && (pos >= endPoint - 0.001 || pos <= startPoint)) { // to avoid cut the last second

            transportSource.setPosition(startPoint);
            transportSource.start();
            pos = transportSource.getCurrentPosition();
            repeatedTimes--;
        }
        double eps = 0.05; // tolerance for float comparison

        // auto-advance only if we are not manually at the end
        if (!is_restartLoop && !isLooping && lengthInSeconds > 0.0 && pos >= lengthInSeconds - eps)
        {
            auto now = juce::Time::getMillisecondCounter();
            if (!hasTriggeredNext && now - lastManualJumpTime > 800) // wait 800 ms after manual jump
            {
                int nextIndex = -1;

                if (isShuffling && playlistFiles.size() > 1)
                    nextIndex = nextShuffledIndex();
                else if (currentTrackIndex + 1 < playlistFiles.size())
                    nextIndex = currentTrackIndex + 1;

                if (nextIndex >= 0)
                    selectTrack(nextIndex);

                hasTriggeredNext = true;
            }
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

    playlistBox.setSelectedId(index + 1, juce::dontSendNotification);
}
void Player::saveLast()
{
    if (!cleared && pinned) {
        if (!readerSource) return;

        juce::File sessionFile = juce::File::getSpecialLocation(
            juce::File::userApplicationDataDirectory).getChildFile("last_session.txt");

        juce::String filePath = playlistFiles.isEmpty() ? "" : playlistFiles[currentTrackIndex].getFullPathName();
        double position = transportSource.getCurrentPosition();

        juce::String content = filePath + "\n" + juce::String(position);
        sessionFile.replaceWithText(content);
    }
}
void Player::loadLast()
{
    juce::File sessionFile = juce::File::getSpecialLocation(
        juce::File::userApplicationDataDirectory).getChildFile("last_session.txt");

    if (!sessionFile.existsAsFile()) return;

    juce::String content = sessionFile.loadFileAsString().trim();

    if (content == "CLEARED") {
        return;
    }
    if (!cleared && pinned) {
        juce::StringArray lines = juce::StringArray::fromLines(content);
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
}


