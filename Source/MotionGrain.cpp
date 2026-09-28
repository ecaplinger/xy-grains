#include "MotionGrain.h"

MotionGrain::MotionGrain (GrainParameters* globalParams) :
    resamplingSource_ (&transportSource_, false)
{
    transportSource_.addChangeListener (this);
    params_ = globalParams;
}

MotionGrain::~MotionGrain()
{
    // The note-off timer may still be armed; stop it before the juce::Timer
    // base class is destroyed (a firing timer would call into a
    // half-destroyed object).
    stopTimer();
    stop();
    // Detach the transport from the reader source before the members are
    // destroyed: readerSource_ dies first (reverse declaration order), and
    // ~AudioTransportSource would then call releaseResources() on the
    // destroyed reader in its setSource(nullptr).
    transportSource_.setSource (nullptr);
}

void MotionGrain::spawn (double position)
{
    transportSource_.setPosition (position);

    resamplingSource_.setResamplingRatio (params_->speed);

    env_.setParameters (juce::ADSR::Parameters (
        params_->attack,
        params_->decay,
        params_->sustain,
        params_->release
    ));

    env_.noteOn();
    transportSource_.start();

    startTimer (params_->noteLength); // start timer to shut off note
}

void MotionGrain::spawn (MotionGrain::GrainParameters params)
{
    transportSource_.setPosition (params.position);
    transportSource_.setGain (params.gain);
    resamplingSource_.setResamplingRatio (params.speed);
    env_.setParameters (juce::ADSR::Parameters (
                params.attack,
                params.decay,
                params.sustain,
                params.release
    ));

    env_.noteOn();
    transportSource_.start();
    startTimer (params.noteLength);
}

void MotionGrain::stop()
{
    transportSource_.stop();
}

void MotionGrain::setFileSource (juce::AudioFormatReader* incomingReader)
{
    stop();
    transportSource_.setSource(nullptr);
    readerSource_.reset (new juce::AudioFormatReaderSource (incomingReader, false));
    transportSource_.setSource (readerSource_.get());
}

void MotionGrain::timerCallback()
{
    env_.noteOff();
    stopTimer();
}

void MotionGrain::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    resamplingSource_.prepareToPlay (samplesPerBlockExpected, sampleRate);
    env_.setSampleRate (sampleRate);
}

void MotionGrain::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    resamplingSource_.getNextAudioBlock (bufferToFill);
    env_.applyEnvelopeToBuffer (*bufferToFill.buffer, bufferToFill.startSample, bufferToFill.buffer->getNumSamples());
}

void MotionGrain::releaseResources ()
{
    transportSource_.releaseResources();
}

void MotionGrain::changeListenerCallback (juce::ChangeBroadcaster* source)
{
    if (source == &transportSource_)
    {
    }
}

void MotionGrain::clearSource()
{
    transportSource_.setSource (nullptr);
    readerSource_.reset();
}
