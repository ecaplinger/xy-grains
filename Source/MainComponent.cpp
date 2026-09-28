#include "MainComponent.h"

#include <cmath>

//==============================================================================
MainComponent::MainComponent()
{
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (800, 600);

    addAndMakeVisible (circlePad);

    mixer.addInputSource (circlePad.getEngine(), false);

    addAndMakeVisible (openButton);
    openButton.setButtonText ("Open...");
    openButton.onClick = [this] { circlePad.openFile(); };

    addAndMakeVisible (playButton);
    playButton.setButtonText ("Play");
    playButton.onClick = [this] { circlePad.startPlaying(); };

    addAndMakeVisible (pauseButton);
    pauseButton.setButtonText ("Pause");
    pauseButton.onClick = [this] { circlePad.pausePlaying(); };

    addAndMakeVisible (stopButton);
    stopButton.setButtonText ("Stop");
    stopButton.onClick = [this] { circlePad.stopPlaying(); };

    addAndMakeVisible (saveButton);
    saveButton.setButtonText ("Save...");
    saveButton.onClick = [this] { saveSettings(); };

    addAndMakeVisible (loadButton);
    loadButton.setButtonText ("Load...");
    loadButton.onClick = [this] { loadSettings(); };

    addAndMakeVisible (bpmLabel);
    bpmLabel.setText ("BPM", juce::dontSendNotification);

    addAndMakeVisible (beatsLabel);
    beatsLabel.setText ("Beats", juce::dontSendNotification);

    auto applyBpm = [this] { circlePad.setBpm (juce::String (bpmText.getText ()).getIntValue ());
                             bpmText.setText (juce::String (circlePad.getBpm ())); };
    addAndMakeVisible (bpmText);
    bpmText.setText (juce::String (circlePad.getBpm ()));
    bpmText.onFocusLost = [this, applyBpm] { applyBpm(); };
    bpmText.onReturnKey = [this, applyBpm] { applyBpm(); };

    auto applyBeats = [this] { circlePad.setBeats (juce::String (beatsText.getText ()).getIntValue ());
                               beatsText.setText (juce::String (circlePad.getBeats ())); };
    addAndMakeVisible (beatsText);
    beatsText.setText (juce::String (circlePad.getBeats ()));
    beatsText.onFocusLost = [this, applyBeats] { applyBeats(); };
    beatsText.onReturnKey = [this, applyBeats] { applyBeats(); };

    addAndMakeVisible (gridLabel);
    gridLabel.setText ("Quantize", juce::dontSendNotification);

    for (int d = 1; d <= 16; ++d)
        gridCombo.addItem (juce::String ("1/") + juce::String (d), d);

    gridCombo.setSelectedId (circlePad.getQuantize ());
    gridCombo.onChange = [this] { circlePad.setQuantize (gridCombo.getSelectedId ()); };
    addAndMakeVisible (gridCombo);

    createParamTabs();
    circlePad.setActiveParamChangedCallback ([this] { updateParamTabs(); });
    updateParamTabs();

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (2, 2);
    }
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::createParamTabs()
{
    for (const auto& tab : CirclePad::getParamTabs())
    {
        auto button = std::make_unique<juce::TextButton> (tab.name);
        button->setButtonText (tab.name + " [" + juce::String (tab.hotkey) + "]");
        button->onClick = [this, param = tab.param] { circlePad.setActiveParam (param); };
        paramTabs.push_back (std::move (button));
        addAndMakeVisible (*paramTabs.back());
    }
}

void MainComponent::updateParamTabs()
{
    const auto tabs = CirclePad::getParamTabs();
    const auto active = circlePad.getActiveParam();

    for (size_t i = 0; i < paramTabs.size() && i < tabs.size(); ++i)
    {
        auto* button = paramTabs[i].get();
        button->setColour (juce::TextButton::textColourOffId, juce::Colours::white);
        button->setColour (juce::TextButton::textColourOnId, juce::Colours::white);

        if (tabs[i].param == active)
            button->setColour (juce::TextButton::buttonColourId, juce::Colours::darkorange);
        else
            button->setColour (juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    }
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    mixer.prepareToPlay (samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    mixer.getNextAudioBlock (bufferToFill);
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    const int controlsHeight = 110;
    const int tabColumnWidth = 170;
    const int padHeight = getHeight() - controlsHeight;

    circlePad.setBounds (0, 0, getWidth() - tabColumnWidth, padHeight);

    const int tabWidth = tabColumnWidth - 20;
    const int tabHeight = 40;
    const int tabX = getWidth() - tabColumnWidth + 10;
    int tabY = 10;

    for (auto& button : paramTabs)
    {
        button->setBounds (tabX, tabY, tabWidth, tabHeight);
        tabY += tabHeight + 6;
    }

    const int buttonWidth = 80, buttonHeight = 40;
    int x = 10;
    const int row1Y = getHeight() - controlsHeight + 10;
    const int row2Y = getHeight() - controlsHeight + 55;

    openButton.setBounds (x, row1Y, buttonWidth, buttonHeight);   x += buttonWidth + 10;
    playButton.setBounds (x, row1Y, buttonWidth, buttonHeight);   x += buttonWidth + 10;
    pauseButton.setBounds (x, row1Y, buttonWidth, buttonHeight);  x += buttonWidth + 10;
    stopButton.setBounds (x, row1Y, buttonWidth, buttonHeight);   x += buttonWidth + 10;
    saveButton.setBounds (x, row1Y, buttonWidth, buttonHeight);   x += buttonWidth + 10;
    loadButton.setBounds (x, row1Y, buttonWidth, buttonHeight);

    const int fieldW = 60;
    x = 10;
    bpmLabel.setBounds (x, row2Y, 40, 25);          x += 40;
    bpmText.setBounds (x, row2Y, fieldW, 25);       x += fieldW + 20;

    beatsLabel.setBounds (x, row2Y, 50, 25);        x += 50;
    beatsText.setBounds (x, row2Y, fieldW, 25);     x += fieldW + 20;

    gridLabel.setBounds (x, row2Y, 60, 25);         x += 60;
    gridCombo.setBounds (x, row2Y, 70, 25);
}

//==============================================================================
// Settings (JSON)

void MainComponent::saveSettings()
{
    chooser = std::make_unique<juce::FileChooser> (
        "Save Settings",
        juce::File::getSpecialLocation (juce::File::userHomeDirectory),
        "*.json");

    chooser->launchAsync (
        juce::FileBrowserComponent::saveMode,
        [this] (const juce::FileChooser& fc)
        {
            const juce::File file = fc.getResult();
            if (file == juce::File())
                return;

            const juce::String json = juce::JSON::toString (circlePad.toJson());

            if (file.replaceWithText (json))
                juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::InfoIcon,
                                                        "Settings saved",
                                                        "Settings written to\n" + file.getFullPathName());
            else
                juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::WarningIcon,
                                                        "Failed to save settings",
                                                        "Couldn't write\n" + file.getFullPathName());
        });
}

void MainComponent::loadSettings()
{
    chooser = std::make_unique<juce::FileChooser> (
        "Load Settings", juce::File(), "*.json");

    chooser->launchAsync (
        juce::FileBrowserComponent::openMode
            | juce::FileBrowserComponent::canSelectFiles,
        [this] (const juce::FileChooser& fc)
        {
            const juce::File file = fc.getResult();
            if (file == juce::File())
                return;

            const juce::var json = juce::JSON::parse (file);
            if (! json.isObject())
            {
                juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::WarningIcon,
                                                        "Failed to load settings",
                                                        file.getFileName()
                                                        + " doesn't contain a valid settings JSON.");
                return;
            }

            const juce::String error = circlePad.fromJson (json);
            if (! error.isEmpty())
                juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::WarningIcon,
                                                        "Settings loaded with errors",
                                                        error);
        });
}
