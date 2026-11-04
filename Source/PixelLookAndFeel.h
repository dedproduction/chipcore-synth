// CHIPCORE - retro pixel look and feel
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "BinaryData.h"

namespace pix
{
// palette
const juce::Colour bg        { 0xff0d0b1e };
const juce::Colour panel     { 0xff1a1638 };
const juce::Colour panelHi   { 0xff3a3470 };
const juce::Colour panelLo   { 0xff07060f };
const juce::Colour text      { 0xffe8e6ff };
const juce::Colour dim       { 0xff6c66a8 };
const juce::Colour pink      { 0xffff4f9a };
const juce::Colour cyan      { 0xff3ce8ff };
const juce::Colour yellow    { 0xffffd23f };
const juce::Colour green     { 0xff5cf06a };
const juce::Colour orange    { 0xffff8c3a };
const juce::Colour purple    { 0xffb56cff };
const juce::Colour red       { 0xffff4a4a };

constexpr int P = 2; // size of one "pixel" in design units

inline void bevel (juce::Graphics& g, juce::Rectangle<int> r, juce::Colour fill, juce::Colour hi, juce::Colour lo, int t = P)
{
    g.setColour (fill); g.fillRect (r);
    g.setColour (hi);
    g.fillRect (r.getX(), r.getY(), r.getWidth(), t);
    g.fillRect (r.getX(), r.getY(), t, r.getHeight());
    g.setColour (lo);
    g.fillRect (r.getX(), r.getBottom() - t, r.getWidth(), t);
    g.fillRect (r.getRight() - t, r.getY(), t, r.getHeight());
}

// draws a 1-bit bitmap given as strings ('#' = filled)
inline void bitmap (juce::Graphics& g, int x, int y, int px, const std::vector<const char*>& rows, juce::Colour c)
{
    g.setColour (c);
    for (size_t r = 0; r < rows.size(); ++r)
        for (int col = 0; rows[r][col] != 0; ++col)
            if (rows[r][col] == '#') g.fillRect (x + col * px, y + (int) r * px, px, px);
}

class LookAndFeel : public juce::LookAndFeel_V4
{
public:
    LookAndFeel()
    {
        typeface = juce::Typeface::createSystemTypefaceFor (BinaryData::PressStart2PRegular_ttf,
                                                            BinaryData::PressStart2PRegular_ttfSize);
        setDefaultSansSerifTypeface (typeface);

        setColour (juce::ResizableWindow::backgroundColourId, bg);
        setColour (juce::PopupMenu::backgroundColourId, panelLo);
        setColour (juce::PopupMenu::textColourId, text);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, pink);
        setColour (juce::PopupMenu::highlightedTextColourId, panelLo);
        setColour (juce::PopupMenu::headerTextColourId, yellow);
        setColour (juce::ComboBox::textColourId, text);
        setColour (juce::ComboBox::backgroundColourId, panelLo);
        setColour (juce::TextButton::textColourOffId, text);
        setColour (juce::TextButton::textColourOnId, panelLo);
        setColour (juce::ToggleButton::textColourId, text);
        setColour (juce::Label::textColourId, text);
        setColour (juce::Slider::rotarySliderFillColourId, pink);
        setColour (juce::ScrollBar::thumbColourId, pink);
    }

    juce::Font font (float h) const { return juce::Font (juce::FontOptions (typeface).withHeight (h)); }

    juce::Typeface::Ptr getTypefaceForFont (const juce::Font&) override { return typeface; }

    // ---- knobs -------------------------------------------------------------
    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider& s) override
    {
        const auto accent = s.findColour (juce::Slider::rotarySliderFillColourId);
        const juce::Rectangle<int> area (x, y, w, h);
        const auto labelArea = area.withTop (area.getBottom() - 12);
        const auto knobArea  = area.withTrimmedBottom (14);

        const int cx = (knobArea.getCentreX() / P) * P;
        const int cy = (knobArea.getCentreY() / P) * P;
        const int r  = juce::jmax (4, juce::jmin (knobArea.getWidth(), knobArea.getHeight()) / (2 * P) - 4);

        // LED ring
        const int segs = 15;
        const bool bipolar = s.getMinimum() < 0.0 && s.getMaximum() > 0.0;
        const float centrePos = bipolar ? (float) s.valueToProportionOfLength (0.0) : 0.0f;
        for (int i = 0; i < segs; ++i)
        {
            const float t = (float) i / (float) (segs - 1);
            const float a = startAngle + (endAngle - startAngle) * t;
            const int px = cx + juce::roundToInt (std::sin (a) * (float) (r + 3) * P) - P;
            const int py = cy - juce::roundToInt (std::cos (a) * (float) (r + 3) * P) - P;
            bool lit;
            if (bipolar) lit = (t >= juce::jmin (pos, centrePos) - 0.035f && t <= juce::jmax (pos, centrePos) + 0.035f);
            else lit = t <= pos + 0.035f && pos > 0.001f;
            g.setColour (lit ? accent : panelHi);
            g.fillRect ((px / P) * P, (py / P) * P, P * 2, P * 2);
        }

        // body (pixel circle)
        for (int dy = -r; dy < r; ++dy)
        {
            const float yy = (float) dy + 0.5f;
            const int half = (int) std::sqrt (juce::jmax (0.0f, (float) (r * r) - yy * yy));
            const int rowY = cy + dy * P;
            g.setColour (dy < -r / 3 ? juce::Colour (0xff4a4390) : juce::Colour (0xff2f2a5e));
            g.fillRect (cx - half * P, rowY, half * 2 * P, P);
            g.setColour (panelLo);
            g.fillRect (cx - half * P, rowY, P, P);
            g.fillRect (cx + (half - 1) * P, rowY, P, P);
        }
        // top/bottom outline
        g.setColour (panelLo);
        {
            const int half = (int) std::sqrt ((float) (r * r) - ((float) r - 0.5f) * ((float) r - 0.5f));
            g.fillRect (cx - half * P, cy - r * P, half * 2 * P, P);
            g.fillRect (cx - half * P, cy + (r - 1) * P, half * 2 * P, P);
        }

        // pointer
        const float a = startAngle + (endAngle - startAngle) * pos;
        g.setColour (s.isMouseButtonDown() ? accent : text);
        for (float t = 0.15f; t <= 0.85f; t += 0.1f)
        {
            const int px = cx + (int) std::floor (std::sin (a) * (float) r * t + 0.5f) * P;
            const int py = cy - (int) std::floor (std::cos (a) * (float) r * t + 0.5f) * P;
            g.fillRect (px - P, py - P, P * 2, P * 2);
        }

        // caption / value
        const bool showValue = s.isMouseOverOrDragging();
        g.setFont (font (8.0f));
        g.setColour (showValue ? accent : dim.brighter (0.5f));
        g.drawText (showValue ? s.getTextFromValue (s.getValue()) : s.getName(),
                    labelArea, juce::Justification::centred, false);
    }

    // ---- combo boxes ---------------------------------------------------------
    void drawComboBox (juce::Graphics& g, int w, int h, bool down, int, int, int, int, juce::ComboBox& box) override
    {
        const auto accent = box.findColour (juce::ComboBox::outlineColourId);
        bevel (g, { 0, 0, w, h }, panelLo, down ? accent : panelHi, down ? panelHi : accent.withAlpha (0.6f));
        // arrow
        const int ax = w - 12, ay = h / 2 - 2;
        g.setColour (accent);
        g.fillRect (ax, ay, 6, 2);
        g.fillRect (ax + 2, ay + 2, 2, 2);
    }

    juce::Font getComboBoxFont (juce::ComboBox&) override { return font (8.0f); }

    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override
    {
        label.setBounds (4, 1, box.getWidth() - 18, box.getHeight() - 2);
        label.setFont (getComboBoxFont (box));
        label.setJustificationType (juce::Justification::centredLeft);
        label.setMinimumHorizontalScale (0.6f);
    }

    // ---- popup menus -----------------------------------------------------------
    juce::Font getPopupMenuFont() override { return font (8.0f); }

    void drawPopupMenuBackground (juce::Graphics& g, int w, int h) override
    {
        bevel (g, { 0, 0, w, h }, panelLo, panelHi, pink.withAlpha (0.6f));
    }

    void drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area, bool isSeparator, bool isActive,
                            bool isHighlighted, bool isTicked, bool, const juce::String& text_, const juce::String&,
                            const juce::Drawable*, const juce::Colour*) override
    {
        if (isSeparator) { g.setColour (panelHi); g.fillRect (area.reduced (6, 0).withHeight (2).withY (area.getCentreY())); return; }
        const auto r = area.reduced (2, 0);
        if (isHighlighted && isActive) { g.setColour (pink); g.fillRect (r); }
        g.setColour (isHighlighted ? panelLo : (isTicked ? yellow : (isActive ? text : dim)));
        g.setFont (font (8.0f));
        if (isTicked) g.fillRect (r.getX() + 6, r.getCentreY() - 2, 4, 4);
        g.drawText (text_, r.withTrimmedLeft (16), juce::Justification::centredLeft, true);
    }

    void drawPopupMenuSectionHeader (juce::Graphics& g, const juce::Rectangle<int>& area, const juce::String& name) override
    {
        g.setColour (yellow);
        g.setFont (font (8.0f));
        g.drawText ("- " + name + " -", area.withTrimmedLeft (8), juce::Justification::centredLeft, false);
    }

    void getIdealPopupMenuItemSize (const juce::String& t, bool isSep, int standardHeight, int& w, int& h) override
    {
        juce::ignoreUnused (standardHeight);
        if (isSep) { w = 50; h = 8; return; }
        h = 18;
        w = juce::GlyphArrangement::getStringWidthInt (font (8.0f), t) + 40;
    }

    // ---- buttons ---------------------------------------------------------------
    void drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down) override
    {
        const auto accent = b.findColour (juce::TextButton::buttonOnColourId);
        auto r = b.getLocalBounds();
        if (b.getToggleState() || down) bevel (g, r, accent, accent.darker (0.4f), accent.brighter (0.4f));
        else bevel (g, r, over ? juce::Colour (0xff2a2552) : panel, panelHi, panelLo);
    }

    juce::Font getTextButtonFont (juce::TextButton&, int) override { return font (8.0f); }

    void drawButtonText (juce::Graphics& g, juce::TextButton& b, bool, bool down) override
    {
        g.setFont (font (8.0f));
        g.setColour ((b.getToggleState() || down) ? panelLo : text);
        g.drawText (b.getButtonText(), b.getLocalBounds().translated (0, down ? 1 : 0), juce::Justification::centred, false);
    }

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool over, bool) override
    {
        const auto accent = b.findColour (juce::ToggleButton::tickColourId);
        const int box = 12;
        const int y = (b.getHeight() - box) / 2;
        bevel (g, { 0, y, box, box }, panelLo, panelLo, over ? accent : panelHi);
        if (b.getToggleState())
        {
            g.setColour (accent);
            g.fillRect (3, y + 3, box - 6, box - 6);
        }
        g.setFont (font (8.0f));
        g.setColour (b.getToggleState() ? accent : text);
        g.drawText (b.getButtonText(), b.getLocalBounds().withTrimmedLeft (box + 5), juce::Justification::centredLeft, false);
    }

private:
    juce::Typeface::Ptr typeface;
};
} // namespace pix
