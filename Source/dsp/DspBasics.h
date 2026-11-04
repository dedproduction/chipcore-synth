// CHIPCORE - DSP building blocks (pure C++)
#pragma once
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <array>

namespace chip
{
constexpr float kPi    = 3.14159265358979f;
constexpr float kTwoPi = 6.28318530717959f;

inline float clampf (float x, float lo, float hi) { return x < lo ? lo : (x > hi ? hi : x); }
inline float lerpf  (float a, float b, float t)   { return a + (b - a) * t; }
inline float midiToHz (float note)                 { return 440.0f * std::exp2 ((note - 69.0f) * (1.0f / 12.0f)); }
inline float timeCoef (float seconds, float sr)    { return seconds <= 0.0f ? 0.0f : std::exp (-4.6f / (seconds * sr)); }

inline float polyBlep (float t, float dt)
{
    if (dt <= 0.0f) return 0.0f;
    if (t < dt)        { t /= dt;            return t + t - t * t - 1.0f; }
    if (t > 1.0f - dt) { t = (t - 1.0f) / dt; return t * t + t + t + 1.0f; }
    return 0.0f;
}

struct Rng
{
    uint32_t s = 0x9E3779B9u;
    uint32_t nextU() { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return s; }
    float bipolar()  { return (float) (nextU() & 0xFFFFFF) / 8388608.0f - 1.0f; }
    float unipolar() { return (float) (nextU() & 0xFFFFFF) / 16777216.0f; }
};

// ---------------------------------------------------------------------------
// Console wavetables
// ---------------------------------------------------------------------------
struct Tables
{
    // NES 2A03 triangle: 32 steps, 4-bit
    static float nesTri (int i)
    {
        static const std::array<float, 32> t = [] {
            std::array<float, 32> a {};
            for (int k = 0; k < 32; ++k)
            {
                int v = k < 16 ? 15 - k : k - 16;
                a[(size_t) k] = (float) v / 7.5f - 1.0f;
            }
            return a;
        }();
        return t[(size_t) (i & 31)];
    }

    // Game Boy style 32 x 4-bit wave RAM shapes
    static float gb (int table, int i)
    {
        static const uint8_t w[8][32] = {
            { 0,2,4,6,8,10,12,14,15,15,14,12,10,8,6,4, 2,0,0,1,3,5,7,9,11,13,15,13,11,7,3,1 },  // bell / "blip"
            { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 },  // double saw
            { 8,11,13,14,15,15,14,13,11,8,5,3,2,1,1,2, 3,5,8,11,13,15,15,13,11,8,4,2,0,0,2,4 }, // organ
            { 15,15,15,15,15,15,15,15,15,15,15,15,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },  // narrow pulse
            { 8,9,11,12,13,14,15,15,15,15,14,13,12,11,9,8, 7,6,4,3,2,1,0,0,0,0,1,2,3,4,6,7 },  // 4-bit sine
            { 15,13,11,9,15,13,11,9,7,5,3,1,7,5,3,1, 15,12,9,6,3,0,15,12,9,6,3,0,8,6,4,2 },    // buzz
            { 0,4,8,12,15,12,8,4,0,0,15,15,0,0,15,15, 8,10,12,14,15,14,12,10,8,6,4,2,0,2,4,6 },// vocal-ish
            { 15,0,15,0,15,15,0,0,15,15,15,0,0,0,15,0, 15,15,0,15,0,0,15,0,15,0,0,0,15,15,0,15 } // harsh
        };
        return (float) w[table & 7][i & 31] / 7.5f - 1.0f;
    }
};

// ---------------------------------------------------------------------------
// Oscillator
// ---------------------------------------------------------------------------
enum Wave { W_PULSE = 0, W_TRI, W_SAW, W_SUPERSAW, W_SINE, W_FM, W_GB, W_NOISE };

inline float fmRatioValue (int idx)
{
    static const float r[8] = { 0.5f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 7.0f, 1.41421356f };
    return r[std::clamp (idx, 0, 7)];
}

struct OscState
{
    float phase = 0.0f, modPhase = 0.0f, noisePhase = 0.0f, noiseOut = 1.0f;
    uint16_t lfsr = 1;

    void reset (float startPhase)
    {
        phase = startPhase; modPhase = 0.0f; noisePhase = 0.0f; noiseOut = 1.0f; lfsr = 1;
    }

    // NES style 15-bit LFSR noise, clocked at clockHz
    float noise (float clockHz, float sr, bool shortMode)
    {
        noisePhase += clockHz / sr;
        int steps = 0;
        while (noisePhase >= 1.0f && steps < 32)
        {
            noisePhase -= 1.0f;
            const uint16_t fb = (uint16_t) ((lfsr & 1u) ^ ((lfsr >> (shortMode ? 6 : 1)) & 1u));
            lfsr = (uint16_t) ((lfsr >> 1) | (fb << 14));
            noiseOut = (lfsr & 1u) ? -1.0f : 1.0f;
            ++steps;
        }
        if (noisePhase >= 1.0f) noisePhase -= std::floor (noisePhase);
        return noiseOut;
    }

    // Renders one sample and advances the phase. dt = freq / sr
    float render (int wave, float freq, float dt, float sr, float shape, float fmRatio)
    {
        float v = 0.0f;
        const float t = phase;
        switch (wave)
        {
            case W_PULSE:
            {
                const float duty = 0.05f + 0.45f * shape;
                v = t < duty ? 1.0f : -1.0f;
                v += polyBlep (t, dt);
                float t2 = t - duty; if (t2 < 0.0f) t2 += 1.0f;
                v -= polyBlep (t2, dt);
                v -= (2.0f * duty - 1.0f); // remove DC
                break;
            }
            case W_TRI:
            {
                const float stepped = Tables::nesTri ((int) (t * 32.0f));
                const float smooth  = 4.0f * std::fabs (t - 0.5f) - 1.0f;
                v = lerpf (stepped, smooth, shape);
                break;
            }
            case W_SAW:
            case W_SUPERSAW:
            {
                v = 2.0f * t - 1.0f - polyBlep (t, dt);
                if (wave == W_SAW && shape > 0.01f)
                {
                    const float steps = lerpf (64.0f, 3.0f, shape);
                    v = std::round (v * steps) / steps;
                }
                break;
            }
            case W_SINE:
            {
                v = std::sin (kTwoPi * t);
                if (shape > 0.001f)
                    v = std::sin (v * (1.0f + shape * 5.0f) * (kPi * 0.5f));
                break;
            }
            case W_FM:
            {
                const float mod = std::sin (kTwoPi * modPhase);
                v = std::sin (kTwoPi * t + shape * 6.0f * mod);
                modPhase += dt * fmRatio;
                if (modPhase >= 1.0f) modPhase -= std::floor (modPhase);
                break;
            }
            case W_GB:
            {
                const float pos = shape * 7.0f;
                const int t0 = (int) pos;
                const int t1 = std::min (t0 + 1, 7);
                const int idx = (int) (t * 32.0f);
                v = lerpf (Tables::gb (t0, idx), Tables::gb (t1, idx), pos - (float) t0);
                break;
            }
            case W_NOISE:
            default:
                v = noise (freq * 16.0f, sr, shape >= 0.5f);
                break;
        }

        phase += dt;
        if (phase >= 1.0f) phase -= std::floor (phase);
        return v;
    }
};

// ---------------------------------------------------------------------------
// ADSR envelope (linear attack, exponential decay/release)
// ---------------------------------------------------------------------------
struct Envelope
{
    enum Stage { Idle, Attack, Decay, Sustain, Release };
    Stage stage = Idle;
    float value = 0.0f;
    float attackInc = 1.0f, decayCoef = 0.0f, releaseCoef = 0.0f, sustain = 1.0f;

    void set (float aMs, float dMs, float s, float rMs, float sr)
    {
        attackInc   = 1.0f / std::max (1.0f, aMs * 0.001f * sr);
        decayCoef   = timeCoef (std::max (0.001f, dMs * 0.001f), sr);
        releaseCoef = timeCoef (std::max (0.001f, rMs * 0.001f), sr);
        sustain     = s;
    }
    void noteOn()   { stage = Attack; }
    void noteOff()  { if (stage != Idle) stage = Release; }
    void kill()     { stage = Idle; value = 0.0f; }
    bool active() const { return stage != Idle; }

    float next()
    {
        switch (stage)
        {
            case Attack:
                value += attackInc;
                if (value >= 1.0f) { value = 1.0f; stage = Decay; }
                break;
            case Decay:
                value = sustain + (value - sustain) * decayCoef;
                if (std::fabs (value - sustain) < 1.0e-4f) { value = sustain; stage = Sustain; }
                break;
            case Sustain:
                value = sustain;
                if (sustain <= 1.0e-5f) { stage = Idle; value = 0.0f; }
                break;
            case Release:
                value *= releaseCoef;
                if (value < 1.0e-4f) { value = 0.0f; stage = Idle; }
                break;
            case Idle:
            default: value = 0.0f; break;
        }
        return value;
    }
};

// ---------------------------------------------------------------------------
// TPT state variable filter (Cytomic / Simper)
// ---------------------------------------------------------------------------
struct SVF
{
    float ic1 = 0.0f, ic2 = 0.0f;
    void reset() { ic1 = ic2 = 0.0f; }

    // mode: 0 LP, 1 HP, 2 BP
    float process (float x, float g, float k, int mode)
    {
        const float a1 = 1.0f / (1.0f + g * (g + k));
        const float a2 = g * a1;
        const float a3 = g * a2;
        const float v3 = x - ic2;
        const float v1 = a1 * ic1 + a2 * v3;
        const float v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2.0f * v1 - ic1;
        ic2 = 2.0f * v2 - ic2;
        if (! std::isfinite (ic1) || ! std::isfinite (ic2)) { ic1 = ic2 = 0.0f; return 0.0f; }
        if (mode == 0) return v2;
        if (mode == 1) return x - k * v1 - v2;
        return v1;
    }
};

struct OnePole
{
    float z = 0.0f;
    float process (float x, float coef) { z = x + (z - x) * coef; return z; }
};
} // namespace chip
