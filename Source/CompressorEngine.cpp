#include "CompressorEngine.h"

CompressorEngine::CompressorEngine() {}

void CompressorEngine::prepare(double sampleRate, int /*maxBlockSize*/, int numChannels)
{
    sr = sampleRate;
    channels.resize(static_cast<size_t>(numChannels));

    for (auto& ch : channels)
    {
        ch.currentGainDb = 0.0f;
        ch.hpf.reset();
        auto coeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 20.0f);
        ch.hpf.coefficients = coeffs;
    }
    meterData = {};
}

void CompressorEngine::setParameters(const Parameters& params)
{
    currentParams = params;

    for (auto& ch : channels)
    {
        if (params.scHpfHz > 20.0f)
        {
            auto coeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(static_cast<double>(sr), params.scHpfHz);
            ch.hpf.coefficients = coeffs;
        }
    }
}

void CompressorEngine::process(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();
    if (numChannels < 2) return;

    float* left  = buffer.getWritePointer(0);
    float* right = buffer.getWritePointer(1);

    // --- Encodage M/S si actif ---
    if (currentParams.msMode)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            float m = (left[i] + right[i]) * 0.5f;
            float s = (left[i] - right[i]) * 0.5f;
            left[i]  = m;
            right[i] = s;
        }
    }

    const float linkFactor = currentParams.stereoLink / 100.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        float inL = left[i];
        float inR = right[i];

        // --- Détection sidechain ---
        float detL = std::abs(inL);
        float detR = std::abs(inR);

        // HPF sur sidechain
        if (currentParams.scHpfHz > 20.0f)
        {
            detL = channels[0].hpf.processSample(detL);
            detR = channels[1].hpf.processSample(detR);
        }

        // Lien stéréo de la détection
        float detLinked = (detL + detR) * 0.5f;
        float scL = detL * (1.0f - linkFactor) + detLinked * linkFactor;
        float scR = detR * (1.0f - linkFactor) + detLinked * linkFactor;

        float inputDbL = 20.0f * std::log10(scL + 1e-10f);
        float inputDbR = 20.0f * std::log10(scR + 1e-10f);

        float targetL = computeGainReduction(inputDbL);
        float targetR = computeGainReduction(inputDbR);

        float grL = targetL; // valeur négative = réduction
        float grR = targetR;

        // --- Smoothing avec courbes & auto-release ---
        channels[0].currentGainDb = smoothGain(targetL, currentParams.attackMs, currentParams.releaseMs,
                                               currentParams.attackCurve, currentParams.autoRelease,
                                               channels[0].currentGainDb, std::abs(grL));

        channels[1].currentGainDb = smoothGain(targetR, currentParams.attackMs, currentParams.releaseMs,
                                               currentParams.attackCurve, currentParams.autoRelease,
                                               channels[1].currentGainDb, std::abs(grR));

        float gainLinL = std::pow(10.0f, channels[0].currentGainDb * 0.05f);
        float gainLinR = std::pow(10.0f, channels[1].currentGainDb * 0.05f);

        float outL = inL * gainLinL;
        float outR = inR * gainLinR;

        // --- Hard Clip Pre Output (sur signal compressé brut) ---
        if (currentParams.hardClipMode == 2)
        {
            applyHardClip(outL, 1.0f);
            applyHardClip(outR, 1.0f);
        }

        // --- Makeup Gain ---
        float makeupLin = std::pow(10.0f, currentParams.makeupDb * 0.05f);
        outL *= makeupLin;
        outR *= makeupLin;

        // --- Hard Clip Post Makeup ---
        if (currentParams.hardClipMode == 1)
        {
            applyHardClip(outL, 1.0f);
            applyHardClip(outR, 1.0f);
        }

        // --- Output Gain ---
        float outLin = std::pow(10.0f, currentParams.outputDb * 0.05f);
        outL *= outLin;
        outR *= outLin;

        left[i]  = outL;
        right[i] = outR;
    }

    // --- Mètres GR ---
    meterData.grLeftDb  = channels[0].currentGainDb;
    meterData.grRightDb = channels[1].currentGainDb;

    // --- Décodage M/S ---
    if (currentParams.msMode)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            float m = left[i];
            float s = right[i];
            left[i]  = m + s;
            right[i] = m - s;
        }
    }
}

float CompressorEngine::computeGainReduction(float inputDb) const
{
    float t = currentParams.thresholdDb;
    float r = currentParams.ratio;
    if (inputDb < t || r <= 1.0f) return 0.0f;

    float over = inputDb - t;
    float reduced = over / r;
    return reduced - over; // négatif = réduction
}

float CompressorEngine::smoothGain(float targetDb, float attackMs, float releaseMs,
                                 float curve, float autoReleaseAmt,
                                 float currentDb, float grDb) const
{
    float error = targetDb - currentDb;
    bool isAttack = error < 0.0f; // on compresse davantage (gain descend)

    float timeMs = isAttack ? attackMs : releaseMs;

    // --- Auto-Release : time = Release / (1 + amount * GR_dB) ---
    if (!isAttack && autoReleaseAmt > 0.0f && grDb > 0.1f)
    {
        timeMs = timeMs / (1.0f + autoReleaseAmt * grDb);
        timeMs = juce::jmax(0.5f, timeMs);
    }

    if (std::abs(curve) < 0.05f)
    {
        // --- Linéaire : Slew Limiter ---
        float samples = timeMs * 0.001f * static_cast<float>(sr);
        float maxDelta = (std::abs(error) > 0.001f) ? (1.0f / samples) : 0.0f;
        float delta = juce::jlimit(-maxDelta, maxDelta, error);
        return currentDb + delta;
    }
    else
    {
        // --- Exponentiel modulé par la courbe ---
        float baseCoef = 1.0f - std::exp(-1.0f / (static_cast<float>(sr) * timeMs * 0.001f));
        float absErr   = std::abs(error);

        if (curve < 0.0f)
        {
            // Concave (log) : ralentit en approchant de la cible
            // shape > 1 quand erreur est petite
            float shape = 1.0f + std::abs(curve) * 3.0f * (1.0f - std::exp(-absErr * 2.0f));
            float coef  = baseCoef / shape;
            return currentDb + error * coef;
        }
        else
        {
            // Convexe : démarre lent, accélère à la fin
            // shape < 1 quand erreur est grande
            float shape = 1.0f / (1.0f + curve * 3.0f * (1.0f - std::exp(-absErr * 2.0f)));
            float coef  = baseCoef * shape;
            return currentDb + error * coef;
        }
    }
}

void CompressorEngine::applyHardClip(float& sample, float threshold) const
{
    sample = juce::jlimit(-threshold, threshold, sample);
}