// CHIPCORE - parameter table (pure C++, shared by plugin, engine and tests)
#pragma once
#include <vector>
#include <string>

namespace chip
{
enum class Kind { Float, Int, Bool, Choice };

// ---- choice lists ---------------------------------------------------------
inline const std::vector<const char*> kWaves      { "PULSE", "TRI NES", "SAW", "SUPERSAW", "SINE", "FM", "GB WAVE", "NOISE" };
inline const std::vector<const char*> kFmRatios   { "FM x0.5", "FM x1", "FM x2", "FM x3", "FM x4", "FM x5", "FM x7", "FM x1.41" };
inline const std::vector<const char*> kSubWaves   { "SQUARE", "TRI", "PULSE25" };
inline const std::vector<const char*> kSubOcts    { "-1 OCT", "-2 OCT" };
inline const std::vector<const char*> kVoiceModes { "POLY", "MONO", "LEGATO" };
inline const std::vector<const char*> kFilters    { "OFF", "LP 12", "LP 24", "HP 12", "BP 12" };
inline const std::vector<const char*> kLfoWaves   { "SINE", "TRI", "SQUARE", "SAW", "S&H" };
inline const std::vector<const char*> kLfoTargets { "PITCH", "SHAPE", "CUTOFF", "VOLUME", "PAN" };
inline const std::vector<const char*> kArpModes   { "UP", "DOWN", "UP-DOWN", "RANDOM", "PLAYED" };
inline const std::vector<const char*> kArpRates   { "1/4", "1/8", "1/8T", "1/16", "1/16T", "1/32", "1/32T", "1/64" };
inline const std::vector<const char*> kChords     { "OFF", "MAJOR", "MINOR", "POWER", "OCTAVE", "MAJ7", "MIN7", "SUS4", "DIM", "MAJ+OCT", "MIN+OCT", "5TH STACK" };
inline const std::vector<const char*> kDistTypes  { "SOFT", "HARD", "FOLD", "FUZZ" };
inline const std::vector<const char*> kDelayTimes { "1/32", "1/16", "1/16D", "1/8", "1/8D", "1/4", "1/4D", "1/2" };

// X(id, display name, kind, min, max, default, skewCentre (0 = linear), choices*, unit)
#define CHIP_PARAMS(X) \
    X(oscA_wave,   "Osc A Wave",    Choice, 0, 7, 0, 0, &kWaves, "") \
    X(oscA_oct,    "Osc A Octave",  Int, -3, 3, 0, 0, nullptr, "oct") \
    X(oscA_semi,   "Osc A Semi",    Int, -12, 12, 0, 0, nullptr, "st") \
    X(oscA_fine,   "Osc A Fine",    Float, -100, 100, 0, 0, nullptr, "ct") \
    X(oscA_level,  "Osc A Level",   Float, 0, 1, 0.8f, 0, nullptr, "%") \
    X(oscA_shape,  "Osc A Shape",   Float, 0, 1, 0.444f, 0, nullptr, "%") \
    X(oscA_uni,    "Osc A Unison",  Int, 1, 7, 1, 0, nullptr, "v") \
    X(oscA_detune, "Osc A Detune",  Float, 0, 1, 0.25f, 0, nullptr, "%") \
    X(oscA_ratio,  "Osc A FM Ratio",Choice, 0, 7, 2, 0, &kFmRatios, "") \
    X(oscB_wave,   "Osc B Wave",    Choice, 0, 7, 1, 0, &kWaves, "") \
    X(oscB_oct,    "Osc B Octave",  Int, -3, 3, 0, 0, nullptr, "oct") \
    X(oscB_semi,   "Osc B Semi",    Int, -12, 12, 0, 0, nullptr, "st") \
    X(oscB_fine,   "Osc B Fine",    Float, -100, 100, 0, 0, nullptr, "ct") \
    X(oscB_level,  "Osc B Level",   Float, 0, 1, 0.0f, 0, nullptr, "%") \
    X(oscB_shape,  "Osc B Shape",   Float, 0, 1, 0.0f, 0, nullptr, "%") \
    X(oscB_uni,    "Osc B Unison",  Int, 1, 7, 1, 0, nullptr, "v") \
    X(oscB_detune, "Osc B Detune",  Float, 0, 1, 0.25f, 0, nullptr, "%") \
    X(oscB_ratio,  "Osc B FM Ratio",Choice, 0, 7, 2, 0, &kFmRatios, "") \
    X(sub_level,   "Sub Level",     Float, 0, 1, 0, 0, nullptr, "%") \
    X(sub_wave,    "Sub Wave",      Choice, 0, 2, 0, 0, &kSubWaves, "") \
    X(sub_oct,     "Sub Octave",    Choice, 0, 1, 0, 0, &kSubOcts, "") \
    X(noise_level, "Noise Level",   Float, 0, 1, 0, 0, nullptr, "%") \
    X(noise_tone,  "Noise Tone",    Float, 0, 1, 0.7f, 0, nullptr, "%") \
    X(noise_short, "Noise Metal",   Bool, 0, 1, 0, 0, nullptr, "") \
    X(noise_decay, "Noise Decay",   Float, 0, 2000, 0, 150, nullptr, "ms") \
    X(penv_amt,    "Pitch Env Amt", Float, -48, 48, 0, 0, nullptr, "st") \
    X(penv_time,   "Pitch Env Time",Float, 1, 2000, 80, 120, nullptr, "ms") \
    X(vib_depth,   "Vibrato Depth", Float, 0, 1, 0, 0, nullptr, "%") \
    X(vib_rate,    "Vibrato Rate",  Float, 0.5f, 15, 6, 0, nullptr, "Hz") \
    X(vib_delay,   "Vibrato Delay", Float, 0, 2000, 200, 400, nullptr, "ms") \
    X(glide,       "Glide",         Float, 0, 2000, 0, 150, nullptr, "ms") \
    X(voice_mode,  "Voice Mode",    Choice, 0, 2, 0, 0, &kVoiceModes, "") \
    X(bend_range,  "Bend Range",    Int, 1, 24, 2, 0, nullptr, "st") \
    X(vel_sens,    "Velocity",      Float, 0, 1, 0.5f, 0, nullptr, "%") \
    X(flt_type,    "Filter Type",   Choice, 0, 4, 0, 0, &kFilters, "") \
    X(flt_cutoff,  "Cutoff",        Float, 20, 20000, 20000, 1000, nullptr, "Hz") \
    X(flt_reso,    "Resonance",     Float, 0, 1, 0.1f, 0, nullptr, "%") \
    X(flt_env,     "Filter Env",    Float, -1, 1, 0, 0, nullptr, "%") \
    X(flt_key,     "Key Track",     Float, 0, 1, 0, 0, nullptr, "%") \
    X(fenv_a,      "F.Env Attack",  Float, 0, 5000, 1, 200, nullptr, "ms") \
    X(fenv_d,      "F.Env Decay",   Float, 1, 5000, 300, 400, nullptr, "ms") \
    X(fenv_s,      "F.Env Sustain", Float, 0, 1, 0, 0, nullptr, "%") \
    X(fenv_r,      "F.Env Release", Float, 1, 5000, 200, 400, nullptr, "ms") \
    X(amp_a,       "Attack",        Float, 0, 5000, 1, 200, nullptr, "ms") \
    X(amp_d,       "Decay",         Float, 1, 5000, 300, 400, nullptr, "ms") \
    X(amp_s,       "Sustain",       Float, 0, 1, 1, 0, nullptr, "%") \
    X(amp_r,       "Release",       Float, 1, 8000, 120, 400, nullptr, "ms") \
    X(amp_quant,   "4-Bit Volume",  Bool, 0, 1, 0, 0, nullptr, "") \
    X(lfo_wave,    "LFO Wave",      Choice, 0, 4, 0, 0, &kLfoWaves, "") \
    X(lfo_rate,    "LFO Rate",      Float, 0.05f, 30, 4, 3, nullptr, "Hz") \
    X(lfo_target,  "LFO Target",    Choice, 0, 4, 2, 0, &kLfoTargets, "") \
    X(lfo_amt,     "LFO Amount",    Float, 0, 1, 0, 0, nullptr, "%") \
    X(arp_on,      "Arp On",        Bool, 0, 1, 0, 0, nullptr, "") \
    X(arp_mode,    "Arp Mode",      Choice, 0, 4, 0, 0, &kArpModes, "") \
    X(arp_rate,    "Arp Rate",      Choice, 0, 7, 5, 0, &kArpRates, "") \
    X(arp_oct,     "Arp Octaves",   Int, 1, 4, 1, 0, nullptr, "oct") \
    X(arp_gate,    "Arp Gate",      Float, 0.05f, 1, 0.8f, 0, nullptr, "%") \
    X(chord,       "Chord",         Choice, 0, 11, 0, 0, &kChords, "") \
    X(dist_type,   "Drive Type",    Choice, 0, 3, 0, 0, &kDistTypes, "") \
    X(dist_drive,  "Drive",         Float, 0, 1, 0, 0, nullptr, "%") \
    X(dist_tone,   "Drive Tone",    Float, 0, 1, 1, 0, nullptr, "%") \
    X(dist_mix,    "Drive Mix",     Float, 0, 1, 1, 0, nullptr, "%") \
    X(crush_bits,  "Crush Bits",    Float, 2, 16, 16, 0, nullptr, "bit") \
    X(crush_rate,  "Crush Rate",    Float, 0, 1, 0, 0, nullptr, "%") \
    X(chorus_mix,  "Chorus Mix",    Float, 0, 1, 0, 0, nullptr, "%") \
    X(chorus_rate, "Chorus Rate",   Float, 0.05f, 5, 0.8f, 0, nullptr, "Hz") \
    X(dly_mix,     "Delay Mix",     Float, 0, 1, 0, 0, nullptr, "%") \
    X(dly_time,    "Delay Time",    Choice, 0, 7, 4, 0, &kDelayTimes, "") \
    X(dly_fb,      "Delay Feedback",Float, 0, 0.95f, 0.35f, 0, nullptr, "%") \
    X(dly_pp,      "Ping-Pong",     Bool, 0, 1, 1, 0, nullptr, "") \
    X(rev_mix,     "Reverb Mix",    Float, 0, 1, 0, 0, nullptr, "%") \
    X(rev_size,    "Reverb Size",   Float, 0, 1, 0.5f, 0, nullptr, "%") \
    X(master_gain, "Master",        Float, -24, 6, -6, 0, nullptr, "dB")

enum P : int
{
#define CHIP_ENUM(id, name, kind, mn, mx, df, sk, ch, unit) id,
    CHIP_PARAMS(CHIP_ENUM)
#undef CHIP_ENUM
    NUM_PARAMS
};

struct ParamInfo
{
    const char* id;
    const char* name;
    Kind kind;
    float min, max, def, skewCentre;
    const std::vector<const char*>* choices;
    const char* unit;
};

inline const std::vector<ParamInfo>& paramTable()
{
    static const std::vector<ParamInfo> t {
#define CHIP_INFO(id, name, kind, mn, mx, df, sk, ch, unit) { #id, name, Kind::kind, (float) (mn), (float) (mx), (float) (df), (float) (sk), ch, unit },
        CHIP_PARAMS(CHIP_INFO)
#undef CHIP_INFO
    };
    return t;
}

inline int paramIndex (const std::string& id)
{
    const auto& t = paramTable();
    for (size_t i = 0; i < t.size(); ++i)
        if (id == t[i].id) return (int) i;
    return -1;
}
} // namespace chip
