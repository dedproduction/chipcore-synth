#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr int kInitPreset = -1;
constexpr int kUserPreset = -2;

juce::String formatValue (const chip::ParamInfo& info, float v)
{
    const juce::String unit (info.unit);
    if (unit == "%")
    {
        if (info.min < 0.0f) return (v >= 0 ? "+" : "") + juce::String (juce::roundToInt (v * 100.0f)) + "%";
        return juce::String (juce::roundToInt (v * 100.0f)) + "%";
    }
    if (unit == "ms")
    {
        if (v < 0.5f && juce::String (info.id) == "noise_decay") return "AMP";
        if (v < 0.5f && juce::String (info.id) == "glide") return "OFF";
        return v < 1000.0f ? juce::String (v, v < 10.0f ? 1 : 0) + "ms" : juce::String (v / 1000.0f, 2) + "s";
    }
    if (unit == "Hz")
    {
        if (v >= 1000.0f) return juce::String (v / 1000.0f, 1) + "k";
        return juce::String (v, v < 10.0f ? 2 : (v < 100.0f ? 1 : 0)) + "Hz";
    }
    if (unit == "st")
    {
        const juce::String s = info.kind == chip::Kind::Int ? juce::String (juce::roundToInt (v)) : juce::String (v, 1);
        return (v > 0 ? "+" : "") + s + "st";
    }
    if (unit == "ct")  return (v > 0 ? "+" : "") + juce::String (juce::roundToInt (v)) + "ct";
    if (unit == "dB")  return juce::String (v, 1) + "dB";
    if (unit == "bit") return v >= 15.99f ? juce::String ("OFF") : juce::String (v, 1) + "b";
    if (unit == "oct") return (v > 0 ? "+" : "") + juce::String (juce::roundToInt (v));
    if (unit == "v")   return juce::String (juce::roundToInt (v)) + "V";
    return juce::String (v, 2);
}
} // namespace

// ---------------------------------------------------------------------------
juce::AudioProcessorValueTreeState::ParameterLayout ChipcoreProcessor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    for (const auto& info : chip::paramTable())
    {
        const juce::ParameterID pid { info.id, 1 };
        switch (info.kind)
        {
            case chip::Kind::Float:
            {
                juce::NormalisableRange<float> range (info.min, info.max);
                if (info.skewCentre > 0.0f) range.setSkewForCentre (info.skewCentre);
                auto attrs = juce::AudioParameterFloatAttributes().withStringFromValueFunction (
                    [info] (float v, int) { return formatValue (info, v); });
                layout.add (std::make_unique<juce::AudioParameterFloat> (pid, info.name, range, info.def, attrs));
                break;
            }
            case chip::Kind::Int:
            {
                auto attrs = juce::AudioParameterIntAttributes().withStringFromValueFunction (
                    [info] (int v, int) { return formatValue (info, (float) v); });
                layout.add (std::make_unique<juce::AudioParameterInt> (pid, info.name, (int) info.min, (int) info.max, (int) info.def, attrs));
                break;
            }
            case chip::Kind::Bool:
                layout.add (std::make_unique<juce::AudioParameterBool> (pid, info.name, info.def > 0.5f));
                break;
            case chip::Kind::Choice:
            {
                juce::StringArray items;
                for (auto* c : *info.choices) items.add (c);
                layout.add (std::make_unique<juce::AudioParameterChoice> (pid, info.name, items, (int) info.def));
                break;
            }
        }
    }
    return layout;
}

ChipcoreProcessor::ChipcoreProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "CHIPCORE", createLayout())
{
    const auto& t = chip::paramTable();
    for (size_t i = 0; i < t.size(); ++i)
    {
        raw[i] = apvts.getRawParameterValue (t[i].id);
        jassert (raw[i] != nullptr);
        values[i] = t[i].def;
    }
    loadPreset (0);
}

bool ChipcoreProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainInputChannelSet().isDisabled();
}

void ChipcoreProcessor::prepareToPlay (double sampleRate, int)
{
    engine.prepare (sampleRate);
}

void ChipcoreProcessor::handleMidi (const juce::MidiMessage& m)
{
    if (m.isNoteOn())                    engine.noteOn (m.getNoteNumber(), m.getFloatVelocity());
    else if (m.isNoteOff())              engine.noteOff (m.getNoteNumber());
    else if (m.isPitchWheel())           engine.pitchBend ((float) (m.getPitchWheelValue() - 8192) / 8192.0f);
    else if (m.isSustainPedalOn())       engine.sustainPedal (true);
    else if (m.isSustainPedalOff())      engine.sustainPedal (false);
    else if (m.isController() && m.getControllerNumber() == 1)
        engine.modWheel ((float) m.getControllerValue() / 127.0f);
    else if (m.isAllNotesOff() || m.isAllSoundOff())
        engine.allNotesOff();
}

void ChipcoreProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    buffer.clear();
    if (buffer.getNumChannels() < 2 || numSamples == 0) return;

    for (size_t i = 0; i < values.size(); ++i)
        values[i] = raw[i]->load (std::memory_order_relaxed);

    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto bpm = pos->getBpm())
                engine.setTempo (*bpm);

    engine.beginBlock (values.data());

    float* L = buffer.getWritePointer (0);
    float* R = buffer.getWritePointer (1);
    int pos = 0;
    for (const auto meta : midi)
    {
        const int t = juce::jlimit (0, numSamples, meta.samplePosition);
        if (t > pos) { engine.render (values.data(), L + pos, R + pos, t - pos); pos = t; }
        handleMidi (meta.getMessage());
    }
    if (pos < numSamples) engine.render (values.data(), L + pos, R + pos, numSamples - pos);

    scope.push (L, numSamples);
    activeVoices.store (engine.activeVoices(), std::memory_order_relaxed);
}

// ---------------------------------------------------------------------------
// Presets
// ---------------------------------------------------------------------------
void ChipcoreProcessor::setParam (const char* id, float plainValue)
{
    if (auto* p = apvts.getParameter (id))
        p->setValueNotifyingHost (p->convertTo0to1 (plainValue));
}

void ChipcoreProcessor::initPatch()
{
    for (const auto& info : chip::paramTable())
        setParam (info.id, info.def);
    currentPreset = kInitPreset;
}

void ChipcoreProcessor::loadPreset (int index)
{
    const auto& bank = chip::factoryPresets();
    if (bank.empty()) return;
    index = juce::jlimit (0, (int) bank.size() - 1, index);
    for (const auto& info : chip::paramTable())
        setParam (info.id, info.def);
    for (const auto& kv : bank[(size_t) index].values)
        setParam (kv.first, kv.second);
    currentPreset = index;
}

juce::String ChipcoreProcessor::getCurrentPresetName() const
{
    const int idx = currentPreset.load();
    if (idx >= 0 && idx < (int) chip::factoryPresets().size())
        return chip::factoryPresets()[(size_t) idx].name;
    if (idx == kInitPreset) return "INIT";
    const juce::ScopedLock sl (nameLock);
    return userPresetName;
}

int ChipcoreProcessor::getNumPrograms()                  { return (int) chip::factoryPresets().size(); }
int ChipcoreProcessor::getCurrentProgram()               { return juce::jmax (0, currentPreset.load()); }
const juce::String ChipcoreProcessor::getProgramName (int i)
{
    if (i >= 0 && i < getNumPrograms()) return chip::factoryPresets()[(size_t) i].name;
    return {};
}
void ChipcoreProcessor::setCurrentProgram (int index)
{
    if (index >= 0 && index < getNumPrograms() && index != currentPreset.load())
        loadPreset (index);
}

juce::File ChipcoreProcessor::userPresetFolder()
{
    auto dir = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("CHIPCORE Presets");
    dir.createDirectory();
    return dir;
}

bool ChipcoreProcessor::savePresetToFile (const juce::File& file)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    if (xml == nullptr) return false;
    xml->setAttribute ("presetName", file.getFileNameWithoutExtension());
    if (! xml->writeTo (file)) return false;
    {
        const juce::ScopedLock sl (nameLock);
        userPresetName = file.getFileNameWithoutExtension();
    }
    currentPreset = kUserPreset;
    return true;
}

bool ChipcoreProcessor::loadPresetFromFile (const juce::File& file)
{
    auto xml = juce::XmlDocument::parse (file);
    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType())) return false;
    // reset to defaults first so missing params in older files behave
    for (const auto& info : chip::paramTable())
        setParam (info.id, info.def);
    for (auto* child : xml->getChildIterator())
    {
        const auto id = child->getStringAttribute ("id");
        if (auto* p = apvts.getParameter (id))
            p->setValueNotifyingHost (p->convertTo0to1 ((float) child->getDoubleAttribute ("value")));
    }
    {
        const juce::ScopedLock sl (nameLock);
        userPresetName = file.getFileNameWithoutExtension();
    }
    currentPreset = kUserPreset;
    return true;
}

// ---------------------------------------------------------------------------
void ChipcoreProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("presetIndex", currentPreset.load(), nullptr);
    state.setProperty ("presetName", getCurrentPresetName(), nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void ChipcoreProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType())) return;
    auto tree = juce::ValueTree::fromXml (*xml);
    apvts.replaceState (tree);
    const int idx = (int) tree.getProperty ("presetIndex", 0);
    {
        const juce::ScopedLock sl (nameLock);
        userPresetName = tree.getProperty ("presetName", "USER").toString();
    }
    currentPreset = idx;
}

juce::AudioProcessorEditor* ChipcoreProcessor::createEditor() { return new ChipcoreEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new ChipcoreProcessor(); }
