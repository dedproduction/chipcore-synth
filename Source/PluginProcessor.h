#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "ParamTable.h"
#include "Presets.h"
#include "dsp/Engine.h"
#include <array>
#include <atomic>

// Lock-free buffer the editor reads to draw the oscilloscope
struct ScopeBuffer
{
    static constexpr int size = 4096;
    std::array<float, size> data {};
    std::atomic<int> writePos { 0 };

    void push (const float* x, int n)
    {
        int w = writePos.load (std::memory_order_relaxed);
        for (int i = 0; i < n; ++i) { data[(size_t) w] = x[i]; w = (w + 1) & (size - 1); }
        writePos.store (w, std::memory_order_release);
    }
    void readLatest (float* out, int n) const
    {
        int r = (writePos.load (std::memory_order_acquire) - n) & (size - 1);
        for (int i = 0; i < n; ++i) { out[i] = data[(size_t) r]; r = (r + 1) & (size - 1); }
    }
};

class ChipcoreProcessor : public juce::AudioProcessor
{
public:
    ChipcoreProcessor();
    ~ChipcoreProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override  { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // ---- presets ----
    void loadPreset (int index);
    void initPatch();
    bool savePresetToFile (const juce::File& file);
    bool loadPresetFromFile (const juce::File& file);
    static juce::File userPresetFolder();

    int getCurrentPresetIndex() const       { return currentPreset.load(); }
    juce::String getCurrentPresetName() const;
    int getActiveVoices() const             { return activeVoices.load(); }

    juce::AudioProcessorValueTreeState apvts;
    ScopeBuffer scope;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void setParam (const char* id, float plainValue);
    void handleMidi (const juce::MidiMessage& m);

    chip::Engine engine;
    std::array<std::atomic<float>*, chip::NUM_PARAMS> raw {};
    std::array<float, chip::NUM_PARAMS> values {};
    std::atomic<int> currentPreset { 0 }, activeVoices { 0 };
    juce::String userPresetName;
    juce::CriticalSection nameLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChipcoreProcessor)
};
