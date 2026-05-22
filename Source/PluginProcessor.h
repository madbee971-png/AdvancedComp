#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "CompressorEngine.h"

class MyCompressorAudioProcessor : public juce::AudioProcessor
{
public:
    MyCompressorAudioProcessor();
    ~MyCompressorAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    const CompressorEngine::MeterData& getMeterData() const { return engine.getMeterData(); }

private:
    juce::AudioProcessorValueTreeState apvts;

    std::atomic<float>* thresholdParam   = nullptr;
    std::atomic<float>* attackParam      = nullptr;
    std::atomic<float>* releaseParam     = nullptr;
    std::atomic<float>* attackCurveParam = nullptr;
    std::atomic<float>* releaseCurveParam= nullptr;
    std::atomic<float>* ratioParam       = nullptr;
    std::atomic<float>* makeupParam      = nullptr;
    std::atomic<float>* outputParam      = nullptr;
    std::atomic<float>* scHpfParam       = nullptr;
    std::atomic<float>* stereoLinkParam = nullptr;
    std::atomic<float>* msModeParam      = nullptr;
    std::atomic<float>* hardClipParam    = nullptr;
    std::atomic<float>* autoReleaseParam = nullptr;

    CompressorEngine engine;

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MyCompressorAudioProcessor)
};