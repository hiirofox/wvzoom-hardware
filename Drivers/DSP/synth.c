#include "synth.h"

////////////////////////////////////////////////////////////// oscillator
void ResetOscillator(Oscillator *p)
{
    p->t = 0;
    p->tri = 0;
}

inline float OscProcSample(Oscillator *p, float f, float ratio, float form)
{ // f是归一化的！
    p->t += f;
    p->t -= (int)p->t;
    return ((1.0 - form) * p->t + form * (p->t >= ratio)) * 2.0 - 1.0;
}

////////////////////////////////////////////////////////////// unison oscillator
void ResetUnisonOsc(UnisonOsc *p)
{
    for (int i = 0; i < MaxUnisonNum; i++)
    {
        ResetOscillator(&p->oscl[i]);
        ResetOscillator(&p->oscr[i]);
        p->oscl[i].t = (float)(rand() % 10000) / 10000.0;
        p->oscr[i].t = (float)(rand() % 10000) / 10000.0;
    }
}
StereoSignal UnisonOscProcSample(UnisonOsc *p, float f, float ratio, float form)
{
    StereoSignal sum = {0, 0};
    float f1 = f;
    for (int i = 0; i < p->n; i++)
    {
        sum.l += OscProcSample(&p->oscl[i], f1, ratio, form);
        sum.r += OscProcSample(&p->oscr[i], f1, ratio, form);
        f1 *= p->df;
    }
    return sum;
}

void UnisonOscSetParam(UnisonOsc *p, int n, float df)
{
    p->n = n;
    p->df = df;
}

////////////////////////////////////////////////////////////// adsr

////////////////////////////////////////////////////////////// synth

void ResetSynth(Synth *p)
{
    for (int i = 0; i < MaxPolyNum; i++)
    {
        ResetUnisonOsc(&p->osc[i]);
    }
    p->param.ratio = 0.5;
    p->param.form = 0.5;
    p->trigPos = 0;
}
void SynthProcMidi(Synth *p, midi_msg msg)
{
    if (msg.trig == 1) // note on
    {
        int hasNoteOff = -1;
        for (int i = 0; i < MaxPolyNum; ++i)
        {
            if (p->trig[i] == 0)
            {
                hasNoteOff = i;
                break;
            }
        }
        if (hasNoteOff != -1)
        {
            p->trigPos = hasNoteOff;
        }
        p->trig[p->trigPos] = 1;
        p->note[p->trigPos] = msg.note;
        p->freq[p->trigPos] = msg.freq / 48000.0;
        p->vol[p->trigPos] = msg.vol;
        ResetUnisonOsc(&p->osc[p->trigPos]);

        p->trigPos = (p->trigPos + 1) % MaxPolyNum;
    }
    else if (msg.trig == 0) // note off
    {
        for (int i = 0; i < MaxPolyNum; i++)
        {
            if (p->note[i] == msg.note)
            {
                p->trig[i] = 0;
                p->note[i] = 0;
                p->vol[i] = 0;
                break;
            }
        }
    }
    else if (msg.trig == 2) // param
    {
        if (msg.channel == 0)
        {
            p->param.form = msg.chanVal;
        }
        else if (msg.channel == 1)
        {
            p->param.ratio = msg.chanVal * 0.48 + 0.5;
        }
        else if (msg.channel == 2)
        {
            p->param.n = (int)(msg.chanVal * (MaxUnisonNum - 1)) + 1;
        }
        else if (msg.channel == 3)
        {
            p->param.df = msg.chanVal * 0.01 + 1.0;
        }

        for (int i = 0; i < MaxPolyNum; i++)
        {
            UnisonOscSetParam(&p->osc[i], p->param.n, p->param.df);
        }
    }
}

void SynthProcessBlock(Synth *p, float *bufl, float *bufr, int numSamples)
{
    StereoSignal tmp;
    for (int i = 0; i < numSamples; i++)
    {
        float suml = 0, sumr = 0;
        for (int j = 0; j < MaxPolyNum; j++)
        {
            tmp = UnisonOscProcSample(&p->osc[j], p->freq[j], p->param.ratio, p->param.form);
            suml += tmp.l * p->vol[j];
            sumr += tmp.r * p->vol[j];
        }
        bufl[i] = suml;
        bufr[i] = sumr;
    }
}