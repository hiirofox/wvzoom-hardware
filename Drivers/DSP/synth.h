#pragma once

#include <math.h>
#include <stdlib.h>
#include "midi.h"

typedef struct
{
    float t, tri;
} Oscillator;
void ResetOscillator(Oscillator *p);
float OscProcSample(Oscillator *p, float f, float ratio, float form);

typedef struct
{
    float l, r;
} StereoSignal;

#define MaxUnisonNum 3
typedef struct
{
    Oscillator oscl[MaxUnisonNum];
    Oscillator oscr[MaxUnisonNum];
    int n;
    float df;
} UnisonOsc;
void ResetUnisonOsc(UnisonOsc *p);
StereoSignal UnisonOscProcSample(UnisonOsc *p, float f, float ratio, float form);
void UnisonOscSetParam(UnisonOsc *p, int n, float df);

typedef struct
{
    float a, d, s, r;
    float v;
    int state;
} ADSR;
void ResetADSR(ADSR *p);
void ADSRSetTrig(ADSR *p, int trig);
float ADSRProcSample(ADSR *p);
void ADSRSetParam(ADSR *p, float a, float d, float s, float r);

#define MaxPolyNum 8
typedef struct
{
    float ratio, form; // osc

    int n; // unison
    float df;

    float oa, od, os, or ; // oscillator adsr
    float fa, fd, fs, fr;  // filter adsr
} SynthParam;

typedef struct
{
    SynthParam param;

    int trig[MaxPolyNum];
    int note[MaxPolyNum];
    float freq[MaxPolyNum];
    float vol[MaxPolyNum];
    UnisonOsc osc[MaxPolyNum];
    int trigPos;
} Synth;
void ResetSynth(Synth *p);
void SynthProcMidi(Synth *p, midi_msg msg);
void SynthProcessBlock(Synth *p, float *bufl, float *bufr, int numSamples);