#!/usr/bin/env python3
"""Generates Source/Presets.h - the CHIPCORE factory bank.

Each preset only lists the parameters that differ from the defaults in
Source/ParamTable.h. Run:  python3 tools/gen_presets.py
"""
import json
import os

# ---- enum helpers -----------------------------------------------------------
PULSE, TRI, SAW, SUPERSAW, SINE, FM, GB, NOISE = range(8)
R05, R1, R2, R3, R4, R5, R7, R141 = range(8)
F_OFF, LP12, LP24, HP12, BP12 = range(5)
L_SINE, L_TRI, L_SQ, L_SAW, L_SH = range(5)
T_PITCH, T_SHAPE, T_CUT, T_VOL, T_PAN = range(5)
UP, DOWN, UPDOWN, RANDOM, PLAYED = range(5)
A4, A8, A8T, A16, A16T, A32, A32T, A64 = range(8)
C_OFF, C_MAJ, C_MIN, C_POW, C_OCT, C_MAJ7, C_MIN7, C_SUS4, C_DIM, C_MAJO, C_MINO, C_5TH = range(12)
SOFT, HARD, FOLD, FUZZ = range(4)
D32, D16, D16D, D8, D8D, D4, D4D, D2 = range(8)
POLY, MONO, LEGATO = range(3)


def duty(d):
    """Pulse width (0.05..0.5) -> shape value."""
    return round((d - 0.05) / 0.45, 4)


def gbt(i):
    """Game Boy wavetable index (0..7) -> shape value."""
    return round(i / 7.0, 4)


P12, P25, P50 = duty(0.125), duty(0.25), 1.0

PRESETS = []


def P(name, cat, **kw):
    PRESETS.append((name, cat, kw))


# =============================================================================
# LEADS
# =============================================================================
P("LD Hero Pulse 25", "LEAD", oscA_shape=P25, voice_mode=LEGATO, glide=25,
  vib_depth=0.25, vib_delay=250, vib_rate=6, amp_r=90, dly_mix=0.15, dly_time=D8D, dly_fb=0.3)
P("LD Hero Pulse 12", "LEAD", oscA_shape=P12, voice_mode=LEGATO, glide=25,
  vib_depth=0.22, vib_delay=300, amp_r=90, dly_mix=0.15, dly_time=D8D)
P("LD Square Knight", "LEAD", oscA_shape=P50, oscB_wave=PULSE, oscB_shape=P25, oscB_level=0.5, oscB_fine=8,
  voice_mode=MONO, vib_depth=0.2, vib_delay=200, rev_mix=0.12)
P("LD Breakdown Screamer", "LEAD", oscA_shape=P25, oscA_uni=3, oscA_detune=0.3, oscB_wave=SAW, oscB_level=0.45,
  voice_mode=LEGATO, glide=40, dist_type=HARD, dist_drive=0.6, dist_tone=0.7, vib_depth=0.3, vib_delay=350,
  amp_r=100, dly_mix=0.18, dly_time=D8D, master_gain=-9)
P("LD Mosh Pit Saw", "LEAD", oscA_wave=SUPERSAW, oscA_shape=0.6, oscA_detune=0.5, dist_type=SOFT, dist_drive=0.4,
  flt_type=LP24, flt_cutoff=9000, voice_mode=LEGATO, glide=30, dly_mix=0.2, dly_time=D8D, rev_mix=0.15, master_gain=-8)
P("LD Inner World", "LEAD", oscA_wave=SUPERSAW, oscA_shape=0.65, oscA_detune=0.45, oscB_wave=PULSE, oscB_shape=P12,
  oscB_oct=1, oscB_level=0.35, chorus_mix=0.3, rev_mix=0.25, rev_size=0.6, dly_mix=0.25, dly_time=D8D, dly_fb=0.4,
  vib_depth=0.15, vib_delay=400, amp_a=5, amp_r=300, master_gain=-8)
P("LD Dinner Bell", "LEAD", oscA_wave=GB, oscA_shape=gbt(0), oscB_wave=PULSE, oscB_shape=P12, oscB_oct=1,
  oscB_level=0.3, vib_depth=0.2, vib_delay=200, amp_d=500, amp_s=0.7, amp_r=150, dly_mix=0.2, dly_time=D16D)
P("LD Shotgun Blip", "LEAD", oscA_shape=P50, penv_amt=12, penv_time=25, voice_mode=MONO, dist_type=HARD,
  dist_drive=0.5, crush_bits=6, amp_r=60, master_gain=-9)
P("LD Grammar Lesson", "LEAD", oscA_wave=FM, oscA_shape=0.3, oscA_ratio=R2, sub_level=0.35, sub_wave=2,
  vib_depth=0.15, vib_delay=300, voice_mode=LEGATO, glide=20, amp_r=150, rev_mix=0.15)
P("LD Snow Eater", "LEAD", oscA_wave=SAW, oscA_shape=0.5, oscB_wave=PULSE, oscB_shape=P25, oscB_fine=-12,
  oscB_level=0.5, dist_type=FUZZ, dist_drive=0.55, dist_tone=0.75, crush_rate=0.2, voice_mode=LEGATO, glide=60,
  vib_depth=0.25, vib_delay=300, master_gain=-9)
P("LD Escimo Ice", "LEAD", oscA_wave=GB, oscA_shape=gbt(4), oscB_wave=TRI, oscB_oct=1, oscB_level=0.5,
  vib_depth=0.2, vib_delay=250, dly_mix=0.3, dly_time=D8D, dly_fb=0.45, rev_mix=0.35, rev_size=0.7, amp_r=200)
P("LD 4-Bit Volume", "LEAD", oscA_shape=P25, amp_quant=1, amp_d=700, amp_s=0.55, amp_r=200, vib_depth=0.2,
  vib_delay=200, voice_mode=MONO)
P("LD Glide Hero", "LEAD", oscA_shape=P12, oscB_wave=PULSE, oscB_shape=P50, oscB_oct=-1, oscB_level=0.4,
  voice_mode=LEGATO, glide=120, vib_depth=0.2, vib_delay=300, dly_mix=0.15)
P("LD Pitch Bomb", "LEAD", oscA_shape=P25, penv_amt=-24, penv_time=60, voice_mode=MONO, amp_r=80, dly_mix=0.12)
P("LD Fuzz Chip Guitar", "LEAD", oscA_shape=P50, oscB_wave=PULSE, oscB_shape=P25, oscB_level=0.7, oscB_fine=14,
  dist_type=FUZZ, dist_drive=0.8, dist_tone=0.55, flt_type=LP12, flt_cutoff=6000, flt_reso=0.25,
  voice_mode=LEGATO, glide=15, vib_depth=0.2, vib_delay=400, amp_r=80, master_gain=-10)
P("LD Supersaw Anthem", "LEAD", oscA_wave=SUPERSAW, oscA_detune=0.65, oscA_shape=0.7, oscB_wave=SUPERSAW,
  oscB_oct=1, oscB_level=0.35, oscB_detune=0.5, oscB_shape=0.6, chorus_mix=0.3, rev_mix=0.3, rev_size=0.65,
  dly_mix=0.2, dly_time=D8D, amp_r=250, master_gain=-9)
P("LD FM Scream", "LEAD", oscA_wave=FM, oscA_ratio=R3, oscA_shape=0.6, lfo_target=T_SHAPE, lfo_wave=L_SINE,
  lfo_rate=5, lfo_amt=0.4, dist_type=SOFT, dist_drive=0.5, voice_mode=LEGATO, glide=30, master_gain=-9)
P("LD Wavefold Rage", "LEAD", oscA_wave=SINE, oscA_shape=0.7, dist_type=FOLD, dist_drive=0.3, voice_mode=MONO,
  vib_depth=0.2, vib_delay=300, amp_r=100)
P("LD Arcade Whistle", "LEAD", oscA_wave=SINE, oscA_shape=0.0, vib_depth=0.35, vib_delay=100, vib_rate=7,
  voice_mode=LEGATO, glide=40, amp_a=10, amp_r=150, rev_mix=0.2)
P("LD Boss Fight", "LEAD", oscA_wave=SAW, oscA_shape=0.0, oscA_uni=5, oscA_detune=0.35, oscB_wave=PULSE,
  oscB_shape=P12, oscB_oct=1, oscB_level=0.4, flt_type=LP24, flt_cutoff=5000, flt_env=0.3, fenv_d=400,
  dist_type=SOFT, dist_drive=0.5, voice_mode=LEGATO, glide=30, dly_mix=0.18, dly_time=D8D, master_gain=-9)
P("LD Pulse Wobble", "LEAD", oscA_shape=0.5, lfo_target=T_SHAPE, lfo_wave=L_TRI, lfo_rate=6, lfo_amt=0.6,
  voice_mode=MONO, amp_r=100, dly_mix=0.15)
P("LD Crushed Hero", "LEAD", oscA_shape=P25, crush_bits=4, crush_rate=0.35, voice_mode=LEGATO, glide=20,
  vib_depth=0.2, vib_delay=250, master_gain=-9)

# =============================================================================
# BASS
# =============================================================================
P("BS NES Triangle", "BASS", oscA_wave=TRI, oscA_shape=0.0, oscA_level=1.0, voice_mode=MONO, amp_r=30)
P("BS Triangle Thump", "BASS", oscA_wave=TRI, oscA_shape=0.0, oscA_level=1.0, penv_amt=12, penv_time=30,
  sub_level=0.3, sub_wave=1, voice_mode=MONO, amp_r=40)
P("BS Pulse Chug", "BASS", oscA_shape=P50, sub_level=0.5, dist_type=HARD, dist_drive=0.4, flt_type=LP24,
  flt_cutoff=1200, flt_env=0.35, fenv_d=150, voice_mode=MONO, amp_d=250, amp_s=0.6, amp_r=40, master_gain=-8)
P("BS Djent Pulse", "BASS", oscA_shape=P12, oscB_wave=SAW, oscB_oct=-1, oscB_level=0.6, dist_type=FUZZ,
  dist_drive=0.6, flt_type=LP24, flt_cutoff=800, flt_reso=0.3, flt_env=0.5, fenv_d=120, voice_mode=MONO,
  amp_r=40, master_gain=-9)
P("BS Saw Growl", "BASS", oscA_wave=SAW, oscA_shape=0.0, oscA_uni=3, oscA_detune=0.2, flt_type=LP24,
  flt_cutoff=400, flt_env=0.6, fenv_d=250, fenv_s=0.2, dist_type=SOFT, dist_drive=0.35, voice_mode=LEGATO,
  glide=20, amp_r=50)
P("BS GB Wave Bass", "BASS", oscA_wave=GB, oscA_shape=gbt(1), oscA_level=0.9, sub_level=0.4, sub_wave=1,
  voice_mode=MONO, amp_r=40)
P("BS Reese Cart", "BASS", oscA_wave=SAW, oscA_shape=0.0, oscA_uni=4, oscA_detune=0.5, flt_type=LP24,
  flt_cutoff=1500, sub_level=0.3, voice_mode=LEGATO, glide=40, amp_r=60)
P("BS FM Punch", "BASS", oscA_wave=FM, oscA_ratio=R1, oscA_shape=0.5, penv_amt=7, penv_time=25,
  flt_type=LP12, flt_cutoff=2000, voice_mode=MONO, amp_r=40)
P("BS Octave Pump", "BASS", oscA_shape=P25, arp_on=1, arp_rate=A16, chord=C_OCT, arp_gate=0.6, sub_level=0.3,
  amp_r=30)
P("BS Sub Rumble", "BASS", oscA_wave=SINE, oscA_shape=0.1, oscA_level=0.9, sub_level=0.5, sub_wave=1,
  voice_mode=MONO, amp_r=60)
P("BS Wobble 8-Bit", "BASS", oscA_wave=SAW, oscA_shape=0.0, flt_type=LP24, flt_cutoff=600, flt_reso=0.5,
  lfo_target=T_CUT, lfo_wave=L_SINE, lfo_rate=3, lfo_amt=0.5, crush_bits=8, voice_mode=LEGATO, glide=30)
P("BS Breakdown Drop", "BASS", oscA_shape=P50, sub_level=0.5, penv_amt=-12, penv_time=400, dist_type=SOFT,
  dist_drive=0.45, voice_mode=MONO, amp_r=80, master_gain=-8)
P("BS Slap Chip", "BASS", oscA_shape=P25, flt_type=LP12, flt_cutoff=900, flt_reso=0.4, flt_env=0.7, fenv_d=120,
  fenv_s=0.2, voice_mode=MONO, amp_r=40)
P("BS Fuzz Chug Gtr", "BASS", oscA_shape=P50, oscB_wave=PULSE, oscB_shape=P25, oscB_level=0.7, oscB_fine=10,
  dist_type=FUZZ, dist_drive=0.85, dist_tone=0.45, flt_type=LP24, flt_cutoff=2500, voice_mode=MONO,
  amp_d=200, amp_s=0.5, amp_r=40, master_gain=-10)
P("BS Rolling 16ths", "BASS", oscA_wave=TRI, oscA_shape=0.0, oscB_wave=PULSE, oscB_shape=P12, oscB_level=0.3,
  arp_on=1, arp_rate=A16, arp_gate=0.5, amp_r=30)
P("BS Acid Cart", "BASS", oscA_wave=SAW, oscA_shape=0.0, flt_type=LP24, flt_cutoff=300, flt_reso=0.75,
  flt_env=0.7, fenv_d=200, dist_type=SOFT, dist_drive=0.3, voice_mode=LEGATO, glide=80, amp_r=50)

# =============================================================================
# ARP
# =============================================================================
P("AR Major Sparkle", "ARP", oscA_shape=P12, arp_on=1, chord=C_MAJ, arp_rate=A32, arp_oct=2, dly_mix=0.2,
  dly_time=D8D)
P("AR Minor Rush", "ARP", oscA_shape=P25, arp_on=1, chord=C_MINO, arp_rate=A32, arp_mode=UPDOWN, dly_mix=0.15)
P("AR Power Stutter", "ARP", oscA_shape=P50, arp_on=1, chord=C_POW, arp_rate=A16, arp_gate=0.5,
  dist_type=HARD, dist_drive=0.45, master_gain=-9)
P("AR Dinner Is Ready", "ARP", oscA_wave=GB, oscA_shape=gbt(0), arp_on=1, chord=C_MAJ7, arp_rate=A16T,
  arp_oct=2, dly_mix=0.25, dly_time=D8D, dly_pp=1)
P("AR Classic NES 1-64", "ARP", oscA_shape=P25, arp_on=1, chord=C_MAJ, arp_rate=A64, arp_gate=1.0, amp_r=80)
P("AR Minor NES 1-64", "ARP", oscA_shape=P12, arp_on=1, chord=C_MIN, arp_rate=A64, arp_gate=1.0, amp_r=80,
  vib_depth=0.15, vib_delay=300)
P("AR Sus Climber", "ARP", oscA_wave=TRI, oscA_shape=0.0, arp_on=1, chord=C_SUS4, arp_oct=3, arp_rate=A16,
  dly_mix=0.15)
P("AR Random Coins", "ARP", oscA_wave=GB, oscA_shape=gbt(4), arp_on=1, chord=C_MAJO, arp_oct=2, arp_mode=RANDOM,
  arp_rate=A16, amp_d=150, amp_s=0.0, amp_r=80, dly_mix=0.2, dly_time=D16D)
P("AR Trance Gate", "ARP", oscA_wave=SUPERSAW, oscA_detune=0.45, oscA_shape=0.6, arp_on=1, arp_rate=A16,
  arp_gate=0.4, rev_mix=0.3, dly_mix=0.15, master_gain=-8)
P("AR Dim Panic", "ARP", oscA_shape=P12, arp_on=1, chord=C_DIM, arp_rate=A32, arp_mode=UPDOWN, arp_oct=2,
  vib_depth=0.2, vib_delay=0)
P("AR 5th Stack Fanfare", "ARP", oscA_shape=P25, oscB_wave=PULSE, oscB_shape=P50, oscB_oct=-1, oscB_level=0.4,
  arp_on=1, chord=C_5TH, arp_rate=A16T, arp_oct=2)
P("AR Echo Cascade", "ARP", oscA_shape=P25, amp_d=200, amp_s=0.0, amp_r=150, arp_on=1, arp_rate=A16, arp_oct=2,
  dly_mix=0.35, dly_time=D8D, dly_fb=0.55)
P("AR Played Order FM", "ARP", oscA_wave=FM, oscA_ratio=R2, oscA_shape=0.35, arp_on=1, arp_mode=PLAYED,
  arp_rate=A16, arp_gate=0.7, dly_mix=0.15)
P("AR Min7 Bubbles", "ARP", oscA_wave=GB, oscA_shape=gbt(4), arp_on=1, chord=C_MIN7, arp_rate=A32T, arp_oct=2,
  chorus_mix=0.35)

# =============================================================================
# CHORD / STAB  (one key = whole chord where noted)
# =============================================================================
P("CH Mosh Stab Minor", "CHORD", oscA_wave=SUPERSAW, oscA_detune=0.4, oscA_shape=0.6, chord=C_MIN,
  amp_d=250, amp_s=0.0, amp_r=120, dist_type=SOFT, dist_drive=0.4, master_gain=-10)
P("CH Major Stab", "CHORD", oscA_shape=P25, oscB_wave=SAW, oscB_level=0.4, chord=C_MAJ, amp_d=220, amp_s=0.0,
  amp_r=100, rev_mix=0.15, master_gain=-9)
P("CH Min7 Chip Stab", "CHORD", oscA_shape=P12, chord=C_MIN7, amp_d=300, amp_s=0.0, amp_r=120,
  dly_mix=0.2, dly_time=D8D, master_gain=-9)
P("CH Power Gate", "CHORD", oscA_wave=SAW, oscA_shape=0.0, oscA_uni=3, oscA_detune=0.3, chord=C_POW,
  lfo_target=T_VOL, lfo_wave=L_SQ, lfo_rate=10, lfo_amt=0.9, dist_type=HARD, dist_drive=0.4, master_gain=-10)
P("CH Euphoric Saws", "CHORD", oscA_wave=SUPERSAW, oscA_detune=0.55, oscA_shape=0.7, amp_a=5, amp_d=600,
  amp_s=0.7, amp_r=400, chorus_mix=0.3, rev_mix=0.35, rev_size=0.7, master_gain=-11)
P("CH Octave Hits", "CHORD", oscA_shape=P50, chord=C_OCT, penv_amt=12, penv_time=10, amp_d=300, amp_s=0.0,
  amp_r=100)
P("CH Chip Brass", "CHORD", oscA_wave=SAW, oscA_shape=0.4, oscB_wave=PULSE, oscB_shape=P25, oscB_level=0.5,
  flt_type=LP12, flt_cutoff=3000, flt_env=0.3, fenv_a=30, fenv_d=300, amp_a=20, amp_r=150, master_gain=-9)
P("CH Detuned Organ", "CHORD", oscA_wave=GB, oscA_shape=gbt(2), oscB_wave=GB, oscB_shape=gbt(2), oscB_oct=1,
  oscB_level=0.5, oscB_uni=2, oscB_detune=0.3, chorus_mix=0.35, amp_r=80, master_gain=-9)
P("CH Crushed Chord", "CHORD", oscA_shape=P25, chord=C_MAJO, crush_bits=5, crush_rate=0.3, amp_d=400, amp_s=0.3,
  amp_r=120, master_gain=-10)
P("CH Trancecore Pluck", "CHORD", oscA_wave=SUPERSAW, oscA_detune=0.5, oscA_shape=0.6, flt_type=LP24,
  flt_cutoff=600, flt_env=0.6, fenv_d=200, fenv_s=0.1, chord=C_MINO, amp_d=400, amp_s=0.0, amp_r=200,
  dly_mix=0.25, dly_time=D8D, rev_mix=0.2, master_gain=-11)

# =============================================================================
# PLUCK / KEYS
# =============================================================================
P("PL Pulse Pluck", "PLUCK", oscA_shape=P25, amp_d=180, amp_s=0.0, amp_r=150, dly_mix=0.15, dly_time=D8D)
P("PL Triangle Mallet", "PLUCK", oscA_wave=TRI, oscA_shape=0.3, oscA_level=1.0, amp_d=300, amp_s=0.0,
  amp_r=200, dly_mix=0.2)
P("PL GB Bell", "PLUCK", oscA_wave=GB, oscA_shape=gbt(0), amp_d=500, amp_s=0.0, amp_r=300, rev_mix=0.2)
P("PL FM Marimba", "PLUCK", oscA_wave=FM, oscA_ratio=R4, oscA_shape=0.4, amp_d=250, amp_s=0.0, amp_r=150,
  rev_mix=0.15)
P("PL Weekly Keys", "PLUCK", oscA_wave=FM, oscA_ratio=R1, oscA_shape=0.25, oscB_wave=PULSE, oscB_shape=P50,
  oscB_level=0.3, amp_d=900, amp_s=0.3, amp_r=300, chorus_mix=0.2)
P("PL Harp Sparkle", "PLUCK", oscA_shape=P12, oscB_wave=TRI, oscB_oct=1, oscB_level=0.5, amp_d=400, amp_s=0.0,
  amp_r=300, rev_mix=0.3)
P("PL Crystal Snow", "PLUCK", oscA_wave=SINE, oscA_shape=0.3, oscB_wave=GB, oscB_shape=gbt(4), oscB_oct=2,
  oscB_level=0.3, amp_d=600, amp_s=0.0, amp_r=400, dly_mix=0.25, dly_pp=1, rev_mix=0.3)
P("PL Saw Pluck", "PLUCK", oscA_wave=SAW, oscA_shape=0.0, flt_type=LP24, flt_cutoff=300, flt_reso=0.3,
  flt_env=0.7, fenv_d=150, amp_d=350, amp_s=0.0, amp_r=200)
P("PL Koto Chip", "PLUCK", oscA_shape=P12, penv_amt=2, penv_time=20, amp_d=300, amp_s=0.0, amp_r=200,
  rev_mix=0.15)
P("PL Music Box", "PLUCK", oscA_wave=GB, oscA_shape=gbt(4), oscA_oct=1, amp_d=700, amp_s=0.0, amp_r=400,
  rev_mix=0.3)

# =============================================================================
# PAD
# =============================================================================
P("PD Overworld Dawn", "PAD", oscA_wave=SUPERSAW, oscA_detune=0.5, oscA_shape=0.7, amp_a=600, amp_r=1500,
  flt_type=LP24, flt_cutoff=4000, chorus_mix=0.4, rev_mix=0.5, rev_size=0.8, master_gain=-11)
P("PD Pulse Choir", "PAD", oscA_shape=P50, oscA_uni=5, oscA_detune=0.4, lfo_target=T_SHAPE, lfo_rate=0.3,
  lfo_amt=0.3, amp_a=400, amp_r=1200, rev_mix=0.45, master_gain=-10)
P("PD Frozen Cartridge", "PAD", oscA_wave=GB, oscA_shape=gbt(4), oscA_uni=4, oscA_detune=0.5, oscB_wave=TRI,
  oscB_level=0.4, amp_a=800, amp_r=1500, crush_bits=8, rev_mix=0.6, rev_size=0.85, master_gain=-10)
P("PD Inner World Haze", "PAD", oscA_wave=SAW, oscA_shape=0.0, oscA_uni=7, oscA_detune=0.3, flt_type=LP24,
  flt_cutoff=1500, lfo_target=T_CUT, lfo_rate=0.2, lfo_amt=0.3, amp_a=700, amp_r=2000, rev_mix=0.6,
  rev_size=0.85, dly_mix=0.2, master_gain=-10)
P("PD Save Point", "PAD", oscA_wave=SINE, oscA_shape=0.0, oscB_wave=FM, oscB_ratio=R2, oscB_shape=0.1,
  oscB_level=0.5, amp_a=1000, amp_r=2000, chorus_mix=0.5, rev_mix=0.5, rev_size=0.75)
P("PD Dark Dungeon", "PAD", oscA_wave=SAW, oscA_shape=0.0, oscA_oct=-1, oscB_wave=PULSE, oscB_shape=P25,
  oscB_level=0.5, oscB_fine=10, flt_type=LP24, flt_cutoff=800, flt_reso=0.2, lfo_target=T_CUT, lfo_rate=0.1,
  lfo_amt=0.25, amp_a=900, amp_r=1800, rev_mix=0.5, master_gain=-9)
P("PD 4-Bit Strings", "PAD", oscA_wave=SAW, oscA_shape=0.3, oscA_uni=3, oscA_detune=0.35, amp_a=300, amp_r=800,
  chorus_mix=0.4, flt_type=LP12, flt_cutoff=6000, master_gain=-10)
P("PD Snowfield", "PAD", oscA_wave=TRI, oscA_shape=0.6, oscB_wave=GB, oscB_shape=gbt(2), oscB_level=0.4,
  oscB_oct=1, amp_a=1200, amp_r=2500, rev_mix=0.7, rev_size=0.9, dly_mix=0.2, dly_time=D4)

# =============================================================================
# DRUMS  (one-shots, play around C3-C5)
# =============================================================================
DRUM = dict(voice_mode=POLY, amp_s=0.0, vel_sens=0.7)
P("DR NES Kick", "DRUM", **DRUM, oscA_wave=TRI, oscA_shape=0.0, oscA_oct=-2, oscA_level=1.0, penv_amt=36,
  penv_time=25, amp_d=180, amp_r=50)
P("DR Pulse Kick", "DRUM", **DRUM, oscA_shape=P50, oscA_oct=-2, penv_amt=48, penv_time=15, amp_d=150, amp_r=40,
  dist_type=SOFT, dist_drive=0.4)
P("DR Breakdown Kick", "DRUM", **DRUM, oscA_wave=SINE, oscA_shape=0.2, oscA_oct=-2, oscA_level=1.0,
  penv_amt=40, penv_time=40, amp_d=400, amp_r=80, dist_type=SOFT, dist_drive=0.5, sub_level=0.3)
P("DR NES Snare", "DRUM", **DRUM, oscA_wave=TRI, oscA_shape=0.0, oscA_level=0.4, penv_amt=12, penv_time=30,
  noise_level=0.8, noise_tone=0.75, noise_decay=150, amp_d=200, amp_r=60)
P("DR Crack Snare", "DRUM", **DRUM, oscA_shape=P50, oscA_level=0.4, penv_amt=24, penv_time=10, noise_level=0.9,
  noise_tone=0.9, noise_decay=120, amp_d=180, amp_r=50, dist_type=HARD, dist_drive=0.3)
P("DR Metal Snare", "DRUM", **DRUM, oscA_wave=NOISE, oscA_shape=0.0, oscA_level=0.4, noise_level=0.8,
  noise_short=1, noise_tone=0.6, noise_decay=100, amp_d=180, amp_r=50)
P("DR Closed Hat", "DRUM", **DRUM, oscA_level=0.0, noise_level=1.0, noise_tone=1.0, noise_decay=35,
  flt_type=HP12, flt_cutoff=7000, amp_d=60, amp_r=30)
P("DR Open Hat", "DRUM", **DRUM, oscA_level=0.0, noise_level=1.0, noise_tone=1.0, noise_decay=300,
  flt_type=HP12, flt_cutoff=6000, amp_d=350, amp_r=80)
P("DR Metal Hat", "DRUM", **DRUM, oscA_level=0.0, noise_level=1.0, noise_short=1, noise_tone=0.95,
  noise_decay=50, flt_type=HP12, flt_cutoff=5000, amp_d=70, amp_r=30)
P("DR Crash 8-Bit", "DRUM", **DRUM, oscA_level=0.0, noise_level=1.0, noise_tone=0.95, noise_decay=1200,
  flt_type=HP12, flt_cutoff=3000, amp_d=1500, amp_r=300, crush_bits=8)
P("DR Tom Drop", "DRUM", **DRUM, oscA_wave=TRI, oscA_shape=0.0, oscA_oct=-1, oscA_level=1.0, penv_amt=12,
  penv_time=120, amp_d=300, amp_r=80)
P("DR Clap Chip", "DRUM", **DRUM, oscA_level=0.0, noise_level=1.0, noise_tone=0.85, noise_decay=90,
  flt_type=HP12, flt_cutoff=900, flt_reso=0.3, lfo_target=T_VOL, lfo_wave=L_SQ, lfo_rate=30, lfo_amt=0.6,
  amp_d=160, amp_r=60)
P("DR Zap Kick", "DRUM", **DRUM, oscA_wave=FM, oscA_ratio=R2, oscA_shape=0.6, oscA_oct=-2, penv_amt=48,
  penv_time=20, amp_d=200, amp_r=40)

# =============================================================================
# FX
# =============================================================================
P("FX Coin Get", "FX", oscA_shape=P50, penv_amt=-5, penv_time=40, amp_quant=1, amp_d=600, amp_s=0.0, amp_r=100,
  voice_mode=MONO)
P("FX Power Up", "FX", oscA_shape=P25, arp_on=1, chord=C_MAJO, arp_rate=A64, arp_oct=3, arp_gate=1.0,
  lfo_target=T_PITCH, lfo_wave=L_SAW, lfo_rate=1.5, lfo_amt=0.5, amp_d=1500, amp_s=0.0, amp_r=100)
P("FX Laser Shot", "FX", oscA_shape=P12, penv_amt=48, penv_time=150, amp_d=250, amp_s=0.0, amp_r=60)
P("FX Jump", "FX", oscA_shape=P25, penv_amt=-12, penv_time=150, amp_d=300, amp_s=0.0, amp_r=60, voice_mode=MONO)
P("FX Explosion", "FX", oscA_level=0.0, noise_level=1.0, noise_tone=0.45, noise_decay=900, crush_bits=6,
  flt_type=LP12, flt_cutoff=2000, amp_d=1200, amp_s=0.0, amp_r=300, penv_amt=-12, penv_time=800)
P("FX 1-Up Chime", "FX", oscA_shape=P12, arp_on=1, chord=C_MAJO, arp_rate=A32, arp_oct=2, amp_d=900,
  amp_s=0.0, amp_r=200, dly_mix=0.2)
P("FX Game Over Dive", "FX", oscA_wave=SAW, oscA_shape=0.5, penv_amt=24, penv_time=1500, amp_d=1800, amp_s=0.0,
  amp_r=200, voice_mode=MONO)
P("FX Warp Riser", "FX", oscA_wave=SUPERSAW, oscA_detune=0.6, oscA_shape=0.6, lfo_target=T_PITCH,
  lfo_wave=L_SAW, lfo_rate=0.25, lfo_amt=0.7, flt_type=LP24, flt_cutoff=3000, rev_mix=0.4, master_gain=-10)
P("FX Glitch Stutter", "FX", oscA_shape=P25, lfo_target=T_VOL, lfo_wave=L_SH, lfo_rate=12, lfo_amt=0.9,
  crush_bits=4, crush_rate=0.5, master_gain=-9)
P("FX Alarm Siren", "FX", oscA_shape=P50, lfo_target=T_PITCH, lfo_wave=L_TRI, lfo_rate=2, lfo_amt=0.55,
  voice_mode=MONO)


# ---- emit -------------------------------------------------------------------
def fmt(v):
    s = ("%.4f" % float(v)).rstrip("0").rstrip(".")
    if "." not in s:
        s += ".0"
    return s + "f"


def load_trims(root):
    """Per-preset loudness trims (dB) measured by tools/calibrate.py."""
    path = os.path.join(root, "tools", "trims.json")
    if os.path.exists(path):
        with open(path) as f:
            return json.load(f)
    return {}


def main():
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    out = os.path.join(root, "Source", "Presets.h")
    trims = load_trims(root)
    names = set()
    lines = [
        "// AUTO-GENERATED by tools/gen_presets.py - do not edit by hand",
        "#pragma once",
        "#include <vector>",
        "#include <utility>",
        "",
        "namespace chip",
        "{",
        "struct PresetDef",
        "{",
        "    const char* name;",
        "    const char* category;",
        "    std::vector<std::pair<const char*, float>> values;",
        "};",
        "",
        "inline const std::vector<PresetDef>& factoryPresets()",
        "{",
        "    static const std::vector<PresetDef> p {",
    ]
    for name, cat, kw in PRESETS:
        assert name not in names, name
        names.add(name)
        kw = dict(kw)
        if name in trims:
            g = kw.get("master_gain", -6.0) + trims[name]
            kw["master_gain"] = round(max(-24.0, min(6.0, g)), 2)
        vals =", ".join('{ "%s", %s }' % (k, fmt(v)) for k, v in kw.items())
        lines.append('        { "%s", "%s", { %s } },' % (name, cat, vals))
    lines += ["    };", "    return p;", "}", "} // namespace chip", ""]
    with open(out, "w") as f:
        f.write("\n".join(lines))
    cats = {}
    for _, c, _ in PRESETS:
        cats[c] = cats.get(c, 0) + 1
    print("wrote %d presets -> %s  %s" % (len(PRESETS), out, cats))


if __name__ == "__main__":
    main()
