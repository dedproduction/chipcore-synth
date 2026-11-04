#include "PluginEditor.h"

namespace
{
constexpr int kPanelW = 250, kGap = 8, kCellW = 58, kCellH = 66, kRowStep = 70, kPad = 9;
const char* const kCategoryOrder[] = { "LEAD", "BASS", "ARP", "CHORD", "PLUCK", "PAD", "DRUM", "FX" };
const char* const kCategoryTitle[] = { "LEADS", "BASS", "ARPS", "CHORDS & STABS", "PLUCKS & KEYS", "PADS", "DRUMS", "FX" };
}

// ---------------------------------------------------------------------------
ChipcoreEditor::ChipcoreEditor (ChipcoreProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&lnf);
    addAndMakeVisible (canvas);
    canvas.setBounds (0, 0, designW, designH);

    using namespace pix;
    const int r2 = 72, r3 = 276, r4 = 480;
    auto px = [] (int i) { return kGap + i * (kPanelW + kGap); };

    // ---- row 2: sound sources ------------------------------------------------
    for (int o = 0; o < 2; ++o)
    {
        const bool a = (o == 0);
        const auto col = a ? pink : cyan;
        const juce::String pre = a ? "oscA_" : "oscB_";
        auto& pn = addPanel ({ px (o), r2, kPanelW, 196 }, a ? "OSC A" : "OSC B", col);
        combo ((pre + "wave").toRawUTF8(),  col, comboRow (pn, 0, 140));
        combo ((pre + "ratio").toRawUTF8(), col, comboRow (pn, 146, 86));
        knob ((pre + "level").toRawUTF8(),  "LEVEL", col, cell (pn, 0, 0));
        knob ((pre + "oct").toRawUTF8(),    "OCT",   col, cell (pn, 1, 0));
        knob ((pre + "semi").toRawUTF8(),   "SEMI",  col, cell (pn, 2, 0));
        knob ((pre + "fine").toRawUTF8(),   "FINE",  col, cell (pn, 3, 0));
        knob ((pre + "shape").toRawUTF8(),  "SHAPE", col, cell (pn, 0, 1));
        knob ((pre + "uni").toRawUTF8(),    "UNISON",col, cell (pn, 1, 1));
        knob ((pre + "detune").toRawUTF8(), "DETUNE",col, cell (pn, 2, 1));
    }
    {
        auto& pn = addPanel ({ px (2), r2, kPanelW, 196 }, "SUB + NOISE", yellow);
        combo ("sub_wave", yellow, comboRow (pn, 0, 110));
        combo ("sub_oct",  yellow, comboRow (pn, 116, 116));
        knob ("sub_level",   "SUB",   yellow, cell (pn, 0, 0));
        knob ("noise_level", "NOISE", yellow, cell (pn, 1, 0));
        knob ("noise_tone",  "TONE",  yellow, cell (pn, 2, 0));
        knob ("noise_decay", "DECAY", yellow, cell (pn, 3, 0));
        toggle ("noise_short", "METAL NOISE", yellow, cell (pn, 0, 1).withWidth (120).withSizeKeepingCentre (120, 20).withX (cell (pn, 0, 1).getX() + 4));
    }
    {
        auto& pn = addPanel ({ px (3), r2, kPanelW, 196 }, "PITCH + VOICE", green);
        combo ("voice_mode", green, comboRow (pn, 0, 110));
        knob ("penv_amt",   "P.ENV",  green, cell (pn, 0, 0));
        knob ("penv_time",  "P.TIME", green, cell (pn, 1, 0));
        knob ("glide",      "GLIDE",  green, cell (pn, 2, 0));
        knob ("bend_range", "BEND",   green, cell (pn, 3, 0));
        knob ("vib_depth",  "VIB",   green, cell (pn, 0, 1));
        knob ("vib_rate",   "V.RATE", green, cell (pn, 1, 1));
        knob ("vib_delay",  "V.DLY", green, cell (pn, 2, 1));
        knob ("vel_sens",   "VELO",   green, cell (pn, 3, 1));
    }

    // ---- row 3: shaping & modulation -------------------------------------------
    {
        auto& pn = addPanel ({ px (0), r3, kPanelW, 196 }, "FILTER", orange);
        combo ("flt_type", orange, comboRow (pn, 0, 110));
        knob ("flt_cutoff", "CUTOFF", orange, cell (pn, 0, 0));
        knob ("flt_reso",   "RESO",   orange, cell (pn, 1, 0));
        knob ("flt_env",    "ENV",    orange, cell (pn, 2, 0));
        knob ("flt_key",    "KEY",    orange, cell (pn, 3, 0));
        knob ("fenv_a",     "F.ATK",  orange, cell (pn, 0, 1));
        knob ("fenv_d",     "F.DEC",  orange, cell (pn, 1, 1));
        knob ("fenv_s",     "F.SUS",  orange, cell (pn, 2, 1));
        knob ("fenv_r",     "F.REL",  orange, cell (pn, 3, 1));
    }
    {
        auto& pn = addPanel ({ px (1), r3, kPanelW, 196 }, "AMP", pink);
        toggle ("amp_quant", "4-BIT VOLUME", pink, comboRow (pn, 0, 160));
        knob ("amp_a", "ATK",  pink, cell (pn, 0, 0));
        knob ("amp_d", "DEC",  pink, cell (pn, 1, 0));
        knob ("amp_s", "SUS",  pink, cell (pn, 2, 0));
        knob ("amp_r", "REL",  pink, cell (pn, 3, 0));
        const auto c0 = cell (pn, 0, 1);
        scopeArea = { c0.getX(), c0.getY() + 2, 4 * kCellW - 4, kCellH - 4 };
    }
    {
        auto& pn = addPanel ({ px (2), r3, kPanelW, 196 }, "LFO", purple);
        combo ("lfo_wave",   purple, comboRow (pn, 0, 110));
        combo ("lfo_target", purple, comboRow (pn, 116, 116));
        knob ("lfo_rate", "RATE",   purple, cell (pn, 0, 0));
        knob ("lfo_amt",  "AMOUNT", purple, cell (pn, 1, 0));
        const auto c2 = cell (pn, 2, 0);
        lfoArea = { c2.getX() + 4, c2.getY() + 6, 2 * kCellW - 8, kCellH - 18 };
    }
    {
        auto& pn = addPanel ({ px (3), r3, kPanelW, 196 }, "ARP + CHORD", cyan);
        toggle ("arp_on", "ARP", cyan, comboRow (pn, 0, 54));
        combo ("arp_mode", cyan, comboRow (pn, 58, 86));
        combo ("arp_rate", cyan, comboRow (pn, 148, 84));
        knob ("arp_oct",  "OCT",  cyan, cell (pn, 0, 0));
        knob ("arp_gate", "GATE",    cyan, cell (pn, 1, 0));
        const auto c2 = cell (pn, 2, 0);
        combo ("chord", cyan, { c2.getX() + 2, c2.getY() + 22, 2 * kCellW - 4, 20 });
    }

    // ---- row 4: effects ---------------------------------------------------------
    {
        auto& pn = addPanel ({ 8, r4, 192, 132 }, "DRIVE", red);
        combo ("dist_type", red, comboRow (pn, 0, 100));
        knob ("dist_drive", "DRIVE", red, cell (pn, 0, 0));
        knob ("dist_tone",  "TONE",  red, cell (pn, 1, 0));
        knob ("dist_mix",   "MIX",   red, cell (pn, 2, 0));
    }
    {
        auto& pn = addPanel ({ 208, r4, 134, 132 }, "CRUSH", orange);
        knob ("crush_bits", "BITS", orange, cell (pn, 0, 0));
        knob ("crush_rate", "RATE", orange, cell (pn, 1, 0));
    }
    {
        auto& pn = addPanel ({ 350, r4, 134, 132 }, "CHORUS", cyan);
        knob ("chorus_mix",  "MIX",  cyan, cell (pn, 0, 0));
        knob ("chorus_rate", "RATE", cyan, cell (pn, 1, 0));
    }
    {
        auto& pn = addPanel ({ 492, r4, 134, 132 }, "DELAY", green);
        combo ("dly_time", green, comboRow (pn, 0, 64));
        toggle ("dly_pp", "P.P", green, comboRow (pn, 68, 48));
        knob ("dly_mix", "MIX",  green, cell (pn, 0, 0));
        knob ("dly_fb",  "FDBK", green, cell (pn, 1, 0));
    }
    {
        auto& pn = addPanel ({ 634, r4, 134, 132 }, "REVERB", purple);
        knob ("rev_mix",  "MIX",  purple, cell (pn, 0, 0));
        knob ("rev_size", "SIZE", purple, cell (pn, 1, 0));
    }
    {
        auto& pn = addPanel ({ 776, r4, 256, 132 }, "MASTER", yellow);
        knob ("master_gain", "VOLUME", yellow, cell (pn, 0, 0));
        const auto c1 = cell (pn, 1, 0);
        voicesArea = { c1.getX() + 6, c1.getY() + 4, 3 * kCellW - 12, kCellH - 8 };
    }

    // ---- header: preset browser --------------------------------------------------
    for (auto* b : { &prevBtn, &nextBtn, &initBtn, &saveBtn, &loadBtn })
    {
        b->setColour (juce::TextButton::buttonOnColourId, pink);
        canvas.addAndMakeVisible (b);
    }
    prevBtn.setBounds (360, 28, 28, 24);
    presetBox.setBounds (392, 28, 330, 24);
    nextBtn.setBounds (726, 28, 28, 24);
    initBtn.setBounds (790, 28, 70, 24);
    saveBtn.setBounds (866, 28, 70, 24);
    loadBtn.setBounds (942, 28, 82, 24);

    presetBox.setColour (juce::ComboBox::outlineColourId, pink);
    presetBox.setTextWhenNothingSelected ("INIT");
    canvas.addAndMakeVisible (presetBox);
    buildPresetMenu();
    presetBox.onChange = [this]
    {
        const int id = presetBox.getSelectedId();
        if (id > 0 && id - 1 != proc.getCurrentPresetIndex()) proc.loadPreset (id - 1);
    };
    prevBtn.onClick = [this] { stepPreset (-1); };
    nextBtn.onClick = [this] { stepPreset (+1); };
    initBtn.onClick = [this] { proc.initPatch(); refreshPresetDisplay(); };
    saveBtn.onClick = [this] { savePreset(); };
    loadBtn.onClick = [this] { loadPresetFile(); };

    refreshPresetDisplay();

    setResizable (true, true);
    if (auto* c = getConstrainer())
    {
        c->setFixedAspectRatio ((double) designW / (double) designH);
        c->setSizeLimits (designW * 6 / 10, designH * 6 / 10, designW * 2, designH * 2);
    }
    setSize (designW, designH);
    startTimerHz (30);
}

ChipcoreEditor::~ChipcoreEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

// ---------------------------------------------------------------------------
ChipcoreEditor::Panel& ChipcoreEditor::addPanel (juce::Rectangle<int> r, const juce::String& title, juce::Colour accent)
{
    panels.push_back ({ r, title, accent });
    return panels.back();
}

juce::Rectangle<int> ChipcoreEditor::cell (const Panel& p, int col, int row) const
{
    return { p.r.getX() + kPad + col * kCellW, p.r.getY() + 48 + row * kRowStep, kCellW, kCellH };
}

juce::Rectangle<int> ChipcoreEditor::comboRow (const Panel& p, int x, int w) const
{
    return { p.r.getX() + kPad + x, p.r.getY() + 24, w, 20 };
}

juce::Slider& ChipcoreEditor::knob (const char* id, const juce::String& caption, juce::Colour accent, juce::Rectangle<int> r)
{
    auto s = std::make_unique<juce::Slider> (juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox);
    s->setName (caption);
    s->setColour (juce::Slider::rotarySliderFillColourId, accent);
    s->setRotaryParameters (juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
    s->setMouseDragSensitivity (220);
    s->setVelocityBasedMode (false);
    s->setRepaintsOnMouseActivity (true);
    s->setBounds (r);
    canvas.addAndMakeVisible (*s);
    sliderAtt.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, id, *s));
    if (auto* prm = proc.apvts.getParameter (id))
        s->setDoubleClickReturnValue (true, prm->convertFrom0to1 (prm->getDefaultValue()));
    sliders.push_back (std::move (s));
    return *sliders.back();
}

juce::ComboBox& ChipcoreEditor::combo (const char* id, juce::Colour accent, juce::Rectangle<int> r)
{
    auto c = std::make_unique<juce::ComboBox>();
    const int idx = chip::paramIndex (id);
    jassert (idx >= 0);
    const auto& info = chip::paramTable()[(size_t) idx];
    int itemId = 1;
    for (auto* name : *info.choices) c->addItem (name, itemId++);
    c->setColour (juce::ComboBox::outlineColourId, accent);
    c->setBounds (r);
    canvas.addAndMakeVisible (*c);
    comboAtt.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, id, *c));
    combos.push_back (std::move (c));
    return *combos.back();
}

juce::ToggleButton& ChipcoreEditor::toggle (const char* id, const juce::String& caption, juce::Colour accent, juce::Rectangle<int> r)
{
    auto t = std::make_unique<juce::ToggleButton> (caption);
    t->setColour (juce::ToggleButton::tickColourId, accent);
    t->setBounds (r);
    canvas.addAndMakeVisible (*t);
    toggleAtt.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, id, *t));
    toggles.push_back (std::move (t));
    return *toggles.back();
}

// ---------------------------------------------------------------------------
// Presets
// ---------------------------------------------------------------------------
void ChipcoreEditor::buildPresetMenu()
{
    presetBox.clear (juce::dontSendNotification);
    const auto& bank = chip::factoryPresets();
    for (size_t c = 0; c < std::size (kCategoryOrder); ++c)
    {
        bool headingAdded = false;
        for (size_t i = 0; i < bank.size(); ++i)
        {
            if (juce::String (bank[i].category) != kCategoryOrder[c]) continue;
            if (! headingAdded) { presetBox.addSectionHeading (kCategoryTitle[c]); headingAdded = true; }
            presetBox.addItem (bank[i].name, (int) i + 1);
        }
    }
}

void ChipcoreEditor::refreshPresetDisplay()
{
    const int idx = proc.getCurrentPresetIndex();
    const auto name = proc.getCurrentPresetName();
    if (idx == shownPreset && name == shownName) return;
    shownPreset = idx;
    shownName = name;
    if (idx >= 0) presetBox.setSelectedId (idx + 1, juce::dontSendNotification);
    else
    {
        presetBox.setSelectedId (0, juce::dontSendNotification);
        presetBox.setText (name, juce::dontSendNotification);
    }
}

void ChipcoreEditor::stepPreset (int delta)
{
    // step in menu (category) order, not raw bank order
    const auto& bank = chip::factoryPresets();
    std::vector<int> order;
    for (auto* cat : kCategoryOrder)
        for (size_t i = 0; i < bank.size(); ++i)
            if (juce::String (bank[i].category) == cat) order.push_back ((int) i);
    if (order.empty()) return;
    const int cur = proc.getCurrentPresetIndex();
    int pos = 0;
    for (size_t k = 0; k < order.size(); ++k) if (order[k] == cur) pos = (int) k;
    if (cur < 0) pos = delta > 0 ? -1 : 0;
    const int n = (int) order.size();
    pos = ((pos + delta) % n + n) % n;
    proc.loadPreset (order[(size_t) pos]);
    refreshPresetDisplay();
}

void ChipcoreEditor::savePreset()
{
    auto start = ChipcoreProcessor::userPresetFolder().getChildFile (
        juce::File::createLegalFileName (proc.getCurrentPresetName()) + " MY.chipcore");
    chooser = std::make_unique<juce::FileChooser> ("Save CHIPCORE preset", start, "*.chipcore");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles
                              | juce::FileBrowserComponent::warnAboutOverwriting,
                          [this] (const juce::FileChooser& fc)
                          {
                              auto f = fc.getResult();
                              if (f == juce::File()) return;
                              proc.savePresetToFile (f.withFileExtension ("chipcore"));
                              refreshPresetDisplay();
                          });
}

void ChipcoreEditor::loadPresetFile()
{
    chooser = std::make_unique<juce::FileChooser> ("Load CHIPCORE preset", ChipcoreProcessor::userPresetFolder(), "*.chipcore");
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                          [this] (const juce::FileChooser& fc)
                          {
                              auto f = fc.getResult();
                              if (f.existsAsFile()) proc.loadPresetFromFile (f);
                              refreshPresetDisplay();
                          });
}

// ---------------------------------------------------------------------------
void ChipcoreEditor::timerCallback()
{
    refreshPresetDisplay();
    proc.scope.readLatest (scopeData.data(), (int) scopeData.size());
    canvas.repaint (scopeArea.expanded (2));
    canvas.repaint (lfoArea.expanded (2));
    const int v = proc.getActiveVoices();
    if (v != lastVoices) { lastVoices = v; canvas.repaint (voicesArea.expanded (2)); }
}

void ChipcoreEditor::resized()
{
    const float scale = (float) getWidth() / (float) designW;
    canvas.setTransform (juce::AffineTransform::scale (scale));
}

void ChipcoreEditor::paint (juce::Graphics& g)
{
    g.fillAll (pix::bg);
}

void ChipcoreEditor::drawLogo (juce::Graphics& g, int x, int y)
{
    using namespace pix;
    // original pixel skull
    static const std::vector<const char*> skull {
        "..######..",
        ".########.",
        "##########",
        "##..##..##",
        "##..##..##",
        "##########",
        ".###..###.",
        "..######..",
        "..#.##.#..",
    };
    bitmap (g, x + 2, y + 6, 4, skull, pink.darker (0.6f));
    bitmap (g, x, y + 4, 4, skull, text);

    const int tx = x + 54;
    g.setFont (lnf.font (24.0f));
    g.setColour (cyan.darker (0.3f));
    g.drawText ("CHIPCORE", tx + 4, y + 8, 280, 26, juce::Justification::left, false);
    g.setColour (pink);
    g.drawText ("CHIPCORE", tx + 2, y + 6, 280, 26, juce::Justification::left, false);
    g.setColour (text);
    g.drawText ("CHIPCORE", tx, y + 4, 280, 26, juce::Justification::left, false);
    g.setFont (lnf.font (8.0f));
    g.setColour (yellow);
    g.drawText ("NINTENDOCORE SYNTH  V1.0", tx, y + 34, 280, 10, juce::Justification::left, false);
}

void ChipcoreEditor::drawScope (juce::Graphics& g, juce::Rectangle<int> r)
{
    using namespace pix;
    bevel (g, r, juce::Colour (0xff050a08), panelLo, panelHi);
    const auto inner = r.reduced (4);
    g.setColour (juce::Colour (0xff0f2a1a));
    for (int x = inner.getX(); x < inner.getRight(); x += 16) g.fillRect (x, inner.getY(), 1, inner.getHeight());
    g.fillRect (inner.getX(), inner.getCentreY(), inner.getWidth(), 1);

    // trigger on rising zero crossing so the wave stands still
    const int n = (int) scopeData.size();
    const int span = 256;
    int start = 0;
    for (int i = 1; i < n - span; ++i)
        if (scopeData[(size_t) i - 1] <= 0.0f && scopeData[(size_t) i] > 0.0f) { start = i; break; }

    const int cols = inner.getWidth() / P;
    int prevY = -1;
    g.setColour (green);
    for (int c = 0; c < cols; ++c)
    {
        const float s = scopeData[(size_t) juce::jlimit (0, n - 1, start + c * span / cols)];
        const int yy = inner.getCentreY() - (int) std::round (juce::jlimit (-1.0f, 1.0f, s * 1.4f) * (float) (inner.getHeight() / 2 - P) / (float) P) * P;
        const int x = inner.getX() + c * P;
        if (prevY < 0) prevY = yy;
        const int y0 = juce::jmin (prevY, yy), y1 = juce::jmax (prevY, yy);
        g.fillRect (x, y0, P, y1 - y0 + P);
        prevY = yy;
    }
    g.setFont (lnf.font (8.0f));
    g.setColour (green.withAlpha (0.6f));
    g.drawText ("OUT", inner.withHeight (10).reduced (2, 0), juce::Justification::right, false);
}

void ChipcoreEditor::drawLfoPreview (juce::Graphics& g, juce::Rectangle<int> r)
{
    using namespace pix;
    bevel (g, r, panelLo, panelLo, panelHi);
    const auto inner = r.reduced (4);
    const int wave = (int) proc.apvts.getRawParameterValue ("lfo_wave")->load();
    const float amt = proc.apvts.getRawParameterValue ("lfo_amt")->load();
    const float rate = proc.apvts.getRawParameterValue ("lfo_rate")->load();
    const float amp = 0.2f + 0.8f * amt;
    const int cols = inner.getWidth() / P;
    const double t0 = juce::Time::getMillisecondCounterHiRes() * 0.001 * juce::jmin (rate, 4.0f) * 0.25;
    juce::Random rnd (1234);
    float sh[8];
    for (auto& v : sh) v = rnd.nextFloat() * 2.0f - 1.0f;
    int prevY = -1;
    g.setColour (purple);
    for (int c = 0; c < cols; ++c)
    {
        double ph = (double) c / (double) cols * 2.0 + t0;
        ph -= std::floor (ph);
        float x = 0.0f;
        switch (wave)
        {
            case 0: x = (float) std::sin (juce::MathConstants<double>::twoPi * ph); break;
            case 1: x = 1.0f - 4.0f * (float) std::abs (ph - 0.5); break;
            case 2: x = ph < 0.5 ? 1.0f : -1.0f; break;
            case 3: x = 2.0f * (float) ph - 1.0f; break;
            default: x = sh[(c * 8 / juce::jmax (1, cols)) & 7]; break;
        }
        const int yy = inner.getCentreY() - (int) std::round (x * amp * (float) (inner.getHeight() / 2 - P) / (float) P) * P;
        const int px_ = inner.getX() + c * P;
        if (prevY < 0) prevY = yy;
        g.fillRect (px_, juce::jmin (prevY, yy), P, std::abs (prevY - yy) + P);
        prevY = yy;
    }
}

void ChipcoreEditor::paintCanvas (juce::Graphics& g)
{
    using namespace pix;
    g.fillAll (bg);
    // subtle dotted background
    g.setColour (juce::Colour (0xff15122e));
    for (int y = 0; y < designH; y += 8)
        for (int x = (y / 8) % 2 * 4; x < designW; x += 8)
            g.fillRect (x, y, 2, 2);

    // header
    bevel (g, { 8, 8, designW - 16, 56 }, panel, panelHi, panelLo);
    drawLogo (g, 18, 10);
    g.setFont (lnf.font (8.0f));
    g.setColour (dim.brighter (0.4f));
    g.drawText ("PRESET", 392, 15, 120, 10, juce::Justification::left, false);
    g.drawText (juce::String (chip::factoryPresets().size()) + " FACTORY", 602, 15, 120, 10, juce::Justification::right, false);

    // panels
    for (const auto& p : panels)
    {
        bevel (g, p.r, panel, panelHi, panelLo);
        const juce::Rectangle<int> bar (p.r.getX() + 2, p.r.getY() + 2, p.r.getWidth() - 4, 16);
        g.setColour (p.accent);
        g.fillRect (bar);
        g.setColour (p.accent.darker (0.5f));
        g.fillRect (bar.getX(), bar.getBottom() - 2, bar.getWidth(), 2);
        g.setFont (lnf.font (8.0f));
        g.setColour (panelLo);
        g.drawText (p.title, bar.reduced (6, 0).translated (0, 1), juce::Justification::centredLeft, false);
        for (int i = 0; i < 3; ++i)
            g.fillRect (bar.getRight() - 10 - i * 6, bar.getY() + 6, 2, 2);
    }

    // captions
    g.setFont (lnf.font (8.0f));
    g.setColour (cyan);
    {
        const auto& arpPanel = panels[7];
        const auto c2 = cell (arpPanel, 2, 0);
        g.drawText ("CHORD", c2.getX() + 2, c2.getY() + 8, 100, 10, juce::Justification::left, false);
        g.setColour (dim.brighter (0.3f));
        const auto c0 = cell (arpPanel, 0, 1);
        g.drawText ("CHORD = 1 KEY CHORDS", c0.getX(), c0.getY() + 14, 232, 10, juce::Justification::left, false);
        g.drawText ("ARP = HOLD KEYS", c0.getX(), c0.getY() + 30, 232, 10, juce::Justification::left, false);
        g.drawText ("SYNCS TO HOST TEMPO", c0.getX(), c0.getY() + 46, 232, 10, juce::Justification::left, false);
    }
    {
        const auto& lfoPanel = panels[6];
        const auto c0 = cell (lfoPanel, 0, 1);
        g.setColour (dim.brighter (0.3f));
        g.drawText ("MOD WHEEL = VIBRATO", c0.getX(), c0.getY() + 14, 232, 10, juce::Justification::left, false);
        g.drawText ("SHAPE: PW / FOLD / FM", c0.getX(), c0.getY() + 30, 232, 10, juce::Justification::left, false);
        g.drawText ("INDEX / GB WAVE", c0.getX(), c0.getY() + 46, 232, 10, juce::Justification::left, false);
    }

    drawScope (g, scopeArea);
    drawLfoPreview (g, lfoArea);

    // voice LEDs
    g.setColour (dim.brighter (0.4f));
    g.drawText ("VOICES", voicesArea.getX(), voicesArea.getY(), 100, 10, juce::Justification::left, false);
    const int active = juce::jmax (0, lastVoices);
    for (int i = 0; i < chip::Engine::kMaxVoices; ++i)
    {
        const int x = voicesArea.getX() + (i % 8) * 18;
        const int y = voicesArea.getY() + 16 + (i / 8) * 18;
        bevel (g, { x, y, 14, 14 }, i < active ? yellow : panelLo, i < active ? yellow.brighter (0.5f) : panelLo, i < active ? yellow.darker (0.5f) : panelHi);
    }
}
