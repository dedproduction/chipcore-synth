#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "PixelLookAndFeel.h"

class ChipcoreEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit ChipcoreEditor (ChipcoreProcessor&);
    ~ChipcoreEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    static constexpr int designW = 1040, designH = 620;

private:
    struct Panel { juce::Rectangle<int> r; juce::String title; juce::Colour accent; };

    // Fixed-size canvas that is scaled to the window size
    struct Canvas : juce::Component
    {
        explicit Canvas (ChipcoreEditor& e) : ed (e) {}
        void paint (juce::Graphics& g) override { ed.paintCanvas (g); }
        ChipcoreEditor& ed;
    };

    void timerCallback() override;
    void paintCanvas (juce::Graphics&);
    void drawScope (juce::Graphics&, juce::Rectangle<int>);
    void drawLfoPreview (juce::Graphics&, juce::Rectangle<int>);
    void drawLogo (juce::Graphics&, int x, int y);

    Panel& addPanel (juce::Rectangle<int> r, const juce::String& title, juce::Colour accent);
    juce::Slider& knob (const char* id, const juce::String& caption, juce::Colour accent, juce::Rectangle<int> r);
    juce::ComboBox& combo (const char* id, juce::Colour accent, juce::Rectangle<int> r);
    juce::ToggleButton& toggle (const char* id, const juce::String& caption, juce::Colour accent, juce::Rectangle<int> r);
    juce::Rectangle<int> cell (const Panel&, int col, int row) const;
    juce::Rectangle<int> comboRow (const Panel&, int x, int w) const;

    void buildPresetMenu();
    void refreshPresetDisplay();
    void stepPreset (int delta);
    void savePreset();
    void loadPresetFile();

    ChipcoreProcessor& proc;
    pix::LookAndFeel lnf;
    Canvas canvas { *this };

    std::vector<Panel> panels;
    juce::Rectangle<int> scopeArea, lfoArea, voicesArea;

    juce::ComboBox presetBox;
    juce::TextButton prevBtn { "<" }, nextBtn { ">" }, initBtn { "INIT" }, saveBtn { "SAVE" }, loadBtn { "LOAD" };
    std::unique_ptr<juce::FileChooser> chooser;
    int shownPreset = -999;
    juce::String shownName;

    std::vector<std::unique_ptr<juce::Slider>> sliders;
    std::vector<std::unique_ptr<juce::ComboBox>> combos;
    std::vector<std::unique_ptr<juce::ToggleButton>> toggles;
    // attachments are declared after the components so they are destroyed first
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliderAtt;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboAtt;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> toggleAtt;

    std::array<float, 512> scopeData {};
    int lastVoices = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChipcoreEditor)
};
