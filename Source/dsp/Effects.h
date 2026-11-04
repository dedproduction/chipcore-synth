// CHIPCORE - effects chain (pure C++)
#pragma once
#include "DspBasics.h"
#include <vector>

namespace chip
{
// ---------------------------------------------------------------------------
struct Drive
{
    OnePole toneL, toneR;

    static float shape (float x, int type)
    {
        switch (type)
        {
            case 0:  return std::tanh (x);                                       // soft
            case 1:  return clampf (x, -1.0f, 1.0f);                              // hard
            case 2:  return std::sin (x);                                        // fold
            default: return x > 0.0f ? std::tanh (x * 1.4f) : std::tanh (x * 0.6f) * 0.9f; // fuzz
        }
    }

    void process (float& l, float& r, int type, float drive, float tone, float mix, float sr)
    {
        if (drive <= 0.0005f && mix > 0.0f && tone >= 0.999f) return;
        const float gain   = 1.0f + drive * drive * 40.0f;
        const float makeup = 1.0f / (1.0f + drive * 1.5f);
        const float fc     = 400.0f * std::pow (50.0f, tone);            // 400 Hz .. 20 kHz
        const float coef   = std::exp (-kTwoPi * std::min (fc, 0.45f * sr) / sr);
        float wl = shape (l * gain, type) * makeup;
        float wr = shape (r * gain, type) * makeup;
        if (tone < 0.999f) { wl = toneL.process (wl, coef); wr = toneR.process (wr, coef); }
        l = lerpf (l, wl, mix);
        r = lerpf (r, wr, mix);
    }
};

// ---------------------------------------------------------------------------
struct Crusher
{
    float phase = 1.0f, holdL = 0.0f, holdR = 0.0f;
    void process (float& l, float& r, float bits, float rate)
    {
        if (rate > 0.001f)
        {
            const float step = 1.0f / (1.0f + rate * rate * 40.0f);
            phase += step;
            if (phase >= 1.0f) { phase -= 1.0f; holdL = l; holdR = r; }
            l = holdL; r = holdR;
        }
        if (bits < 15.99f)
        {
            const float levels = std::exp2 (bits - 1.0f);
            l = std::round (l * levels) / levels;
            r = std::round (r * levels) / levels;
        }
    }
};

// ---------------------------------------------------------------------------
struct DelayLine
{
    std::vector<float> buf;
    int w = 0;
    void init (int size) { buf.assign ((size_t) std::max (size, 4), 0.0f); w = 0; }
    void clear()         { std::fill (buf.begin(), buf.end(), 0.0f); }
    void push (float x)  { buf[(size_t) w] = x; if (++w >= (int) buf.size()) w = 0; }
    float read (float delaySamples) const
    {
        const int size = (int) buf.size();
        delaySamples = clampf (delaySamples, 1.0f, (float) (size - 2));
        float rp = (float) w - delaySamples;
        while (rp < 0.0f) rp += (float) size;
        const int i0 = (int) rp;
        const int i1 = (i0 + 1) % size;
        const float f = rp - (float) i0;
        return buf[(size_t) i0] + (buf[(size_t) i1] - buf[(size_t) i0]) * f;
    }
};

// ---------------------------------------------------------------------------
struct Chorus
{
    DelayLine dl, dr;
    float phase = 0.0f, sr = 44100.0f;
    void prepare (float sampleRate) { sr = sampleRate; dl.init ((int) (sr * 0.05f)); dr.init ((int) (sr * 0.05f)); }
    void clear() { dl.clear(); dr.clear(); }
    void process (float& l, float& r, float mix, float rate)
    {
        if (mix <= 0.0005f) { dl.push (l); dr.push (r); return; }
        phase += rate / sr; if (phase >= 1.0f) phase -= 1.0f;
        const float ml = std::sin (kTwoPi * phase);
        const float mr = std::sin (kTwoPi * phase + kPi * 0.5f);
        dl.push (l); dr.push (r);
        const float wl = dl.read ((0.012f + 0.004f * ml) * sr);
        const float wr = dr.read ((0.014f + 0.004f * mr) * sr);
        l = l * (1.0f - 0.5f * mix) + wl * 0.7f * mix;
        r = r * (1.0f - 0.5f * mix) + wr * 0.7f * mix;
    }
};

// ---------------------------------------------------------------------------
struct StereoDelay
{
    DelayLine dl, dr;
    OnePole lpL, lpR;
    float sr = 44100.0f, curTime = 0.0f;
    void prepare (float sampleRate) { sr = sampleRate; dl.init ((int) (sr * 2.6f)); dr.init ((int) (sr * 2.6f)); curTime = 0.0f; }
    void clear() { dl.clear(); dr.clear(); }

    void process (float& l, float& r, float mix, float timeSec, float fb, bool pingPong)
    {
        const float target = clampf (timeSec, 0.005f, 2.5f) * sr;
        if (curTime <= 0.0f) curTime = target;
        curTime += (target - curTime) * 0.0005f;
        if (mix <= 0.0005f) { dl.push (0.0f); dr.push (0.0f); return; }

        const float coef = std::exp (-kTwoPi * 6000.0f / sr);
        const float yl = lpL.process (dl.read (curTime), coef);
        const float yr = lpR.process (dr.read (curTime), coef);
        if (pingPong)
        {
            const float in = 0.5f * (l + r);
            dl.push (in + yr * fb);
            dr.push (yl * fb);
        }
        else
        {
            dl.push (l + yl * fb);
            dr.push (r + yr * fb);
        }
        l += yl * mix;
        r += yr * mix;
    }
};

// ---------------------------------------------------------------------------
// Freeverb style reverb
// ---------------------------------------------------------------------------
struct Reverb
{
    struct Comb
    {
        std::vector<float> b; int i = 0; float store = 0.0f;
        float process (float x, float fb, float damp)
        {
            float out = b[(size_t) i];
            store = out * (1.0f - damp) + store * damp;
            b[(size_t) i] = x + store * fb;
            if (++i >= (int) b.size()) i = 0;
            return out;
        }
    };
    struct Allpass
    {
        std::vector<float> b; int i = 0;
        float process (float x)
        {
            float bo = b[(size_t) i];
            float out = -x + bo;
            b[(size_t) i] = x + bo * 0.5f;
            if (++i >= (int) b.size()) i = 0;
            return out;
        }
    };

    Comb cl[8], cr[8];
    Allpass al[4], ar[4];

    void prepare (float sr)
    {
        const int combT[8] = { 1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617 };
        const int apT[4]   = { 556, 441, 341, 225 };
        const float s = sr / 44100.0f;
        for (int k = 0; k < 8; ++k)
        {
            cl[k].b.assign ((size_t) std::max (8, (int) (combT[k] * s)), 0.0f);
            cr[k].b.assign ((size_t) std::max (8, (int) ((combT[k] + 23) * s)), 0.0f);
            cl[k].i = cr[k].i = 0; cl[k].store = cr[k].store = 0.0f;
        }
        for (int k = 0; k < 4; ++k)
        {
            al[k].b.assign ((size_t) std::max (8, (int) (apT[k] * s)), 0.0f);
            ar[k].b.assign ((size_t) std::max (8, (int) ((apT[k] + 23) * s)), 0.0f);
            al[k].i = ar[k].i = 0;
        }
    }
    void clear()
    {
        for (auto& c : cl) { std::fill (c.b.begin(), c.b.end(), 0.0f); c.store = 0.0f; }
        for (auto& c : cr) { std::fill (c.b.begin(), c.b.end(), 0.0f); c.store = 0.0f; }
        for (auto& a : al) std::fill (a.b.begin(), a.b.end(), 0.0f);
        for (auto& a : ar) std::fill (a.b.begin(), a.b.end(), 0.0f);
    }

    void process (float& l, float& r, float mix, float size)
    {
        if (mix <= 0.0005f) return;
        const float in = (l + r) * 0.015f;
        const float fb = 0.70f + size * 0.28f;
        const float damp = 0.45f - size * 0.2f;
        float ol = 0.0f, orr = 0.0f;
        for (int k = 0; k < 8; ++k) { ol += cl[k].process (in, fb, damp); orr += cr[k].process (in, fb, damp); }
        for (int k = 0; k < 4; ++k) { ol = al[k].process (ol); orr = ar[k].process (orr); }
        l = l * (1.0f - 0.4f * mix) + ol * mix;
        r = r * (1.0f - 0.4f * mix) + orr * mix;
    }
};

// ---------------------------------------------------------------------------
struct DcBlocker
{
    float x1 = 0.0f, y1 = 0.0f;
    float process (float x) { const float y = x - x1 + 0.995f * y1; x1 = x; y1 = y; return y; }
};

inline float safetyClip (float x)
{
    const float t = 0.9f;
    const float a = std::fabs (x);
    if (a <= t) return x;
    const float over = a - t;
    const float y = t + (1.0f - t) * std::tanh (over / (1.0f - t));
    return x < 0.0f ? -y : y;
}
} // namespace chip
