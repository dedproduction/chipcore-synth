// Developer test tool (not shipped):
//  1) loads the built CHIPCORE.vst3 like a DAW would, checks programs + renders audio
//  2) renders the editor into a PNG (compiled from sources) for a visual check
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include "../../Source/PluginProcessor.h"
#include "../../Source/PluginEditor.h"

static int testHostedPlugin (const juce::String& vst3Path)
{
    juce::AudioPluginFormatManager fm;
    fm.addFormat (new juce::VST3PluginFormat());
    juce::OwnedArray<juce::PluginDescription> types;
    juce::VST3PluginFormat vst3;
    vst3.findAllTypesForFile (types, vst3Path);
    if (types.isEmpty()) { std::printf ("FAIL: no plugin found in %s\n", vst3Path.toRawUTF8()); return 1; }
    auto& d = *types[0];
    std::printf ("found: %s by %s, instrument=%d, ins=%d outs=%d\n", d.name.toRawUTF8(), d.manufacturerName.toRawUTF8(),
                 (int) d.isInstrument, d.numInputChannels, d.numOutputChannels);

    juce::String err;
    auto inst = fm.createPluginInstance (d, 48000.0, 512, err);
    if (! inst) { std::printf ("FAIL: %s\n", err.toRawUTF8()); return 1; }
    inst->setPlayConfigDetails (0, 2, 48000.0, 512);
    inst->prepareToPlay (48000.0, 512);
    std::printf ("programs: %d, params: %d\n", inst->getNumPrograms(), (int) inst->getParameters().size());

    int bad = 0;
    for (int p : { 0, 5, 22, 38, 52, 80, 93, 102 })
    {
        if (p >= inst->getNumPrograms()) continue;
        inst->setCurrentProgram (p);
        juce::AudioBuffer<float> buf (2, 512);
        float peak = 0.0f;
        for (int b = 0; b < 120; ++b)
        {
            juce::MidiBuffer midi;
            if (b == 0)  { midi.addEvent (juce::MidiMessage::noteOn (1, 60, (juce::uint8) 110), 10); midi.addEvent (juce::MidiMessage::noteOn (1, 64, (juce::uint8) 110), 10); }
            if (b == 60) { midi.addEvent (juce::MidiMessage::noteOff (1, 60), 0); midi.addEvent (juce::MidiMessage::noteOff (1, 64), 0); }
            buf.clear();
            inst->processBlock (buf, midi);
            peak = juce::jmax (peak, buf.getMagnitude (0, 512));
        }
        std::printf ("  program %3d %-26s peak %.3f%s\n", p, inst->getProgramName (p).toRawUTF8(), peak, peak < 0.01f ? "  SILENT!" : "");
        bad += peak < 0.01f || ! std::isfinite (peak);
    }

    // state round trip
    juce::MemoryBlock mb;
    inst->getStateInformation (mb);
    inst->setCurrentProgram (3);
    inst->setStateInformation (mb.getData(), (int) mb.getSize());
    std::printf ("state size %d bytes, after restore program=%d\n", (int) mb.getSize(), inst->getCurrentProgram());
    inst->releaseResources();
    return bad;
}

static int snapshotEditor (const juce::File& outPng)
{
    ChipcoreProcessor proc;
    proc.prepareToPlay (48000.0, 512);
    proc.loadPreset (5);
    // feed some audio so the scope shows a wave
    juce::AudioBuffer<float> buf (2, 512);
    for (int b = 0; b < 20; ++b)
    {
        juce::MidiBuffer midi;
        if (b == 0) midi.addEvent (juce::MidiMessage::noteOn (1, 64, (juce::uint8) 110), 0);
        proc.processBlock (buf, midi);
    }
    std::unique_ptr<juce::AudioProcessorEditor> ed (proc.createEditor());
    ed->setSize (ChipcoreEditor::designW, ChipcoreEditor::designH);
    auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f);
    juce::PNGImageFormat png;
    outPng.deleteFile();
    juce::FileOutputStream os (outPng);
    if (! os.openedOk() || ! png.writeImageToStream (img, os)) { std::printf ("FAIL: could not write png\n"); return 1; }
    std::printf ("editor snapshot -> %s (%dx%d)\n", outPng.getFullPathName().toRawUTF8(), img.getWidth(), img.getHeight());
    return 0;
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    int fails = 0;
    if (argc > 1) fails += testHostedPlugin (argv[1]);
    if (argc > 2) fails += snapshotEditor (juce::File (argv[2]));
    std::printf (fails ? "HOST TEST: %d FAILURES\n" : "HOST TEST: OK\n", fails);
    return fails;
}
