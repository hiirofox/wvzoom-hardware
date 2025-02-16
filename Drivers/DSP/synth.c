#include "synth.h"

////////////////////////////////////////////////////////////// oscillator
void ResetOscillator(Oscillator *p)
{
    p->t1 = 0;
    p->t2 = 0;
}

#define floorf (int32_t) // 有奇效(-1%)
inline float OscProcSampleSync(Oscillator *p,
                               float f,
                               float form1, float ratio1,
                               float form2, float ratio2, float pitch2,
                               float deep, float mix)
{ // f是归一化的！
    p->t2 += f * pitch2;
    p->t1 += f;
    p->t2 *= 1.0 - floorf(p->t1); // hard sync
    p->t1 -= floorf(p->t1);
    p->t2 -= floorf(p->t2);

    // float o1 = ((1.0 - form1) * p->t1 + form1 * (int)(p->t1 + ratio1));
    // float o2 = ((1.0 - form2) * p->t2 + form2 * (int)(p->t2 + ratio2));
    float o1 = (p->t1 - form1 * (p->t1 - floorf(p->t1 + ratio1)));
    float o2 = (p->t2 - form2 * (p->t2 - floorf(p->t2 + ratio2)));

    return (o1 + mix * (o2 - o1));
}
inline float OscProcSampleFM(Oscillator *p,
                             float f,
                             float form1, float ratio1,
                             float form2, float ratio2, float pitch2,
                             float deep, float mix)
{                                             // f是归一化的！
    p->t1 += f + (p->t2 - 0.5) * deep * 0.05; // fm
    p->t2 += f * pitch2;
    p->t1 -= floorf(p->t1);
    p->t2 -= floorf(p->t2);

    // float o1 = ((1.0 - form1) * p->t1 + form1 * (int)(p->t1 + ratio1));
    // float o2 = ((1.0 - form2) * p->t2 + form2 * (int)(p->t2 + ratio2));
    float o1 = (p->t1 - form1 * (p->t1 - floorf(p->t1 + ratio1)));
    float o2 = (p->t2 - form2 * (p->t2 - floorf(p->t2 + ratio2)));

    return (o1 + mix * (o2 - o1));
}
inline float OscProcSampleAM(Oscillator *p,
                             float f,
                             float form1, float ratio1,
                             float form2, float ratio2, float pitch2,
                             float deep, float mix)
{ // f是归一化的！
    p->t1 += f;
    p->t2 += f * pitch2;
    p->t1 -= floorf(p->t1);
    p->t2 -= floorf(p->t2);

    // float o1 = ((1.0 - form1) * p->t1 + form1 * (int)(p->t1 + ratio1));
    // float o2 = ((1.0 - form2) * p->t2 + form2 * (int)(p->t2 + ratio2));
    float o1 = (p->t1 - form1 * (p->t1 - floorf(p->t1 + ratio1)));
    float o2 = (p->t2 - form2 * (p->t2 - floorf(p->t2 + ratio2)));

    return (o2 + (1.0 - mix) * (o1 * o2 - o2));
}
////////////////////////////////////////////////////////////// unison oscillator
void ResetUnisonOsc(UnisonOsc *p)
{
    for (int i = 0; i < MaxUnisonNum; i++)
    {
        ResetOscillator(&p->oscl[i]);
        ResetOscillator(&p->oscr[i]);
        p->oscl[i].t1 = (float)(rand() % 10000) / 10000.0;
        p->oscl[i].t2 = p->oscl[i].t1;
        p->oscr[i].t1 = (float)(rand() % 10000) / 10000.0;
        p->oscr[i].t2 = p->oscr[i].t1;
    }
}
StereoSignal UnisonOscProcSample(UnisonOsc *p,
                                 float f,
                                 float form1, float ratio1,
                                 float form2, float ratio2, float pitch2,
                                 float deep, float mix, int mode)
{
    StereoSignal sum = {0, 0};
    float f1 = f;
    if (mode == 0) // sync
    {
        sum.l = OscProcSampleSync(&p->oscl[0], f1, form1, ratio1, form2, ratio2, pitch2, deep, mix) - 0.5;
        sum.r = sum.l;
        for (int i = 1; i < p->n; i++)
        {
            float v = OscProcSampleSync(&p->oscl[i], f1, form1, ratio1, form2, ratio2, pitch2, deep, mix) - 0.5;
            sum.l += v;
            sum.r -= v;
            f1 *= p->df;
        }
    }

    else if (mode == 1) // fm
    {
        sum.l = OscProcSampleFM(&p->oscl[0], f1, form1, ratio1, form2, ratio2, pitch2, deep, mix) - 0.5;
        sum.r = sum.l;
        for (int i = 1; i < p->n; i++)
        {
            float v = OscProcSampleFM(&p->oscl[i], f1, form1, ratio1, form2, ratio2, pitch2, deep, mix) - 0.5;
            sum.l += v;
            sum.r -= v;
            f1 *= p->df;
        }
    }
    else if (mode == 2) // am
    {
        sum.l = OscProcSampleAM(&p->oscl[0], f1, form1, ratio1, form2, ratio2, pitch2, deep, mix) - 0.5;
        sum.r = sum.l;
        for (int i = 1; i < p->n; i++)
        {
            float v = OscProcSampleAM(&p->oscl[i], f1, form1, ratio1, form2, ratio2, pitch2, deep, mix) - 0.5;
            sum.l += v;
            sum.r -= v;
            f1 *= p->df;
        }
    }
    return sum;
}

void UnisonOscSetParam(UnisonOsc *p, int n, float df)
{
    p->n = n;
    p->df = df;
}

////////////////////////////////////////////////////////////// filter

void ResetSVF(SVFilter *p)
{
    p->z1 = p->z2 = 0;
}
void SVFCheckStability(SVFilter *p, float v, float k) // 数值稳定性检查
{
    if (isnan(p->z1))
        p->z1 = 0; // 假如最可怕的事情还是发生了(极点爆炸)
    if (isnan(p->z2))
        p->z2 = 0;
    if (p->z1 > v)
        p->z1 = (p->z1 - v) * k + v; // 削波
    if (p->z1 < -v)
        p->z1 = (p->z1 + v) * k - v;
    if (p->z2 > v)
        p->z2 = (p->z2 - v) * k + v;
    if (p->z2 < -v)
        p->z2 = (p->z2 + v) * k - v;
}
float SVFProcLPF(SVFilter *p, float vin, float ctof, float reso)
{ // 二阶低通带反馈
    float fb = reso + reso / (1.0 - ctof);
    p->z1 += ctof * (vin - p->z1 + fb * (p->z1 - p->z2));
    p->z2 += ctof * (p->z1 - p->z2);
    return p->z2;
}
float SVFProcLPFOverSampling(SVFilter *p, float vin, float ctof, float reso)
{ // 二阶低通带反馈(超采样防止奈奎斯特频率附近发生变形)
    ctof *= 0.5;
    float fb = reso + reso / (1.0 - ctof);
    p->z1 += ctof * (vin - p->z1 + fb * (p->z1 - p->z2));
    p->z2 += ctof * (p->z1 - p->z2);
    p->z1 += ctof * (vin - p->z1 + fb * (p->z1 - p->z2));
    p->z2 += ctof * (p->z1 - p->z2);
    return p->z2; // 反正fs后面没内容所以不用抗混叠
}

////////////////////////////////////////////////////////////// adsr

void ResetADSR(ADSR *p)
{
    p->v = 0;
    p->state = 0;
}
void ADSRSetTrig(ADSR *p, int trig)
{
    if (trig == 1)
    {
        if (p->state == 0)
        {
            p->state = 1;
        }
    }
    else
    {
        p->state = 0;
    }
}
void ADSRSetParam(ADSR *p, float a, float d, float s, float r)
{
    p->a = a;
    p->d = d;
    p->s = s;
    p->r = r;
}
float ADSRProcSample(ADSR *p)
{
    if (p->state == 1)
    {
        p->v += p->a * (1.99 - p->v);
        p->state = 1 + (int)p->v;
        p->v = ((int)p->v) ? 1.0 : p->v;
    }
    else if (p->state == 2)
    {
        p->v += p->d * (p->s - p->v);
    }
    else
    {
        p->v *= p->r;
    }
    return p->v; // v*v
}

////////////////////////////////////////////////////////////// lfo

void ResetLFO(LFO *p)
{
    p->x = 1.0;
    p->y = 0.0;
}
void LFONormalize(LFO *p)
{
    float v = 1.0 / sqrtf(p->x * p->x + p->y * p->y);
    p->x *= v;
    p->y *= v;
}
void LFOSetFreq(LFO *p, float f)
{
    p->f = f;
}
float LFOProcSample(LFO *p)
{
    float lx = p->x;
    p->x += p->y * p->f;
    p->y -= lx * p->f;
    return p->y + 1.0;
}

////////////////////////////////////////////////////////////// synth

void ResetSynth(Synth *p)
{
    for (int i = 0; i < MaxPolyNum; i++)
    {
        ResetUnisonOsc(&p->osc[i]);
        ResetADSR(&p->oscadsr[i]);
        ResetADSR(&p->svfadsr[i]);
        ResetSVF(&p->svfl[i]);
        ResetSVF(&p->svfr[i]);
    }
    p->param.ratio1 = 0.5;
    p->param.form1 = 0.5;
    p->trigPos = 0;
    ResetLFO(&p->lfo1);
    ResetLFO(&p->lfo2);
    for (int i = 0; i < MaxLPCBufLen; ++i)
    {
        p->lpcbuf[i] = 0;
    }
}
void SynthProcMidi(Synth *p, midi_msg msg)
{
    if (msg.trig == 1) // note on
    {
        /*
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
        }*/
        p->trig[p->trigPos] = 1;
        p->note[p->trigPos] = msg.note;
        p->freq[p->trigPos] = msg.freq / 48000.0;
        p->vol[p->trigPos] = msg.vol;
        ResetUnisonOsc(&p->osc[p->trigPos]);
        ResetADSR(&p->oscadsr[p->trigPos]); // new note
        ResetADSR(&p->svfadsr[p->trigPos]);
        ADSRSetTrig(&p->oscadsr[p->trigPos], 1);
        ADSRSetTrig(&p->svfadsr[p->trigPos], 1);
        p->trigPos = (p->trigPos + 1) % MaxPolyNum;
    }
    else if (msg.trig == 0) // note off
    {
        for (int i = 0; i < MaxPolyNum; i++)
        {
            if (p->note[i] == msg.note)
            {
                ADSRSetTrig(&p->oscadsr[i], 0);
                ADSRSetTrig(&p->svfadsr[i], 0);
                p->trig[i] = 0;
                // p->note[i] = 0;
                // break;
            }
        }
    }
    else if (msg.trig == 2) // param
    {
        if (msg.channel == 0) // oscillator
        {
            p->param.form1 = msg.chanVal;
        }
        else if (msg.channel == 1)
        {
            p->param.ratio1 = msg.chanVal * 0.48 + 0.5;
        }
        else if (msg.channel == 3)
        {
            p->param.form2 = msg.chanVal;
        }
        else if (msg.channel == 4)
        {
            p->param.ratio2 = msg.chanVal * 0.48 + 0.5;
        }
        else if (msg.channel == 5)
        {
            p->param.pitch2 = powf(2.0, (msg.chanVal * 2.0 - 1.0) * 2.0);
        }
        else if (msg.channel == 6)
        {
            int lastmode = p->param.oscmode;
            p->param.oscmode = msg.chanVal * 2.99; // 0,1,2
            if (lastmode != p->param.oscmode)
            {
                if (p->param.oscmode == 0)
                {
                    uart_printf("\r\noscmode:sync");
                }
                else if (p->param.oscmode == 1)
                {
                    uart_printf("\r\noscmode:fm");
                }
                else if (p->param.oscmode == 2)
                {
                    uart_printf("\r\noscmode:am");
                }
            }
        }
        else if (msg.channel == 7)
        {
            p->param.oscdeep = msg.chanVal;
        }
        else if (msg.channel == 8)
        {
            p->param.oscmix = msg.chanVal;
        }
        else if (msg.channel == 9) // Unison&LFO
        {
            p->param.unison = (int)(msg.chanVal * (MaxUnisonNum - 1)) + 1;
        }
        else if (msg.channel == 10)
        {
            p->param.df = msg.chanVal * 0.01 + 1.0;
        }
        else if (msg.channel == 11)
        {
            p->param.dpan = msg.chanVal;
        }
        else if (msg.channel == 12) // LFO
        {
            p->param.lfo1f = (expf((msg.chanVal - 1.0) * 6.0) - expf(-6.0)) * 100.0 / SampleRate;
        }
        else if (msg.channel == 13)
        {
            p->param.lfo1amount = msg.chanVal * 0.5;
        }
        else if (msg.channel == 14)
        {
            int lastmode = p->param.lfo1mode;
            p->param.lfo1mode = msg.chanVal * 2.99;
            if (lastmode != p->param.lfo1mode)
            {
                if (p->param.lfo1mode == 0)
                {
                    uart_printf("\r\nlfo1mode:osc1Ratio");
                }
                else if (p->param.lfo1mode == 1)
                {
                    uart_printf("\r\nlfo1mode:ctof");
                }
                else if (p->param.lfo1mode == 2)
                {
                    uart_printf("\r\nlfo1mode:osc2Pitch");
                }
            }
        }
        else if (msg.channel == 15)
        {
            p->param.lfo2f = (expf((msg.chanVal - 1.0) * 6.0) - expf(-6.0)) * 100.0 / SampleRate;
        }
        else if (msg.channel == 16)
        {
            p->param.lfo2amount = msg.chanVal * 0.5;
        }
        else if (msg.channel == 17)
        {
            int lastmode = p->param.lfo2mode;
            p->param.lfo2mode = msg.chanVal * 2.99;
            if (lastmode != p->param.lfo2mode)
            {
                if (p->param.lfo2mode == 0)
                {
                    uart_printf("\r\nlfo2mode:osc2Ratio");
                }
                else if (p->param.lfo2mode == 1)
                {
                    uart_printf("\r\nlfo2mode:deep");
                }
                else if (p->param.lfo2mode == 2)
                {
                    uart_printf("\r\nlfo2mode:mix");
                }
            }
        }

        else if (msg.channel == 18) // ADSR
        {
            p->param.oa = expf((msg.chanVal - 1.0) * 16.0);
        }
        else if (msg.channel == 19)
        {
            p->param.od = expf((-msg.chanVal) * 16.0);
        }
        else if (msg.channel == 20)
        {
            p->param.os = msg.chanVal;
        }
        else if (msg.channel == 21)
        {
            p->param.or = 1.0 - expf((-msg.chanVal) * 12.0);
        }
        else if (msg.channel == 22)
        {
            p->param.famount = msg.chanVal;
        }
        else if (msg.channel == 23)
        {
            p->param.fa = expf((msg.chanVal - 1.0) * 16.0);
        }
        else if (msg.channel == 24)
        {
            p->param.fd = expf((-msg.chanVal) * 16.0);
        }
        else if (msg.channel == 25)
        {
            p->param.fs = msg.chanVal;
        }
        else if (msg.channel == 26)
        {
            p->param.fr = 1.0 - expf((-msg.chanVal) * 12.0);
        }
        else if (msg.channel == 27) // ctof
        {
            p->param.ctof = (expf((msg.chanVal - 1.0) * 6.0) - expf(-6.0)) / (1.0 - expf(-6.0));
        }
        else if (msg.channel == 28)
        {
            p->param.reso = 1.0 - expf(-msg.chanVal * 6.0);
        }

        // updata param
        for (int i = 0; i < MaxPolyNum; i++)
        {
            UnisonOscSetParam(&p->osc[i], p->param.unison, p->param.df);
            ADSRSetParam(&p->oscadsr[i], p->param.oa, p->param.od, p->param.os, p->param.or); // osc
            ADSRSetParam(&p->svfadsr[i], p->param.fa, p->param.fd, p->param.fs, p->param.fr); // filter
            LFOSetFreq(&p->lfo1, p->param.lfo1f);
            LFOSetFreq(&p->lfo2, p->param.lfo2f);
        }
    }
}

void SynthProcessBlock(Synth *p, float *recl, float *recr, float *bufl, float *bufr, int numSamples)
{
    for (int j = 0; j < MaxPolyNum; j++)
    {
        SVFCheckStability(&p->svfl[j], 10.0, 0.1); // 形同虚设
        SVFCheckStability(&p->svfr[j], 10.0, 0.1);
    }
    LFONormalize(&p->lfo1);
    LFONormalize(&p->lfo2);
    float osc1RatioAdd = 0, osc2RatioAdd = 0, osc2PitchAdd = 0, ctofAdd = 0, deepAdd = 0, mixAdd = 0;
    for (int i = 0; i < numSamples; i++)
    {
        float suml = 0, sumr = 0;
        float lfov1 = LFOProcSample(&p->lfo1) * p->param.lfo1amount;
        float lfov2 = LFOProcSample(&p->lfo2) * p->param.lfo2amount;
        osc1RatioAdd = 0, osc2RatioAdd = 0, osc2PitchAdd = 0, ctofAdd = 0, deepAdd = 0, mixAdd = 0;
        if (p->param.lfo1mode == 0)
        {
            osc1RatioAdd = lfov1;
        }
        else if (p->param.lfo1mode == 1)
        {
            ctofAdd = lfov1;
        }
        else if (p->param.lfo1mode == 2)
        {
            osc2PitchAdd = lfov1;
        }
        if (p->param.lfo2mode == 0)
        {
            osc2RatioAdd = lfov2;
        }
        else if (p->param.lfo2mode == 1)
        {
            deepAdd = lfov2;
        }
        else if (p->param.lfo2mode == 2)
        {
            mixAdd = lfov2;
        }

        for (int j = 0; j < MaxPolyNum; j++)
        {
            StereoSignal tmp =
                UnisonOscProcSample(&p->osc[j], p->freq[j],
                                    p->param.form1, p->param.ratio1 + osc1RatioAdd,
                                    p->param.form2, p->param.ratio2 + osc2RatioAdd, p->param.pitch2 + osc2PitchAdd,
                                    p->param.oscdeep + deepAdd, p->param.oscmix + mixAdd, p->param.oscmode);
            float adsrv = ADSRProcSample(&p->oscadsr[j]);
            float totvol = p->vol[j] * adsrv;

            float adsrctof = ADSRProcSample(&p->svfadsr[j]);
            float ctof = p->param.famount * adsrctof + p->param.ctof + ctofAdd;
            if (ctof > 2.5)
            {
                ctof = 2.5;
            }
            else if (ctof < 0)
            {
                ctof = 0;
            }
            tmp.l = SVFProcLPFOverSampling(&p->svfl[j], tmp.l, ctof, p->param.reso);
            tmp.r = SVFProcLPFOverSampling(&p->svfr[j], tmp.r, ctof, p->param.reso);
            suml += tmp.l * totvol;
            sumr += tmp.r * totvol;
        }

        float sum = suml + sumr;
        float outl1 = sum - p->param.dpan * (sum - suml);
        float outr1 = sum - p->param.dpan * (sum - sumr);
        float outl2 = outl1, outr2 = outr1;
        const float hpctof = 40.0 / 48000.0;
        for (int j = 0; j < 4; ++j)
        {
            outl2 = SVFProcLPF(&p->outhpl[j], outl2, hpctof, 0);
            outr2 = SVFProcLPF(&p->outhpr[j], outr2, hpctof, 0);
        }
        bufl[i] = outl1 - outl2;
        bufr[i] = outr1 - outr2;
    }

    // effects
    memcpy(&p->lpcbuf[0], &p->lpcbuf[numSamples], MaxLPCBufLen - numSamples);
    memcpy(&p->lpcbuf[MaxLPCBufLen - numSamples], &recl[0], numSamples);
    float errv = LPC_ProcDurbin(&p->lpc, p->lpcbuf, p->lpcCoeffs, MaxLPCBufLen, 16);
    LPC_FilterPredictStereo(&p->lpcfilt, bufl, bufr, bufl, bufr, p->lpcCoeffs, numSamples, 16, 30.0);
}
