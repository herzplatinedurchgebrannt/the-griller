/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GainAndDspAudioProcessor::GainAndDspAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
                    valueTree(*this, nullptr, "Parameters", createParameters()),
                    lowPassFilter(juce::dsp::IIR::Coefficients<float>::makeLowPass(44100, 20000.0f, 0.1f))
#endif
{
    //state = new juce::AudioProcessorValueTreeState(*this, nullptr);
    //state->createAndAddParameter("drive", "Drive", "Drive", juce::NormalisableRange<float>(0.f, 1.f, 0.0001), 1.0, nullptr, nullptr);
    //state->createAndAddParameter("range", "Range", "Range", juce::NormalisableRange<float>(0.f, 3000.f, 0.0001), 1.0, nullptr, nullptr);
    //state->createAndAddParameter("blend", "Blend", "Blend", juce::NormalisableRange<float>(0.f, 1.f, 0.0001), 1.0, nullptr, nullptr);
    //state->createAndAddParameter("volume", "Volume", "Volume", juce::NormalisableRange<float>(0.f, 3.f, 0.0001), 1.0, nullptr, nullptr);

    //state->state = juce::ValueTree("drive");
    //state->state = juce::ValueTree("range");
    //state->state = juce::ValueTree("blend");
    //state->state = juce::ValueTree("volume");
}

GainAndDspAudioProcessor::~GainAndDspAudioProcessor()
{
}

//==============================================================================
const juce::String GainAndDspAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool GainAndDspAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool GainAndDspAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool GainAndDspAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double GainAndDspAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int GainAndDspAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int GainAndDspAudioProcessor::getCurrentProgram()
{
    return 0;
}

void GainAndDspAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String GainAndDspAudioProcessor::getProgramName (int index)
{
    return {};
}

void GainAndDspAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void GainAndDspAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    lastSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = lastSampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getMainBusNumOutputChannels();

    lowPassFilter.reset();
    lowPassFilter.prepare(spec);
}

void GainAndDspAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void GainAndDspAudioProcessor::updateFilter()
{
    auto filterCutoffValue = valueTree.getRawParameterValue("FILTER_CUTOFF");
    float filterCutoff = filterCutoffValue->load();

    auto filterResValue = valueTree.getRawParameterValue("FILTER_RES");
    float filterRes = filterResValue->load();

    *lowPassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(lastSampleRate, filterCutoff, filterRes);

}

#ifndef JucePlugin_PreferredChannelConfigurations
bool GainAndDspAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void GainAndDspAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();



    auto driveValue = valueTree.getRawParameterValue("GAIN_DRIVE");
    float drive = driveValue->load();

    auto rangeValue = valueTree.getRawParameterValue("GAIN_RANGE");
    float range = rangeValue->load();

    auto blendValue = valueTree.getRawParameterValue("GAIN_BLEND");
    float blend = blendValue->load();

    auto volumeValue = valueTree.getRawParameterValue("GAIN_VOLUME");
    float volume = volumeValue->load();
    
    std::cout << "drive" << std::endl;



    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());



    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); sample++) {

            float cleanSig = *channelData;

            *channelData *= drive * range;

            *channelData = (((((2.f / juce::float_Pi) * atan(*channelData)) * blend) + (cleanSig * (1.f - blend))) / 2.f) * volume;
            channelData++;
        }
    }

    juce::dsp::AudioBlock<float> block(buffer);
    updateFilter();
    lowPassFilter.process(juce::dsp::ProcessContextReplacing<float>(block));
}

juce::AudioProcessorValueTreeState& GainAndDspAudioProcessor::getState()
{
    return *state;
};

//==============================================================================
bool GainAndDspAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* GainAndDspAudioProcessor::createEditor()
{
    return new GainAndDspAudioProcessorEditor (*this);
}

//==============================================================================
void GainAndDspAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void GainAndDspAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GainAndDspAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout GainAndDspAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTER_CUTOFF", "FilterCutoff", 20.0f, 20000.0f, 600.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTER_RES", "FilterRes", 0.1f, 1.0f, 0.1f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>("GAIN_DRIVE", "GainDrive", 0.f, 1.f, 0.0001));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("GAIN_RANGE", "GainRange", 0.f, 3000.f, 0.0001));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("GAIN_BLEND", "GainBlend", 0.f, 1.f, 0.0001));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("GAIN_VOLUME", "GainVOlume", 0.f, 3.f, 0.0001));

    return { params.begin(), params.end() };
}