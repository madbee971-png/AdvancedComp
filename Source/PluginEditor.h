#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class MyCompressorAudioProcessorEditor : public juce::AudioProcessorEditor,
                                         private juce::Timer
{
public:
    MyCompressorAudioProcessorEditor(MyCompressorAudioProcessor&);
    ~MyCompressorAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    MyCompressorAudioProcessor& processor;

    struct Knob
    {
        juce::Slider slider;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
        juce::Label label;
    };

    Knob thresholdKnob, attackKnob, releaseKnob, attackCurveKnob, releaseCurveKnob;
    Knob ratioKnob, makeupKnob, outputKnob, scHpfKnob, stereoLinkKnob, autoReleaseKnob;

    juce::ComboBox msModeBox, hardClipBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> msModeAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> hardClipAttach;

    struct VUMeter : juce::Component
    {
        float grLeft = 0.0f, grRight = 0.0f;

        void paint(juce::Graphics& g) override
        {
            auto b = getLocalBounds().toFloat();
            float w = b.getWidth() * 0.45f;
            float h = b.getHeight();

            g.setColour(juce::Colours::black);
            g.fillRect(b);

            // Barres GR (0 en haut, -30 en bas)
            float hL = juce::jmap(juce::jlimit(-30.0f, 0.0f, grLeft),  -30.0f, 0.0f, h, 0.0f);
            float hR = juce::jmap(juce::jlimit(-30.0f, 0.0f, grRight), -30.0f, 0.0f, h, 0.0f);

            g.setColour(juce::Colours::green);
            g.fillRect(b.getX(), b.getY() + h - hL, w, hL);
            g.fillRect(b.getX() + w + b.getWidth()*0.1f, b.getY() + h - hR, w, hR);

            g.setColour(juce::Colours::white);
            g.setFont(12.0f);
            g.drawText(juce::String(grLeft, 1) + " dB", b.getX(), 2, (int)w, 14, juce::Justification::centred);
            g.drawText(juce::String(grRight, 1) + " dB", b.getX() + (int)w + (int)(b.getWidth()*0.1f), 2, (int)w, 14, juce::Justification::centred);
        }
    };

    VUMeter vuMeter;

    void setupKnob(Knob& knob, const juce::String& name, const juce::String& paramId,
                   juce::AudioProcessorValueTreeState& apvts);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MyCompressorAudioProcessorEditor)
};