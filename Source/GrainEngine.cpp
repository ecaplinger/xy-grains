#include "GrainEngine.h"

GrainEngine::GrainEngine()
{
    formatManager.registerBasicFormats();
    syncGrains(32);
}

GrainEngine::~GrainEngine()
{
    stopAll();
    releaseResources();
}

void GrainEngine::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    mixer.prepareToPlay (samplesPerBlockExpected, sampleRate);
}

void GrainEngine::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    mixer.getNextAudioBlock (bufferToFill);
}

void GrainEngine::releaseResources ()
{
    mixer.releaseResources();
}

bool GrainEngine::loadFile (const juce::File& file)
{
    stopAll();
    reader.reset (formatManager.createReaderFor (file));
    if (reader == nullptr)
        return false;

    for (auto&& grain : grains)
        grain->setFileSource (reader.get());

    sampleLength = (double) reader->lengthInSamples / reader->sampleRate;
    loadedFile = file;
    return true;
}

juce::File GrainEngine::getLoadedFile() const
{
    return loadedFile;
}

void GrainEngine::syncGrains (int numCircles)
{
    while ((int) grains.size() < numCircles)
    {
        auto grain = std::make_unique<MotionGrain> (&globalParams);
        mixer.addInputSource (grain.get(), false);
        if (reader != nullptr)
            grain->setFileSource (reader.get());
        grains.push_back (std::move (grain));
    }

    while ((int) grains.size() > numCircles)
    {
        mixer.removeInputSource (grains.back().get());
        grains.pop_back();
    }
}

void GrainEngine::trigger (int circleIndex, MotionGrain::GrainParameters params)
{
    params.position *= sampleLength;
    grains[grainIndex++]->spawn(params);
    grainIndex = grainIndex % grains.size();
}

void GrainEngine::stopAll ()
{
    for (auto&& grain : grains)
        grain->stop();
}
