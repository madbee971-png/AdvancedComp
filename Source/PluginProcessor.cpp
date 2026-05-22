#include "PluginProcessor.h"
#include "PluginEditor.h"

MyCompressorAudioProcessor::MyCompressorAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    thresholdParam   = apvts.getRawParameterValue("threshold");
    attackParam      = apvts.getRawParameterValue("attack");
    releaseParam     = apvts.getRawParameterValue("release");
    attackCurveParam = apvts.getRawParameterValue("attackCurve");
    releaseCurveParam= apvts.getRawParameterValue("releaseCurve");
    ratioParam       = apvts.getRawParameterValue("ratio");
    makeupParam      = apvts.getRawParameterValue("makeup");
    outputParam      = apvts.getRawParameterValue("output");
    scHpfParam       = apvts.getRawParameterValue("scHpf");
    stereoLinkParam  = apvts.getRawParameterValue("stereoLink");
    msModeParam      = apvts.getRawParameterValue("msMode");
    hardClipParam    = apvts.getRawParameterValue("hardClip");
    autoReleaseParam = apvts.getRawParameterValue("autoRelease");
}

MyCompressorAudioProcessor::~MyCompressorAudioProcessor() {}

void MyCompressorAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    engine.prepare(sampleRate, samplesPerBlock, getTotalNumInputChannels());
}

void MyCompressorAudioProcessor::releaseResources() {}

void MyCompressorAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    CompressorEngine::Parameters params;
    params.thresholdDb  = thresholdParam->load();
    params.attackMs     = attackParam->load();
    params.releaseMs    = releaseParam->load();
    params.attackCurve  = attackCurveParam->load();
    params.releaseCurve = releaseCurveParam->load();
    params.ratio        = ratioParam->load();
    params.makeupDb     = makeupParam->load();
    params.outputDb     = outputParam->load();
    params.scHpfHz      = scHpfParam->load();
    params.stereoLink   = stereoLinkParam->load();
    params.msMode       = msModeParam->load() > 0.5f;
    params.hardClipMode = static_cast<int>(hardClipParam->load());
    params.autoRelease  = autoReleaseParam->load();

    engine.setParameters(params);
    engine.process(buffer);
}

juce::AudioProcessorEditor* MyCompressorAudioProcessor::createEditor()
{
    return new MyCompressorAudioProcessorEditor(*this);
}

bool MyCompressorAudioProcessor::hasEditor() const { return true; }

const juce::String MyCompressorAudioProcessor::getName() const { return "My Compressor"; }
bool MyCompressorAudioProcessor::acceptsMidi() const { return false; }
bool MyCompressorAudioProcessor::producesMidi() const { return false; }
bool MyCompressorAudioProcessor::isMidiEffect() const { return false; }
double MyCompressorAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int MyCompressorAudioProcessor::getNumPrograms() { return 1; }
int MyCompressorAudioProcessor::getCurrentProgram() { return 0; }
void MyCompressorAudioProcessor::setCurrentProgram(int) {}
const juce::String MyCompressorAudioProcessor::getProgramName(int) { return {}; }
void MyCompressorAudioProcessor::changeProgramName(int, const juce::String&) {}

void MyCompressorAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void MyCompressorAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout MyCompressorAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>("threshold",   "Threshold",    -60.0f, 0.0f,   -18.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("attack",      "Attack",       0.1f,   100.0f, 10.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("release",     "Release",      10.0f,  5000.0f,100.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("attackCurve", "Attack Curve", -1.0f,  1.0f,   0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("releaseCurve","Release Curve",-1.0f,  1.0f,   0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("ratio",       "Ratio",        1.0f,   20.0f,  4.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("makeup",      "Makeup Gain",  -12.0f, 24.0f,  0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("output",      "Output Gain",  -12.0f, 12.0f,  0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("scHpf",       "SC HPF",       20.0f,  1000.0f,20.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("stereoLink",  "Stereo Link",  0.0f,   100.0f, 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("msMode",      "Mode",         juce::StringArray{"L/R", "M/S"}, 0));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("hardClip",    "Hard Clip",    juce::StringArray{"Off", "Post Makeup", "Pre Output"}, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("autoRelease", "Auto Release", 0.0f,   1.0f,   0.0f));

    return { params.begin(), params.end() };
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MyCompressorAudioProcessor();
}