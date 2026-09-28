#pragma once

#include <JuceHeader.h>

//==============================================================================
/** A single granular grain: resampled playback of a shared file reader,
    with an ADSR envelope. Copied from MotionGranular, except spawn() takes
    the grain's start position (in samples from the start of the file). */
class MotionGrain : public juce::AudioSource,
                    public juce::ChangeListener,
                    public juce::Timer
{
public:
    struct GrainParameters
    {
        int noteLength = 1000;

        float attack  = 0.2f;
        float decay   = 0.1f;
        float sustain = 0.5f;
        float release = 1.f;

        double position = 0.0;
        double speed    = 1.f;

        float pan  = 0.5f;
        float gain = 1.f;
    };

    MotionGrain (GrainParameters* globalParams);
    ~MotionGrain() override;

    void setFileSource (juce::AudioFormatReader* incomingReader);
    void clearSource();

    /** Triggers the grain starting @position samples into the file. */
    void spawn (double position);
    void spawn (GrainParameters params);
    void stop();

    // from AudioSource
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources () override;

    // from ChangeListener
    void changeListenerCallback (juce::ChangeBroadcaster* source) override;

    // from Timer
    void timerCallback () override;

private:
    // playback components
    juce::AudioTransportSource transportSource_;
    juce::ResamplingAudioSource resamplingSource_;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource_;
    juce::ADSR env_;

    // parameters
    GrainParameters* params_;

    JUCE_DECLARE_NON_COPYABLE (MotionGrain)
};
