#include "CirclePad.h"

#include <cmath>

CirclePad::CirclePad()
{
    circles.push_back (Circle { { 0.5f, 0.5f }, false });
    selectedIndex = 0;
    // engine.syncGrains ((int) circles.size());
    setWantsKeyboardFocus (true);
}

CirclePad::~CirclePad()
{
    stopTimer();
}

void CirclePad::paint (juce::Graphics& g)
{
    juce::Colour ccirc;

    switch (activeParam)
    {
        case POSITION:
            ccirc = juce::Colours::dodgerblue;
            break;

        case ATTACK:
            ccirc = juce::Colours::goldenrod;
            break;

        case DECAY:
            ccirc = juce::Colours::ivory;
            break;

        case SUSTAIN:
            ccirc = juce::Colours::lavender;
            break;

        case RELEASE:
            ccirc = juce::Colours::lawngreen;
            break;

        case NOTE_LENGTH:
            ccirc = juce::Colours::khaki;
            break;

        case SPEED:
            ccirc = juce::Colours::indianred;
            break;

        case GAIN:
            ccirc = juce::Colours::aqua;
            break;
    }

    g.setColour (juce::Colours::darkgrey);
    g.fillAll();

    // Beat grid
    const int n = numBeats;
    g.setColour (juce::Colours::white.withAlpha (0.18f));
    for (int i = 1; i < n; ++i)
    {
        const int x = (i * getWidth()) / n;
        g.drawVerticalLine (x, 0.0f, (float) getHeight());
    }

    for (size_t i = 0; i < circles.size(); ++i)
    {
        const float x = circles[i].centre.x * getWidth();
        const float y = circles[i].centre.y * getHeight();
        const juce::Rectangle<float> circleRect (x - circleRadius, y - circleRadius,
                                                 circleRadius * 2.0f, circleRadius * 2.0f);

        g.setColour (ccirc);
        g.fillEllipse (circleRect);

        if (circles[i].selected)
        {
            if (i == selectedIndex)
                g.setColour(juce::Colours::orchid);
            else
                g.setColour (juce::Colours::peachpuff);
            g.drawEllipse (circleRect, 2.0f);
        }
    }

    // Playhead
    const double playheadX = (elapsedSeconds / getDurationSeconds()) * (double) getWidth();
    g.setColour (juce::Colours::red);
    g.fillRect (juce::Rectangle<float> ((float) playheadX - 1.0f, 0.0f, 2.0f, (float) getHeight()));

    // Rubber-band selection rectangle
    if (isSelecting && ! selectRect.isEmpty())
    {
        g.setColour (juce::Colours::white);
        g.drawRect (selectRect.toNearestInt (), 1);
    }
}

bool CirclePad::keyPressed (const juce::KeyPress& key)
{
    std::cerr << "key pressed" << std::endl;
    const bool isDelete = key.getKeyCode () == juce::KeyPress::deleteKey
                      || key.getKeyCode () == juce::KeyPress::backspaceKey;

    if (isDelete)
    {
        for (int i = circles.size() - 1; i >= 0; i--)
            if (circles[i].selected)
                circles.erase (circles.begin() + i);
        // engine.syncGrains ((int) circles.size());
        repaint();
        return true;
    }

    std::cerr << key.getTextCharacter() << std::endl;

    const juce::ModifierKeys mods = key.getModifiers();
    if ((mods.isCtrlDown () || mods.isCommandDown ())
         && (key.getTextCharacter () == 'a' || key.getTextCharacter () == 'A'))
    {
        for (Circle& c : circles)
            c.selected = true;
        repaint();
        return true;
    }

    switch (key.getTextCharacter())
    {
        case 'l':
        case 'L':
        {
            if (selectedIndex == -1)
                return false;

            const float selectedY = circles[selectedIndex].centre.y;
            for (Circle& c : circles)
                if (c.selected)
                    c.centre.y = selectedY;

            syncCircleParams();
            repaint();

            return true;
        }

        case 'x':
        case 'X':
        case 'y':
        case 'Y':
        {
            if (isDragging)
            {
                dragCircles(lastMousePosition);
                repaint();
            }
            return true;
        }

        case 'q':
        case 'Q':
        {
            const int n = numBeats * quantizeDenominator;
            for (Circle& c : circles)
                if (c.selected)
                    c.centre.x = (float) (std::floor (c.centre.x * (double) n) / (double) n);

            repaint();
            return true;
        }

        case 'a':
        case 'A':
        {
            changeActiveParam (ATTACK);
            return true;
        }

        case 1:
        {
            for (auto&& c : circles)
                c.selected = true;
            return true;
        }

        case 'd':
        case 'D':
        {
            changeActiveParam (DECAY);
            return true;
        }

        case 's':
        case 'S':
        {
            changeActiveParam (SUSTAIN);
            return true;
        }

        case 'r':
        case 'R':
        {
            changeActiveParam (RELEASE);
            return true;
        }

        case 'p':
        case 'P':
        {
            changeActiveParam (POSITION);
            return true;
        }

        case 'e':
        case 'E':
        {
            changeActiveParam (SPEED);
            return true;
        }

        case 'n':
        case 'N':
        {
            changeActiveParam (NOTE_LENGTH);
            return true;
        }

        case 'g':
        case 'G':
        {
            changeActiveParam (GAIN);
            return true;
        }

        case 3:
        {
            copyCircles.clear();
            for (auto&& c : circles)
                copyCircles.push_back(c);
            return true;
        }

        case 22:
        {
            if (!circles.empty())
                for (auto&& c : circles)
                    c.selected = false;
            for (auto&& c : copyCircles)
                circles.push_back(c);
            return true;
        }
    }

    return false;
}

bool CirclePad::keyStateChanged (bool isDown)
{
    // When a key is released while dragging, re-apply the drag as if the
    // mouse had just moved, so axis-locked circles snap to the current
    // mouse position. (KeyPress::isKeyCurrentlyDown is the source of truth
    // for which keys are down; we simply re-run the drag math.)
    if (! isDown)
    {
        dragCircles (lastMousePosition);
        repaint();
    }

    return false;
}

void CirclePad::modifierKeysChanged (const juce::ModifierKeys& keys)
{
    shiftPressed = keys.isShiftDown();
    ctrlPressed = keys.isCtrlDown();
    std::cerr << "yeah, shiftPressed = " << shiftPressed << ", ctrlPressed = " << ctrlPressed << std::endl;
}

void CirclePad::mouseDown (const juce::MouseEvent& event)
{
    grabKeyboardFocus();

    const int index = hitTest (event.position.toFloat());

    if (!shiftPressed)
    {
        if (!circles.empty())
            for (Circle& c : circles)
                c.selected = false;
    }

    if (index != -1)
        circles[index].selected = true;

    selectedIndex = index;
    isDragging = (index >= 0);
    lastMousePosition = event.position.toFloat();

    if (isDragging)
    {
        dragOffset = { circles[selectedIndex].centre.x * getWidth() - event.position.x,
                       circles[selectedIndex].centre.y * getHeight() - event.position.y };
        setMouseCursor (juce::MouseCursor::DraggingHandCursor);

        for (Circle& c : circles)
            if (c.selected)
                c.lockedCentre = c.centre;
    }
    else
    {
        // Start a rubber-band selection in empty space.
        isSelecting = true;
        selectStart = event.position.toFloat();
        selectRect = {};
        setMouseCursor (juce::MouseCursor::NormalCursor);
    }

    repaint();
}

void CirclePad::mouseDrag (const juce::MouseEvent& event)
{
    lastMousePosition = event.position.toFloat();

    if (isSelecting)
    {
        selectRect = juce::Rectangle<float> (selectStart, event.position.toFloat());

        for (Circle& c : circles)
            if (selectRect.contains (c.centre.x * getWidth(), c.centre.y * getHeight()))
                c.selected = true;
    }
    else
    {
        dragCircles (lastMousePosition);
    }

    repaint();
}

void CirclePad::dragCircles (const juce::Point<float>& position)
{
    if (! isDragging)
        return;

    const float x = (position.x + dragOffset.x) / getWidth();
    const float y = (position.y + dragOffset.y) / getHeight();

    const float dx = x - circles[selectedIndex].lockedCentre.x;
    const float dy = y - circles[selectedIndex].lockedCentre.y;

    // Axis-locked dragging: holding 'x' locks the y-axis, 'y' locks the x-axis.
    // Query the OS-level key state so it works even if focus is lost mid-drag.
    const bool xHeld = juce::KeyPress::isKeyCurrentlyDown ('x');
    const bool yHeld = juce::KeyPress::isKeyCurrentlyDown ('y');
    const bool lockY = xHeld && !yHeld;
    const bool lockX = yHeld && !xHeld;

    for (int i = 0; i < circles.size(); i++)
        if (circles[i].selected)
        {
            circles[i].centre = { lockX ? circles[i].lockedCentre.x : juce::jlimit(0.0f, 1.0f, circles[i].lockedCentre.x + dx),
                                  lockY ? circles[i].lockedCentre.y : juce::jlimit(0.0f, 1.0f, circles[i].lockedCentre.y + dy) };
            syncCircleParams ();
        }
}

void CirclePad::mouseUp (const juce::MouseEvent&)
{
    isDragging = false;
    isSelecting = false;
    setMouseCursor (juce::MouseCursor::NormalCursor);
    repaint();
}

void CirclePad::mouseDoubleClick (const juce::MouseEvent& event)
{
    const int index = hitTest (event.position.toFloat());

    MotionGrain::GrainParameters lastParams;
    if (!circles.empty())
    {
        if (selectedIndex == -1)
            lastParams = circles.back().params;
        else
            lastParams = circles[selectedIndex].params;
    }

    // clear selection if shift is not pressed
    if (!shiftPressed)
        if (!circles.empty())
            for (Circle& c : circles)
                c.selected = false;

    // Double-clicking on an existing circle just (re)selects it.
    if (index >= 0)
    {
        selectedIndex = index;
        repaint();
        return;
    }

    // Double-clicking on empty space adds a new circle there.
    float x = event.position.x;
    float y = event.position.y;

    if (hitTest (juce::Point<float> (x, y)) >= 0)
        x += circleRadius * 2.0f;  // nudge if it lands on an existing circle

    circles.push_back (Circle { { x / (float) getWidth(), y / (float) getHeight() }, true, lastParams });
    syncCircleParams();
    selectedIndex = (int) circles.size () - 1;
    // engine.syncGrains ((int) circles.size());
    repaint();
}

int CirclePad::hitTest (juce::Point<float> point) const
{
    for (int i = (int) circles.size () - 1; i >= 0; --i)
    {
        const float dx = circles[i].centre.x * getWidth() - point.x;
        const float dy = circles[i].centre.y * getHeight() - point.y;
        if (dx * dx + dy * dy <= circleRadius * circleRadius)
            return i;
    }

    return -1;
}

void CirclePad::changeActiveParam (CirclePad::ActiveParam param)
{
    activeParam = param;

    for (auto&& c : circles)
    {
        switch (activeParam)
        {
            case POSITION:
                c.centre.y = denormalize(c.params.position, minParams.position, maxParams.position);
                break;

            case ATTACK:
                c.centre.y = denormalize(c.params.attack, minParams.attack, maxParams.attack);
                break;

            case SUSTAIN:
                c.centre.y = denormalize(c.params.sustain, minParams.sustain, maxParams.sustain);
                break;

            case DECAY:
                c.centre.y = denormalize(c.params.decay, minParams.decay, maxParams.decay);
                break;

            case RELEASE:
                c.centre.y = denormalize(c.params.release, minParams.release, maxParams.release);
                break;

            case NOTE_LENGTH:
                c.centre.y = denormalize(c.params.noteLength, minParams.noteLength, maxParams.noteLength);
                break;

            case SPEED:
                c.centre.y = denormalize(c.params.speed, minParams.speed, maxParams.speed);
                break;

            case GAIN:
                c.centre.y = denormalize(c.params.gain, minParams.gain, maxParams.gain);
                break;
        }
    }

    repaint();

    if (activeParamChangedCallback)
        activeParamChangedCallback();
}

void CirclePad::syncCircleParams ()
{
    for (auto&& c : circles)
    {
        switch (activeParam)
        {
            case POSITION:
                c.params.position = normalize(c.centre.y, minParams.position, maxParams.position);
                break;

            case ATTACK:
                c.params.attack = normalize(c.centre.y, minParams.attack, maxParams.attack);
                break;

            case SUSTAIN:
                c.params.sustain = normalize(c.centre.y, minParams.sustain, maxParams.sustain);
                break;

            case DECAY:
                c.params.decay = normalize(c.centre.y, minParams.decay, maxParams.decay);
                break;

            case RELEASE:
                c.params.release = normalize(c.centre.y, minParams.release, maxParams.release);
                break;

            case NOTE_LENGTH:
                c.params.noteLength = normalize(c.centre.y, minParams.noteLength, maxParams.noteLength);
                break;

            case SPEED:
                c.params.speed = normalize(c.centre.y, minParams.speed, maxParams.speed);
                break;

            case GAIN:
                c.params.gain = (float) normalize(c.centre.y, minParams.gain, maxParams.gain);
                break;
        }
    }
}

//==============================================================================
// Transport

void CirclePad::startPlaying()
{
    if (isPlaying)
        return;

    isPlaying = true;
    lastTickTime = (double) juce::Time::getMillisecondCounter();
    startTimer (16);
}

void CirclePad::pausePlaying()
{
    if (! isPlaying)
        return;

    isPlaying = false;
    stopTimer();
    repaint();
}

void CirclePad::stopPlaying()
{
    isPlaying = false;
    stopTimer();
    engine.stopAll();
    elapsedSeconds = 0.0;
    lastPlayheadX = 0.0;
    repaint();
}

int CirclePad::getBpm() const
{
    return bpm;
}

void CirclePad::setBpm (int newBpm)
{
    bpm = juce::jlimit (1, 1000, newBpm);
    repaint();
}

int CirclePad::getBeats() const
{
    return numBeats;
}

void CirclePad::setBeats (int newBeats)
{
    newBeats = juce::jlimit (1, 64, newBeats);

    if (newBeats != numBeats)
    {
        // Rescale trigger x positions to preserve their beat index.
        const double scale = (double) numBeats / (double) newBeats;
        for (Circle& c : circles)
            c.centre.x = (float) (c.centre.x * scale);

        numBeats = newBeats;
    }

    repaint();
}

double CirclePad::getDurationSeconds() const
{
    return (numBeats * 60.0) / bpm;
}

void CirclePad::timerCallback()
{
    if (! isPlaying)
        return;

    const double now = (double) juce::Time::getMillisecondCounter();
    elapsedSeconds += (now - lastTickTime) / 1000.0;
    lastTickTime = now;

    // Loop back to the start at the end of the window.
    const double duration = getDurationSeconds();
    elapsedSeconds = std::fmod (elapsedSeconds, duration);

    const double prevX = lastPlayheadX;
    const double newX = elapsedSeconds / duration;
    lastPlayheadX = newX;

    for (int i = 0; i < (int) circles.size(); i++)
        if (crosses (prevX, newX, circles[i].centre.x))
            engine.trigger (i, circles[i].params);

    repaint();
}

bool CirclePad::crosses (double prevX, double newX, double circleX)
{
    if (newX >= prevX)
        return circleX > prevX && circleX <= newX;

    // Wrapped around the end of the window: (prevX, 1] or [0, newX].
    return circleX > prevX || circleX <= newX;
}

//==============================================================================
// Quantize

void CirclePad::setQuantize (int denom)
{
    quantizeDenominator = juce::jlimit (1, 16, denom);
}

int CirclePad::getQuantize() const
{
    return quantizeDenominator;
}

//==============================================================================
// Active parameter (tabs)

const std::vector<CirclePad::ParamTab>& CirclePad::getParamTabs()
{
    static const std::vector<ParamTab> tabs = {
        { POSITION,    "Position",    "p" },
        { ATTACK,      "Attack",      "a" },
        { DECAY,       "Decay",       "d" },
        { SUSTAIN,     "Sustain",     "s" },
        { RELEASE,     "Release",     "r" },
        { NOTE_LENGTH, "Note Length", "n" },
        { SPEED,       "Speed",       "e" },
        { GAIN,        "Gain",        "g" },
    };
    return tabs;
}

CirclePad::ActiveParam CirclePad::getActiveParam() const
{
    return activeParam;
}

void CirclePad::setActiveParam (ActiveParam p)
{
    if (p == activeParam)
        return;

    changeActiveParam (p);
}

void CirclePad::setActiveParamChangedCallback (std::function<void()> cb)
{
    activeParamChangedCallback = std::move (cb);
}

//==============================================================================
// Settings (JSON)

namespace
{
    juce::var paramsToJson (const MotionGrain::GrainParameters& p)
    {
        juce::var params (new juce::DynamicObject ());
        auto* paramsObj = params.getDynamicObject();
        paramsObj->setProperty ("noteLength", p.noteLength);
        paramsObj->setProperty ("attack",     p.attack);
        paramsObj->setProperty ("decay",      p.decay);
        paramsObj->setProperty ("sustain",    p.sustain);
        paramsObj->setProperty ("release",    p.release);
        paramsObj->setProperty ("position",   p.position);
        paramsObj->setProperty ("speed",      p.speed);
        paramsObj->setProperty ("pan",        p.pan);
        paramsObj->setProperty ("gain",       p.gain);
        return params;
    }

    MotionGrain::GrainParameters paramsFromJson (const juce::var& json)
    {
        MotionGrain::GrainParameters p;
        p.noteLength = static_cast<int> (json ["noteLength"]);
        p.attack     = (float) json ["attack"];
        p.decay      = (float) json ["decay"];
        p.sustain    = (float) json ["sustain"];
        p.release    = (float) json ["release"];
        p.position   =         json ["position"];
        p.speed      =         json ["speed"];
        p.pan        = (float) json ["pan"];
        p.gain       = (float) json ["gain"];
        return p;
    }
}

juce::var CirclePad::toJson() const
{
    juce::Array<juce::var> circlesArray;

    for (const auto& c : circles)
    {
        juce::var circle (new juce::DynamicObject ());
        auto* circleObj = circle.getDynamicObject();
        circleObj->setProperty ("x",      c.centre.x);
        circleObj->setProperty ("y",      c.centre.y);
        circleObj->setProperty ("params", paramsToJson (c.params));
        circlesArray.add (circle);
    }

    juce::var root (new juce::DynamicObject ());
    auto* rootObj = root.getDynamicObject();
    rootObj->setProperty ("format",      1);
    rootObj->setProperty ("bpm",         bpm);
    rootObj->setProperty ("beats",       numBeats);
    rootObj->setProperty ("quantize",    quantizeDenominator);
    rootObj->setProperty ("activeParam", (int) activeParam);
    rootObj->setProperty ("loadedFile",  engine.getLoadedFile().getFullPathName());
    rootObj->setProperty ("circles",     juce::var (circlesArray));
    return root;
}

juce::String CirclePad::fromJson (const juce::var& json)
{
    stopPlaying();

    setBpm      (static_cast<int> (json ["bpm"]));
    setBeats    (static_cast<int> (json ["beats"]));
    setQuantize (static_cast<int> (json ["quantize"]));

    circles.clear();
    copyCircles.clear();
    selectedIndex = -1;

    const juce::var circlesVar = json ["circles"];
    if (circlesVar.isArray())
        for (int i = 0; i < circlesVar.size(); ++i)
        {
            const juce::var circleVar = circlesVar[i];
            if (! circleVar.isObject())
                continue;

            Circle c;
            c.centre   = juce::Point<float> ((float) circleVar ["x"],
                                              (float) circleVar ["y"]);
            c.params   = paramsFromJson (circleVar ["params"]);
            c.selected = false;
            circles.push_back (c);
        }

    if (circles.empty())
        circles.push_back (Circle { { 0.5f, 0.5f }, false });

    const int activeParamInt = static_cast<int> (json ["activeParam"]);
    if (activeParamInt >= 0 && activeParamInt < (int) getParamTabs().size())
    {
        const ActiveParam param = static_cast<ActiveParam> (activeParamInt);
        if (param != activeParam)
            setActiveParam (param);
    }

    juce::String error;

    const juce::String filePath = json ["loadedFile"];
    if (! filePath.isEmpty())
    {
        const juce::File file (filePath);
        if (file.existsAsFile())
        {
            if (! engine.loadFile (file))
                error = "Couldn't read the audio file\n" + file.getFullPathName() + ".";
        }
        else
            error = "The saved audio file no longer exists:\n" + file.getFullPathName();
    }

    syncCircleParams();
    repaint();
    return error;
}

//==============================================================================
// File loading

void CirclePad::openFile()
{
    chooser_ = std::make_unique<juce::FileChooser> ("Choose File You Cur", juce::File(), "*.wav");

    chooser_->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                           [this] (const juce::FileChooser& fc)
                           {
                               const juce::File result = fc.getResult();
                               if (result != juce::File())
                                   engine.loadFile (result);
                           });
}
