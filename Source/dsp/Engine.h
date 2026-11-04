// CHIPCORE - synth engine: voices, arpeggiator, chord memory, FX chain (pure C++)
#pragma once
#include "DspBasics.h"
#include "Effects.h"
#include "../ParamTable.h"
#include <vector>

namespace chip
{
inline int pi (const float* v, int idx) { return (int) std::lround (v[idx]); }

inline const std::vector<int>& chordIntervals (int chordIdx)
{
    static const std::vector<std::vector<int>> c {
        { 0 },              // off
        { 0, 4, 7 },        // major
        { 0, 3, 7 },        // minor
        { 0, 7 },           // power
        { 0, 12 },          // octave
        { 0, 4, 7, 11 },    // maj7
        { 0, 3, 7, 10 },    // min7
        { 0, 5, 7 },        // sus4
        { 0, 3, 6 },        // dim
        { 0, 4, 7, 12 },    // maj + oct
        { 0, 3, 7, 12 },    // min + oct
        { 0, 7, 14 }        // 5th stack
    };
    return c[(size_t) std::clamp (chordIdx, 0, (int) c.size() - 1)];
}

inline double arpBeats (int idx)
{
    static const double b[8] = { 1.0, 0.5, 1.0 / 3.0, 0.25, 1.0 / 6.0, 0.125, 1.0 / 12.0, 0.0625 };
    return b[std::clamp (idx, 0, 7)];
}

inline double delayBeats (int idx)
{
    static const double b[8] = { 0.125, 0.25, 0.375, 0.5, 0.75, 1.0, 1.5, 2.0 };
    return b[std::clamp (idx, 0, 7)];
}

// ---------------------------------------------------------------------------
// Per-block cooked parameters
// ---------------------------------------------------------------------------
struct Cooked
{
    float sr = 44100.0f;
    // oscillators
    int   wave[2] {}, uni[2] { 1, 1 };
    float offset[2] {}, shape[2] {}, fmRatio[2] { 1, 1 };
    float ratio[2][7] {}, gL[2][7] {}, gR[2][7] {};
    bool  oscOn[2] {};
    // sub / noise
    float subLevel = 0; int subWave = 0; int subOct = 0;
    float noiseLevel = 0, noiseTone = 0.7f; bool noiseShort = false; float noiseCoef = 0; bool noiseDecay = false;
    // pitch
    float penvAmt = 0, penvCoef = 0, glideCoef = 0;
    float vibDepth = 0, vibRate = 6, vibDelay = 0.2f;
    // filter
    int   fltType = 0; float cutoff = 20000, k = 2, fltEnv = 0, fltKey = 0;
    // amp
    bool  ampQuant = false; float velSens = 0.5f;
    // per-sample global modulation
    float bendSemis = 0, lfoPitch = 0, lfoShape = 0, lfoCut = 0, tremolo = 1, modWheel = 0;
};

// ---------------------------------------------------------------------------
class Voice
{
public:
    int note = -1;
    float velocity = 1.0f;
    bool gate = false;
    uint64_t age = 0;
    float pitch = 60.0f, target = 60.0f;

    bool active() const { return amp.active(); }

    void start (int n, float vel, float fromPitch, bool retrigger, const Cooked& c, Rng& rng, uint64_t stamp)
    {
        const bool wasActive = active();
        note = n; velocity = vel; gate = true; age = stamp;
        target = (float) n;
        pitch = fromPitch;
        if (retrigger || ! wasActive)
        {
            for (int o = 0; o < 2; ++o)
                for (int i = 0; i < 7; ++i)
                    osc[o][i].reset (c.uni[o] > 1 ? rng.unipolar() : 0.0f);
            sub.reset (0.0f);
            noise.reset (0.0f);
            penv = 1.0f;
            noiseEnv = 1.0f;
            vibTime = 0.0f;
            vibPhase = 0.0f;
            if (! wasActive)
                for (auto& st : flt) for (auto& f : st) f.reset();
            amp.noteOn();
            fenv.noteOn();
        }
    }

    void legatoTo (int n, float vel)
    {
        note = n; velocity = vel; target = (float) n; gate = true;
    }

    void release() { gate = false; amp.noteOff(); fenv.noteOff(); }
    void kill()    { gate = false; amp.kill(); fenv.kill(); note = -1; }

    void setEnvelopes (const float* v, float sr)
    {
        amp.set  (v[amp_a],  v[amp_d],  v[amp_s],  v[amp_r],  sr);
        fenv.set (v[fenv_a], v[fenv_d], v[fenv_s], v[fenv_r], sr);
    }

    void render (float& outL, float& outR, const Cooked& c)
    {
        const float sr = c.sr;
        pitch = target + (pitch - target) * c.glideCoef;
        penv *= c.penvCoef;

        // vibrato
        vibTime += 1.0f / sr;
        float vib = 0.0f;
        const float depth = c.vibDepth + c.modWheel * 0.7f;
        if (depth > 0.0001f)
        {
            const float fade = c.modWheel > 0.0f ? 1.0f : clampf ((vibTime - c.vibDelay) / 0.15f, 0.0f, 1.0f);
            vibPhase += c.vibRate / sr;
            if (vibPhase >= 1.0f) vibPhase -= 1.0f;
            vib = std::sin (kTwoPi * vibPhase) * depth * fade;
        }

        const float pe = penv * c.penvAmt;
        const float basePitch = pitch + c.bendSemis + pe + vib + c.lfoPitch;

        float l = 0.0f, r = 0.0f;
        for (int o = 0; o < 2; ++o)
        {
            if (! c.oscOn[o]) continue;
            const float f = midiToHz (basePitch + c.offset[o]);
            const float shape = clampf (c.shape[o] + c.lfoShape, 0.0f, 1.0f);
            for (int i = 0; i < c.uni[o]; ++i)
            {
                const float fi = f * c.ratio[o][i];
                if (fi >= 0.48f * sr) continue;
                const float s = osc[o][i].render (c.wave[o], fi, fi / sr, sr, shape, c.fmRatio[o]);
                l += s * c.gL[o][i];
                r += s * c.gR[o][i];
            }
        }

        float mono = 0.0f;
        if (c.subLevel > 0.0f)
        {
            const float f = midiToHz (basePitch - 12.0f * (float) (1 + c.subOct));
            static const int   sw[3] = { W_PULSE, W_TRI, W_PULSE };
            static const float ss[3] = { 1.0f, 0.0f, 0.444f };
            mono += sub.render (sw[c.subWave], f, f / sr, sr, ss[c.subWave], 1.0f) * c.subLevel * 0.55f;
        }
        if (c.noiseLevel > 0.0f)
        {
            float nAmp = c.noiseLevel * 0.5f;
            if (c.noiseDecay) { noiseEnv *= c.noiseCoef; nAmp *= noiseEnv; }
            if (nAmp > 1.0e-5f)
            {
                const float clock = 200.0f * std::exp2 (c.noiseTone * 8.0f + pe / 12.0f);
                mono += noise.noise (clock, sr, c.noiseShort) * nAmp;
            }
        }
        l += mono; r += mono;

        const float fe = fenv.next();
        if (c.fltType > 0)
        {
            const float oct = fe * c.fltEnv * 7.0f + c.fltKey * ((float) note - 60.0f) / 12.0f + c.lfoCut;
            const float fc  = clampf (c.cutoff * std::exp2 (oct), 20.0f, 0.45f * sr);
            const float g   = std::tan (kPi * fc / sr);
            const int mode  = c.fltType == 3 ? 1 : (c.fltType == 4 ? 2 : 0);
            l = flt[0][0].process (l, g, c.k, mode);
            r = flt[0][1].process (r, g, c.k, mode);
            if (c.fltType == 2)
            {
                l = flt[1][0].process (l, g, c.k, 0);
                r = flt[1][1].process (r, g, c.k, 0);
            }
        }

        float e = amp.next();
        if (c.ampQuant) e = std::round (e * 15.0f) / 15.0f;
        const float vel = 1.0f - c.velSens + c.velSens * velocity * velocity;
        const float gain = e * vel * c.tremolo;
        outL += l * gain;
        outR += r * gain;

        if (! amp.active()) note = -1;
    }

private:
    OscState osc[2][7];
    OscState sub, noise;
    Envelope amp, fenv;
    SVF flt[2][2];
    float penv = 0.0f, noiseEnv = 0.0f, vibPhase = 0.0f, vibTime = 0.0f;
};

// ---------------------------------------------------------------------------
class Engine
{
public:
    static constexpr int kMaxVoices = 16;

    void prepare (double sampleRate)
    {
        sr = (float) sampleRate;
        chorus.prepare (sr);
        delay.prepare (sr);
        reverb.prepare (sr);
        reset();
    }

    void reset()
    {
        for (auto& vo : voices) vo.kill();
        held.clear(); down.clear(); monoStack.clear();
        arpNote = -1; arpIndex = 0; arpCounter = 0.0; arpDir = 1;
        chorus.clear(); delay.clear(); reverb.clear();
        crusher = Crusher(); drive = Drive();
        dcL = DcBlocker(); dcR = DcBlocker();
        sustain = false;
    }

    void setTempo (double bpm) { if (bpm > 20.0 && bpm < 999.0) tempo = bpm; }

    // ---- MIDI from the host ----------------------------------------------
    void noteOn (int n, float vel)
    {
        removeFrom (held, n);
        held.push_back ({ n, vel });
        removeFrom (down, n);
        down.push_back ({ n, vel });
        lastVel = vel;
        if (arpActive)
        {
            if (held.size() == 1) { arpCounter = 0.0; arpIndex = 0; arpDir = 1; }
            return;
        }
        for (int iv : chordIntervals (chordIdx))
            startNote (n + iv, vel);
    }

    void noteOff (int n)
    {
        removeFrom (down, n);
        if (sustain) return;
        releaseKey (n);
    }

    void sustainPedal (bool on)
    {
        if (sustain && ! on)
        {
            sustain = false;
            std::vector<Key> copy = held;
            for (auto& k : copy)
                if (! contains (down, k.note)) releaseKey (k.note);
        }
        sustain = on;
    }

    void pitchBend (float v) { bend = clampf (v, -1.0f, 1.0f); }
    void modWheel (float v)  { wheel = clampf (v, 0.0f, 1.0f); }

    void allNotesOff()
    {
        held.clear(); down.clear(); monoStack.clear();
        for (auto& vo : voices) vo.release();
        arpNote = -1;
    }

    // ---- audio -------------------------------------------------------------
    // Call once per block (before any MIDI of that block) with the parameter array.
    // The array must stay valid until the next call.
    void beginBlock (const float* v) { cook (v); }

    void render (const float* v, float* outL, float* outR, int numSamples)
    {
        if (lastParams != v) cook (v);

        for (int s = 0; s < numSamples; ++s)
        {
            tickLfo (v);
            if (arpActive) tickArp (v);

            float l = 0.0f, r = 0.0f;
            for (auto& vo : voices)
                if (vo.active()) vo.render (l, r, c);

            if (std::fabs (lfoPan) > 1.0e-6f)
            {
                l *= std::sqrt (clampf (1.0f - lfoPan, 0.0f, 2.0f));
                r *= std::sqrt (clampf (1.0f + lfoPan, 0.0f, 2.0f));
            }

            drive.process (l, r, pi (v, dist_type), v[dist_drive], v[dist_tone], v[dist_mix], sr);
            crusher.process (l, r, v[crush_bits], v[crush_rate]);
            chorus.process (l, r, v[chorus_mix], v[chorus_rate]);
            delay.process (l, r, v[dly_mix], (float) (delayBeats (pi (v, dly_time)) * 60.0 / tempo), v[dly_fb], v[dly_pp] > 0.5f);
            reverb.process (l, r, v[rev_mix], v[rev_size]);

            gainSmooth += (masterGain - gainSmooth) * 0.002f;
            l = safetyClip (dcL.process (l * gainSmooth));
            r = safetyClip (dcR.process (r * gainSmooth));
            if (! std::isfinite (l) || ! std::isfinite (r)) { l = r = 0.0f; panicReset(); }
            outL[s] = l;
            outR[s] = r;
        }
    }

    int activeVoices() const
    {
        int n = 0;
        for (auto& vo : voices) n += vo.active() ? 1 : 0;
        return n;
    }

private:
    struct Key { int note; float vel; };

    static void removeFrom (std::vector<Key>& vec, int n)
    {
        vec.erase (std::remove_if (vec.begin(), vec.end(), [n] (const Key& k) { return k.note == n; }), vec.end());
    }
    static bool contains (const std::vector<Key>& vec, int n)
    {
        for (auto& k : vec) if (k.note == n) return true;
        return false;
    }

    void releaseKey (int n)
    {
        if (! contains (held, n)) return;
        removeFrom (held, n);
        if (! arpActive)
            for (int iv : chordIntervals (chordIdx))
                stopNote (n + iv);
    }

    void panicReset()
    {
        for (auto& vo : voices) vo.kill();
        chorus.clear(); delay.clear(); reverb.clear();
    }

    // ---- voice allocation ---------------------------------------------------
    void startNote (int n, float vel)
    {
        if (n < 0 || n > 127 || lastParams == nullptr) return;
        ++stamp;
        if (voiceMode == 0)
        {
            Voice* target = nullptr;
            for (auto& vo : voices) if (vo.active() && vo.note == n) { target = &vo; break; }
            if (! target) for (auto& vo : voices) if (! vo.active()) { target = &vo; break; }
            if (! target)
            {
                // steal: prefer released voices, then the oldest
                Voice* best = nullptr;
                for (auto& vo : voices)
                {
                    if (! best) { best = &vo; continue; }
                    const bool vr = ! vo.gate, br = ! best->gate;
                    if ((vr && ! br) || (vr == br && vo.age < best->age)) best = &vo;
                }
                target = best;
            }
            const float from = glideOn && lastNote >= 0 ? (float) lastNote : (float) n;
            target->setEnvelopes (lastParams, sr);
            target->start (n, vel, from, true, c, rng, stamp);
        }
        else
        {
            monoStack.erase (std::remove (monoStack.begin(), monoStack.end(), n), monoStack.end());
            monoStack.push_back (n);
            Voice& vo = voices[0];
            vo.setEnvelopes (lastParams, sr);
            if (voiceMode == 2 && vo.active() && vo.gate)
                vo.legatoTo (n, vel);
            else
            {
                const float from = vo.active() ? vo.pitch : (glideOn && lastNote >= 0 ? (float) lastNote : (float) n);
                vo.start (n, vel, glideOn ? from : (float) n, true, c, rng, stamp);
            }
        }
        lastNote = n;
    }

    void stopNote (int n)
    {
        if (voiceMode == 0)
        {
            for (auto& vo : voices)
                if (vo.gate && vo.note == n) vo.release();
            return;
        }
        monoStack.erase (std::remove (monoStack.begin(), monoStack.end(), n), monoStack.end());
        Voice& vo = voices[0];
        if (vo.note != n || ! vo.gate) return;
        if (! monoStack.empty())
        {
            const int back = monoStack.back();
            if (voiceMode == 2) vo.legatoTo (back, vo.velocity);
            else vo.start (back, vo.velocity, glideOn ? vo.pitch : (float) back, true, c, rng, ++stamp);
            lastNote = back;
        }
        else vo.release();
    }

    // ---- arpeggiator --------------------------------------------------------
    void buildPattern (const float* v)
    {
        pattern.clear();
        const int mode = pi (v, arp_mode);
        const int octs = std::clamp (pi (v, arp_oct), 1, 4);
        const auto& iv = chordIntervals (chordIdx);

        std::vector<int> base;
        for (auto& k : held) base.push_back (k.note);
        if (mode != 4) std::sort (base.begin(), base.end());

        std::vector<int> one;
        for (int n : base) for (int i : iv) one.push_back (n + i);
        if (mode != 4)
        {
            std::sort (one.begin(), one.end());
            one.erase (std::unique (one.begin(), one.end()), one.end());
        }
        for (int o = 0; o < octs; ++o)
            for (int n : one) if (n + 12 * o <= 127) pattern.push_back (n + 12 * o);

        if (mode == 1) std::reverse (pattern.begin(), pattern.end());
        if (mode == 2 && pattern.size() > 2)
            for (int i = (int) pattern.size() - 2; i >= 1; --i) pattern.push_back (pattern[(size_t) i]);
    }

    void tickArp (const float* v)
    {
        if (held.empty())
        {
            if (arpNote >= 0) { stopNote (arpNote); arpNote = -1; }
            return;
        }
        const double stepLen = std::max (1.0, arpBeats (pi (v, arp_rate)) * 60.0 / tempo * sr);
        if (arpCounter <= 0.0)
        {
            if (arpNote >= 0) { stopNote (arpNote); arpNote = -1; }
            buildPattern (v);
            if (! pattern.empty())
            {
                int idx;
                if (pi (v, arp_mode) == 3) idx = (int) (rng.nextU() % pattern.size());
                else idx = arpIndex % (int) pattern.size();
                arpIndex = (arpIndex + 1) % std::max (1, (int) pattern.size());
                arpNote = pattern[(size_t) idx];
                startNote (arpNote, lastVel);
                arpGate = stepLen * (double) v[arp_gate];
            }
            arpCounter += stepLen;
        }
        arpCounter -= 1.0;
        if (arpNote >= 0 && v[arp_gate] < 0.999f)
        {
            arpGate -= 1.0;
            if (arpGate <= 0.0) { stopNote (arpNote); arpNote = -1; }
        }
    }

    // ---- LFO -----------------------------------------------------------------
    void tickLfo (const float* v)
    {
        lfoPhase += v[lfo_rate] / sr;
        if (lfoPhase >= 1.0f) { lfoPhase -= 1.0f; shValue = rng.bipolar(); }
        float x = 0.0f;
        switch (pi (v, lfo_wave))
        {
            case 0: x = std::sin (kTwoPi * lfoPhase); break;
            case 1: x = 1.0f - 4.0f * std::fabs (lfoPhase - 0.5f); break;
            case 2: x = lfoPhase < 0.5f ? 1.0f : -1.0f; break;
            case 3: x = 2.0f * lfoPhase - 1.0f; break;
            default: x = shValue; break;
        }
        const float amt = v[lfo_amt];
        c.lfoPitch = c.lfoShape = c.lfoCut = 0.0f; c.tremolo = 1.0f; lfoPan = 0.0f;
        if (amt <= 0.0f) return;
        switch (pi (v, lfo_target))
        {
            case 0: c.lfoPitch = x * amt * amt * 12.0f; break;
            case 1: c.lfoShape = x * amt * 0.5f; break;
            case 2: c.lfoCut   = x * amt * 4.0f; break;
            case 3: c.tremolo  = 1.0f - amt * (0.5f + 0.5f * x); break;
            default: lfoPan    = x * amt; break;
        }
    }

    // ---- per-block parameter cooking -----------------------------------------
    void cook (const float* v)
    {
        lastParams = v;
        c.sr = sr;

        const int newMode = pi (v, voice_mode);
        const bool newArp = v[arp_on] > 0.5f;
        const int newChord = pi (v, chord);
        if (newMode != voiceMode || newArp != arpActive || newChord != chordIdx)
        {
            for (auto& vo : voices) vo.release();
            monoStack.clear();
            arpNote = -1;
            if (! newArp && arpActive)
            {
                // re-trigger currently held keys as normal notes
                voiceMode = newMode; arpActive = newArp; chordIdx = newChord;
                for (auto& k : held) for (int iv : chordIntervals (chordIdx)) startNote (k.note + iv, k.vel);
            }
            voiceMode = newMode; arpActive = newArp; chordIdx = newChord;
            arpCounter = 0.0;
        }

        static const int waveIdx[2] = { oscA_wave, oscB_wave };
        for (int o = 0; o < 2; ++o)
        {
            const int b = waveIdx[o]; // block of 9 params per osc
            const int wave = pi (v, b + 0);
            const float level = v[b + 4];
            c.oscOn[o] = level > 0.0005f;
            c.wave[o] = wave;
            c.offset[o] = 12.0f * (float) pi (v, b + 1) + (float) pi (v, b + 2) + v[b + 3] * 0.01f;
            c.shape[o] = v[b + 5];
            c.fmRatio[o] = fmRatioValue (pi (v, b + 8));
            const float det = v[b + 7];
            int n = std::clamp (pi (v, b + 6), 1, 7);
            if (wave == W_SUPERSAW) n = 7;
            c.uni[o] = n;

            static const float ssOff[7] = { -0.11002313f, -0.06288439f, -0.01952356f, 0.0f, 0.01991221f, 0.06216538f, 0.10745242f };
            const float norm = 0.5f * level / std::sqrt ((float) n);
            for (int i = 0; i < n; ++i)
            {
                float pos = n > 1 ? (float) i / (float) (n - 1) * 2.0f - 1.0f : 0.0f;
                float gain = norm;
                if (wave == W_SUPERSAW)
                {
                    c.ratio[o][i] = 1.0f + ssOff[i] * (0.15f + det * 0.85f);
                    const bool centre = (i == 3);
                    const float side = c.shape[o];
                    gain = 0.5f * level * (centre ? (1.0f - 0.55f * side) : side * 0.55f);
                }
                else
                    c.ratio[o][i] = n > 1 ? std::exp2 (pos * det * 0.5f / 12.0f) : 1.0f;
                const float spread = n > 1 ? 0.8f : 0.0f;
                const float p = pos * spread;
                c.gL[o][i] = gain * std::sqrt (1.0f - p);
                c.gR[o][i] = gain * std::sqrt (1.0f + p);
            }
        }

        c.subLevel = v[sub_level]; c.subWave = std::clamp (pi (v, sub_wave), 0, 2); c.subOct = std::clamp (pi (v, sub_oct), 0, 1);
        c.noiseLevel = v[noise_level]; c.noiseTone = v[noise_tone]; c.noiseShort = v[noise_short] > 0.5f;
        c.noiseDecay = v[noise_decay] > 0.5f;
        c.noiseCoef = timeCoef (v[noise_decay] * 0.001f, sr);

        c.penvAmt = v[penv_amt];
        c.penvCoef = timeCoef (v[penv_time] * 0.001f, sr);
        glideOn = v[glide] > 0.5f;
        c.glideCoef = glideOn ? timeCoef (v[glide] * 0.001f, sr) : 0.0f;
        c.vibDepth = v[vib_depth]; c.vibRate = v[vib_rate]; c.vibDelay = v[vib_delay] * 0.001f;

        c.fltType = pi (v, flt_type);
        c.cutoff = v[flt_cutoff];
        c.k = 2.0f - 1.96f * clampf (v[flt_reso], 0.0f, 1.0f);
        c.fltEnv = v[flt_env];
        c.fltKey = v[flt_key];

        c.ampQuant = v[amp_quant] > 0.5f;
        c.velSens = v[vel_sens];
        c.bendSemis = bend * (float) pi (v, bend_range);
        c.modWheel = wheel;

        for (auto& vo : voices) if (vo.active()) vo.setEnvelopes (v, sr);

        masterGain = std::pow (10.0f, v[master_gain] / 20.0f);
        if (gainSmooth < 0.0f) gainSmooth = masterGain;
    }

    float sr = 44100.0f;
    double tempo = 120.0;
    Voice voices[kMaxVoices];
    Cooked c;
    Rng rng;
    const float* lastParams = nullptr;
    uint64_t stamp = 0;

    std::vector<Key> held, down;
    std::vector<int> monoStack, pattern;
    bool sustain = false, arpActive = false, glideOn = false;
    int voiceMode = 0, chordIdx = 0, lastNote = -1;
    float bend = 0.0f, wheel = 0.0f, lastVel = 1.0f;

    int arpNote = -1, arpIndex = 0, arpDir = 1;
    double arpCounter = 0.0, arpGate = 0.0;

    float lfoPhase = 0.0f, shValue = 0.0f, lfoPan = 0.0f;
    float masterGain = 0.5f, gainSmooth = -1.0f;

    Drive drive;
    Crusher crusher;
    Chorus chorus;
    StereoDelay delay;
    Reverb reverb;
    DcBlocker dcL, dcR;
};
} // namespace chip
