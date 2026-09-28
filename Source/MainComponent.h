#pragma once

#include <JuceHeader.h>
#include "CirclePad.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    // Your private member variables go here...
    CirclePad circlePad;
    juce::MixerAudioSource mixer;
    juce::TextButton openButton, playButton, pauseButton, stopButton, saveButton, loadButton;
    juce::TextEditor bpmText, beatsText;
    juce::Label bpmLabel, beatsLabel;
    juce::ComboBox gridCombo;
    juce::Label gridLabel;

    std::vector<std::unique_ptr<juce::TextButton>> paramTabs;
    void createParamTabs();
    void updateParamTabs();

    //==============================================================================
    // Settings (JSON)
    std::unique_ptr<juce::FileChooser> chooser;
    void saveSettings();
    void loadSettings();


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
