// Headless engine test: renders every factory preset, checks for NaN / silence / clipping,
// and optionally writes a WAV preview per preset.
#include "../Source/dsp/Engine.h"
#include "../Source/Presets.h"
#include <cstdio>
#include <string>
#include <vector>
#include <cstring>

static void writeWav (const std::string& path, const std::vector<float>& l, const std::vector<float>& r, int sr)
{
    FILE* f = std::fopen (path.c_str(), "wb");
    if (! f) return;
    const uint32_t n = (uint32_t) l.size();
    const uint32_t dataBytes = n * 2 * 2;
    auto w32 = [f] (uint32_t x) { std::fwrite (&x, 4, 1, f); };
    auto w16 = [f] (uint16_t x) { std::fwrite (&x, 2, 1, f); };
    std::fwrite ("RIFF", 1, 4, f); w32 (36 + dataBytes); std::fwrite ("WAVEfmt ", 1, 8, f);
    w32 (16); w16 (1); w16 (2); w32 ((uint32_t) sr); w32 ((uint32_t) sr * 4); w16 (4); w16 (16);
    std::fwrite ("data", 1, 4, f); w32 (dataBytes);
    for (uint32_t i = 0; i < n; ++i)
    {
        int16_t a = (int16_t) (chip::clampf (l[i], -1.0f, 1.0f) * 32767.0f);
        int16_t b = (int16_t) (chip::clampf (r[i], -1.0f, 1.0f) * 32767.0f);
        std::fwrite (&a, 2, 1, f); std::fwrite (&b, 2, 1, f);
    }
    std::fclose (f);
}

int main (int argc, char** argv)
{
    const char* wavDir = argc > 1 && std::strcmp (argv[1], "-") != 0 ? argv[1] : nullptr;
    FILE* levels = argc > 2 ? std::fopen (argv[2], "w") : nullptr;
    const int sr = 48000;
    const auto& table = chip::paramTable();
    int failures = 0;

    for (size_t p = 0; p < chip::factoryPresets().size(); ++p)
    {
        const auto& preset = chip::factoryPresets()[p];
        std::vector<float> v (chip::NUM_PARAMS);
        for (int i = 0; i < chip::NUM_PARAMS; ++i) v[(size_t) i] = table[(size_t) i].def;
        for (auto& kv : preset.values)
        {
            const int idx = chip::paramIndex (kv.first);
            if (idx < 0) { std::printf ("!! unknown param '%s' in preset '%s'\n", kv.first, preset.name); ++failures; continue; }
            const auto& info = table[(size_t) idx];
            if (kv.second < info.min - 1e-4f || kv.second > info.max + 1e-4f)
            { std::printf ("!! out-of-range %s=%g in '%s'\n", kv.first, kv.second, preset.name); ++failures; }
            v[(size_t) idx] = chip::clampf (kv.second, info.min, info.max);
        }

        chip::Engine eng;
        eng.prepare (sr);
        eng.setTempo (150.0);

        const int block = 256;
        const int total = sr * 3;
        std::vector<float> L ((size_t) total), R ((size_t) total);
        const std::string cat = preset.category;
        const bool drum = cat == "DRUM" || cat == "FX";
        // simple riff: a chord for 1.2 s (single note for drums / mono / arps)
        std::vector<int> notes;
        const bool monoish = v[chip::voice_mode] > 0.5f || v[chip::arp_on] > 0.5f || drum;
        if (monoish) notes = { cat == "BASS" ? 40 : (drum ? 60 : 64) };
        else notes = { cat == "BASS" ? 40 : 60, cat == "BASS" ? 47 : 64, cat == "BASS" ? 52 : 67 };

        double peak = 0.0, sumSq = 0.0;
        bool nan = false;
        for (int pos = 0; pos < total; pos += block)
        {
            const int n = std::min (block, total - pos);
            eng.beginBlock (v.data());
            if (pos == 0) for (int nt : notes) eng.noteOn (nt, 0.9f);
            if (pos <= sr * 6 / 10 && pos + n > sr * 6 / 10 && monoish && ! drum && v[chip::arp_on] < 0.5f)
            { eng.noteOn (notes[0] + 3, 0.9f); eng.noteOff (notes[0]); notes[0] += 3; }
            if (pos <= sr * 12 / 10 && pos + n > sr * 12 / 10) for (int nt : notes) eng.noteOff (nt);
            eng.render (v.data(), L.data() + pos, R.data() + pos, n);
        }
        for (int i = 0; i < total; ++i)
        {
            const float a = std::max (std::fabs (L[(size_t) i]), std::fabs (R[(size_t) i]));
            if (! std::isfinite (a)) nan = true;
            peak = std::max (peak, (double) a);
            sumSq += (double) L[(size_t) i] * L[(size_t) i];
        }
        const double rms = std::sqrt (sumSq / total);
        const double rmsDb = 20.0 * std::log10 (rms + 1e-9);
        const bool silent = peak < 0.01;
        const bool hot = peak > 0.995;
        std::printf ("%3zu %-6s %-28s peak %.3f  rms %6.1f dB %s%s%s\n", p + 1, preset.category, preset.name, peak, rmsDb,
                     nan ? " NAN!" : "", silent ? " SILENT!" : "", hot ? " (clip-limit)" : "");
        if (nan || silent) ++failures;
        if (levels) std::fprintf (levels, "%s\t%.6f\t%.6f\n", preset.name, peak, rms);

        if (wavDir)
        {
            char fn[512];
            std::snprintf (fn, sizeof (fn), "%s/%03zu_%s.wav", wavDir, p + 1, preset.name);
            for (char* ch = fn + std::strlen (wavDir) + 1; *ch; ++ch) if (*ch == ' ' || *ch == '/') *ch = '_';
            writeWav (fn, L, R, sr);
        }
    }
    if (levels) std::fclose (levels);
    std::printf ("\n%zu presets, %d problems\n", chip::factoryPresets().size(), failures);
    return failures == 0 ? 0 : 1;
}
