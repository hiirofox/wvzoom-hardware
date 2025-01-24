#pragma once

#include <math.h>
#include <stdlib.h>
#include "midi.h"

typedef struct
{
    float t1, t2;
} Oscillator;
void ResetOscillator(Oscillator *p);
float OscProcSampleSync(Oscillator *p,
                        float f,
                        float form1, float ratio1,
                        float form2, float ratio2, float pitch2,
                        float deep, float mix);
float OscProcSampleFM(Oscillator *p,
                      float f,
                      float form1, float ratio1,
                      float form2, float ratio2, float pitch2,
                      float deep, float mix);
float OscProcSampleAM(Oscillator *p,
                      float f,
                      float form1, float ratio1,
                      float form2, float ratio2, float pitch2,
                      float deep, float mix);
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
void UnisonOscSetParam(UnisonOsc *p, int n, float df);
StereoSignal UnisonOscProcSample(UnisonOsc *p,
                                 float f,
                                 float form1, float ratio1,
                                 float form2, float ratio2, float pitch2,
                                 float deep, float mix, int mode);

typedef struct
{
    float z1, z2;
} SVFilter;

void ResetSVF(SVFilter *p);
void SVFCheckStability(SVFilter *p, float v, float k); // 数值稳定性检查
float SVFProcLPF(SVFilter *p, float vin, float ctof, float reso);
float SVFProcLPFOverSampling(SVFilter *p, float vin, float ctof, float reso);

typedef struct
{
    float a, d, s, r;
    float v;
    int state; // 0:release 1:attack 2:decay&sustain
} ADSR;
void ResetADSR(ADSR *p);
void ADSRSetTrig(ADSR *p, int trig);
float ADSRProcSample(ADSR *p);
void ADSRSetParam(ADSR *p, float a, float d, float s, float r);

#define MaxPolyNum 8
typedef struct
{
    float form1, ratio1; // osc
    float form2, ratio2, pitch2;
    int oscmode;
    float oscdeep, oscmix;

    int unison; // unison
    float df;

    float oa, od, os, or ;         // oscillator adsr
    float fa, fd, fs, fr, famount; // filter adsr

    float ctof, reso; // filter
} SynthParam;

typedef struct
{
    SynthParam param;

    int trig[MaxPolyNum];
    int note[MaxPolyNum];
    float freq[MaxPolyNum];
    float vol[MaxPolyNum];
    UnisonOsc osc[MaxPolyNum];
    ADSR oscadsr[MaxPolyNum];
    ADSR svfadsr[MaxPolyNum];
    SVFilter svfl[MaxPolyNum];
    SVFilter svfr[MaxPolyNum];
    int trigPos;
} Synth;
void ResetSynth(Synth *p);
void SynthProcMidi(Synth *p, midi_msg msg);
void SynthProcessBlock(Synth *p, float *bufl, float *bufr, int numSamples);