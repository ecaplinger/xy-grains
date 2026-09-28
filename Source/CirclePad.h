#pragma once

#include <JuceHeader.h>
#include "GrainEngine.h"

//==============================================================================
/** A pad that draws circles which can be dragged around, plus a sweeping
    playhead driven by a play/pause/stop transport. */
class CirclePad    : public juce::Component,
                       public juce::Timer
{
public:
    CirclePad();
    ~CirclePad();

    void paint (juce::Graphics& g) override;
    bool keyPressed (const juce::KeyPress& key) override;
    bool keyStateChanged (bool isDown) override;
    void modifierKeysChanged (const juce::ModifierKeys& keys) override;

    void mouseDown (const juce::MouseEvent& event) override;
    void mouseDrag (const juce::MouseEvent& event) override;
    void mouseUp (const juce::MouseEvent& event) override;
    void mouseDoubleClick (const juce::MouseEvent& event) override;

    //=========================================================================
    // Transport / time signature
    void startPlaying();
    void pausePlaying();
    void stopPlaying();

    int getBpm() const;
    void setBpm (int newBpm);

    /** Number of beats in the window (1..64). Changing this rescales the
        triggers' x positions to preserve their beat index. */
    int getBeats() const;
    void setBeats (int newBeats);

    /** Window length in seconds, derived from BPM and number of beats. */
    double getDurationSeconds() const;

    //=========================================================================
    // Grain engine / file loading
    /** The audio source to feed into the app's mixer. */
    juce::AudioSource* getEngine() { return &engine; }

    /** Opens a file chooser and loads the chosen file into the grain engine. */
    void openFile();

    //=========================================================================
    // Settings (JSON)
    /** A var object describing all current settings (transport, quantize,
        active param, and every circle), suitable for @juce::JSON::toString. */
    juce::var toJson() const;

    /** Restores settings from a var object previously made by @method toJson.
        Also attempts to reload the audio file that was saved with the settings;
        returns an empty string on success, or an error message if the audio
        file couldn't be loaded. */
    juce::String fromJson (const juce::var& json);

    //=========================================================================
    // Quantize
    /** Sets the quantize fraction (1/1 .. 1/16): denom quantization slots
        per beat. Snapping sets a circle's x to
        floor(x * (beats * denom)) / (beats * denom). */
    void setQuantize (int denom);
    int getQuantize() const;

    //=========================================================================
    // Active parameter (tabs)
    enum ActiveParam
    {
        POSITION,
        ATTACK,
        DECAY,
        SUSTAIN,
        RELEASE,
        NOTE_LENGTH,
        SPEED,
        GAIN
    };

    /** A tab label: display name, hotkey, and the param it activates. */
    struct ParamTab
    {
        ActiveParam param;
        juce::String name;
        juce::String hotkey;
    };

    /** Ordered list of all parameter tabs. */
    static const std::vector<ParamTab>& getParamTabs();

    /** The currently-active parameter. */
    ActiveParam getActiveParam() const;

    /** Change the active parameter (no-op if already active). */
    void setActiveParam (ActiveParam p);

    /** Called after the active parameter changes (e.g. via hotkey). */
    void setActiveParamChangedCallback (std::function<void()> cb);

private:
    struct Circle
    {
        juce::Point<float> centre;   // normalised 0..1 coordinates
        bool selected;

        MotionGrain::GrainParameters params;
        juce::Point<float> lockedCentre;   // normalised 0..1 coordinates
    };

    void timerCallback () override;

    const float circleRadius = 20.0f;
    GrainEngine engine;
    std::unique_ptr<juce::FileChooser> chooser_;
    std::vector<Circle> circles;
    std::vector<Circle> copyCircles;
    int selectedIndex = -1;
    bool isDragging = false;
    bool shiftPressed = false;
    bool ctrlPressed = false;
    bool isSelecting = false;
    juce::Point<float> selectStart;
    juce::Rectangle<float> selectRect;
    juce::Point<float> dragOffset;
    juce::Point<float> lastMousePosition;
    ActiveParam activeParam = POSITION;
    std::function<void()> activeParamChangedCallback;

    // Quantize
    int quantizeDenominator = 4;

    // Transport state
    int bpm = 120;
    int numBeats = 4;
    double elapsedSeconds = 0.0;     // 0..duration
    double lastTickTime = 0.0;       // system time at last timer tick
    bool isPlaying = false;
    double lastPlayheadX = 0.0;      // normalised 0..1

    /** Moves the selected circles as if the mouse is at the given position. */
    void dragCircles (const juce::Point<float>& position);

    /** Returns the index of the topmost circle under the given point, or -1. */
    int hitTest (juce::Point<float> point) const;

    void changeActiveParam (ActiveParam param);
    void syncCircleParams ();

    /** Constant GrainParameters for minimum and maximum values (for normalization) **/
    const MotionGrain::GrainParameters minParams {
        0,              // note length
        0.f,            // attack
        0.f,            // decay
        0.f,            // sustain
        0.f,            // release
        0.0,            // position
        0.625,          // speed
        0.f,            // pan
        0.f             // gain
    };

    const MotionGrain::GrainParameters maxParams {
        10000,          // note length
        5.f,            // attack
        5.f,            // decay
        1.f,            // sustain
        10.f,           // release
        1.0,            // position
        16.0,           // speed
        1.f,            // pan
        1.f             // gain
    };
    /** True if a playhead sweep from prevX to newX passes through circleX
        (handles the wrap-around at the end of the window). */
    static bool crosses (double prevX, double newX, double circleX);

    inline double normalize(double value, double min, double max) const
    {
        return value * (max - min) + min;
    }

    inline double denormalize(double value, double min, double max) const
    {
        return (value - min) / (max - min);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CirclePad)
};
