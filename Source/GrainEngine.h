#pragma once

#include <JuceHeader.h>
#include "MotionGrain.h"

//==============================================================================
/** Owns one grain per circle: loads the sample file, mixes all grains, and
    triggers grains by circle index.
    All parameters are global for now (see MotionGrain::GrainParameters). */
class GrainEngine : public juce::AudioSource
{
public:
    GrainEngine();
    ~GrainEngine();

    // from AudioSource
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources () override;

    /** Loads @file for all grains. Returns false if the file can't be read. */
    bool loadFile (const juce::File& file);

    /** Makes sure there is exactly one grain per circle. */
    void syncGrains (int numCircles);

    /** Triggers the grain for circle @circleIndex at @position, a 0..1
        fraction of the file's total playtime. */
    void trigger (int circleIndex, MotionGrain::GrainParameters params);

    void stopAll();

    juce::MixerAudioSource mixer;

    MotionGrain::GrainParameters globalParams;

    /** The file currently loaded, or an empty File if none. */
    juce::File getLoadedFile() const;

private:
    std::vector<std::unique_ptr<MotionGrain>> grains;
    std::unique_ptr<juce::AudioFormatReader> reader;
    juce::AudioFormatManager formatManager;
    double sampleLength = 0.0;    // seconds
    juce::File loadedFile;

    int grainIndex = 0;

    const static int NUM_GRAINS = 32;

    JUCE_DECLARE_NON_COPYABLE (GrainEngine)
};
