#pragma once
#include <juce_dsp/juce_dsp.h>

class CompressorEngine
{
public:
    struct Parameters
    {
        float thresholdDb   = -18.0f;
        float attackMs      = 10.0f;
        float releaseMs     = 100.0f;
        float attackCurve   = 0.0f;   // -1 = log concave, 0 = linéaire, 1 = log convexe
        float releaseCurve  = 0.0f;
        float ratio         = 4.0f;
        float makeupDb      = 0.0f;
        float outputDb      = 0.0f;
        float scHpfHz       = 20.0f;
        float stereoLink    = 100.0f; // 0..100 %
        bool  msMode        = false;
        int   hardClipMode  = 0;      // 0=Off, 1=Post Makeup, 2=Pre Output
        float autoRelease   = 0.0f;   // 0..1
    };

    struct MeterData
    {
        float grLeftDb  = 0.0f;
        float grRightDb = 0.0f;
    };

    CompressorEngine();

    void prepare(double sampleRate, int maxBlockSize, int numChannels);
    void setParameters(const Parameters& params);
    void process(juce::AudioBuffer<float>& buffer);

    const MeterData& getMeterData() const { return meterData; }

private:
    struct ChannelState
    {
        float currentGainDb = 0.0f;
        juce::dsp::IIR::Filter<float> hpf;
    };

    double sr = 44100.0;
    Parameters currentParams;
    std::vector<<ChannelState> channels;
    MeterData meterData;

    float computeGainReduction(float inputDb) const;
    float smoothGain(float targetDb, float attackMs, float releaseMs,
                     float curve, float autoReleaseAmt,
                     float currentDb, float grDb) const;
    void applyHardClip(float& sample, float threshold) const;
};