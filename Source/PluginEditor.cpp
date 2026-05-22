#include "PluginEditor.h"

MyCompressorAudioProcessorEditor::MyCompressorAudioProcessorEditor(MyCompressorAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(900, 420);
    auto& apvts = processor.getAPVTS();

    setupKnob(thresholdKnob,   "Threshold",    "threshold",    apvts);
    setupKnob(attackKnob,      "Attack",       "attack",       apvts);
    setupKnob(releaseKnob,     "Release",      "release",      apvts);
    setupKnob(attackCurveKnob, "Atk Curve",    "attackCurve",  apvts);
    setupKnob(releaseCurveKnob,"Rel Curve",    "releaseCurve", apvts);
    setupKnob(ratioKnob,       "Ratio",        "ratio",        apvts);
    setupKnob(makeupKnob,      "Makeup",       "makeup",       apvts);
    setupKnob(outputKnob,      "Output",       "output",       apvts);
    setupKnob(scHpfKnob,       "SC HPF",       "scHpf",        apvts);
    setupKnob(stereoLinkKnob,  "Link",         "stereoLink",   apvts);
    setupKnob(autoReleaseKnob, "Auto Rel",     "autoRelease",  apvts);

    msModeBox.addItemList(juce::StringArray{"L/R", "M/S"}, 1);
    msModeAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "msMode", msModeBox);
    addAndMakeVisible(msModeBox);

    hardClipBox.addItemList(juce::StringArray{"Off", "Post Mkup", "Pre Out"}, 1);
    hardClipAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "hardClip", hardClipBox);
    addAndMakeVisible(hardClipBox);

    addAndMakeVisible(vuMeter);

    startTimerHz(30);
}

MyCompressorAudioProcessorEditor::~MyCompressorAudioProcessorEditor() {}

void MyCompressorAudioProcessorEditor::setupKnob(Knob& knob, const juce::String& name,
                                                 const juce::String& paramId,
                                                 juce::AudioProcessorValueTreeState& apvts)
{
    knob.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    knob.slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    knob.label.setText(name, juce::dontSendNotification);
    knob.label.setJustificationType(juce::Justification::centred);
    knob.label.attachToComponent(&knob.slider, false);
    knob.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, paramId, knob.slider);
    addAndMakeVisible(knob.slider);
}

void MyCompressorAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xFF2B2B2B));
}

void MyCompressorAudioProcessorEditor::resized()
{
    auto b = getLocalBounds().reduced(20);
    auto top = b.removeFromTop(160);

    auto r1 = top.removeFromTop(80);
    thresholdKnob.slider.setBounds(r1.removeFromLeft(80));
    attackKnob.slider.setBounds(r1.removeFromLeft(80));
    releaseKnob.slider.setBounds(r1.removeFromLeft(80));
    attackCurveKnob.slider.setBounds(r1.removeFromLeft(80));
    releaseCurveKnob.slider.setBounds(r1.removeFromLeft(80));

    auto r2 = top.removeFromTop(80);
    ratioKnob.slider.setBounds(r2.removeFromLeft(80));
    makeupKnob.slider.setBounds(r2.removeFromLeft(80));
    outputKnob.slider.setBounds(r2.removeFromLeft(80));
    scHpfKnob.slider.setBounds(r2.removeFromLeft(80));
    stereoLinkKnob.slider.setBounds(r2.removeFromLeft(80));

    auto bot = b.removeFromTop(80);
    autoReleaseKnob.slider.setBounds(bot.removeFromLeft(80));
    msModeBox.setBounds(bot.removeFromLeft(90).reduced(0, 20));
    hardClipBox.setBounds(bot.removeFromLeft(90).reduced(0, 20));
    vuMeter.setBounds(bot.reduced(10, 0));
}

void MyCompressorAudioProcessorEditor::timerCallback()
{
    const auto& m = processor.getMeterData();
    vuMeter.grLeft  = m.grLeftDb;
    vuMeter.grRight = m.grRightDb;
    vuMeter.repaint();
}