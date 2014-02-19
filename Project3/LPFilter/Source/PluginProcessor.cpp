/*
==============================================================================

This file was auto-generated!

It contains the basic startup code for a Juce application.

==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#if WIN32
#define _USE_MATH_DEFINES
#include <math.h>
#endif


//==============================================================================
LpfilterAudioProcessor::LpfilterAudioProcessor() {
	filterState = kHiLoFilterStatePassthru;
	filterPosition = kHiLoFilterPositionMax / 2.0f;
	filterResonance = kHiLoFilterResonanceDefault;
	hiFilterLimit = kHiLoFilterRangeMax;
	loFilterLimit = kHiLoFilterRangeMin;
	deadZoneSize = kHiLoFilterDeadZoneDefault;
}

LpfilterAudioProcessor::~LpfilterAudioProcessor() {
}

//==============================================================================
const String LpfilterAudioProcessor::getName() const {
	return JucePlugin_Name;
}

void LpfilterAudioProcessor::resetLastIOData() {
	for (int i = 0; i < 2; i++) {
		lastInput1[i] = 0.0f;
		lastInput2[i] = 0.0f;
		lastInput3[i] = 0.0f;
		lastOutput1[i] = 0.0f;
		lastOutput2[i] = 0.0f;
	}
}

int LpfilterAudioProcessor::getNumParameters() {
	return kHiLoFilterNumParams;
}

static float scaleFrequencyToParameterRange(float value, float max, float min) {
	return (logf(value) - logf(min)) / (logf(max) - logf(min));
}

float LpfilterAudioProcessor::getParameter(int index) {
	switch (index) {
	case kHiLoFilterParamFilterPosition:
		return (filterPosition / kHiLoFilterPositionMax);
	case kHiLoFilterParamFilterResonance:
		return (filterResonance - kHiLoFilterResonanceMin) / (kHiLoFilterResonanceMax - kHiLoFilterResonanceMin);
	case kHiLoFilterParamHiFilterRange:
		return scaleFrequencyToParameterRange(hiFilterLimit, kHiLoFilterRangeMax, kHiLoFilterRangeMin);
	case kHiLoFilterParamLoFilterRange:
		return scaleFrequencyToParameterRange(loFilterLimit, kHiLoFilterRangeMax, kHiLoFilterRangeMin);
	case kHiLoFilterParamDeadZoneSize:
		return (deadZoneSize - kHiLoFilterDeadZoneMin) / (kHiLoFilterDeadZoneMax - kHiLoFilterDeadZoneMin);
	default:
		return 0.0f;
	}
}

static float scaleParameterRangeToFrequency(float value, float max, float min) {
	float frequency = expf(value * (logf(max) - logf(min)) + logf(min));
	if (frequency > max) {
		return max;
	}
	else if (frequency < min) {
		return min;
	}
	else {
		return frequency;
	}
}

float LpfilterAudioProcessor::getHiFilterCutoffPosition() {
	return (kHiLoFilterPositionMax + deadZoneSize) / 2.0f;
}

float LpfilterAudioProcessor::getLoFilterCutoffPosition() {
	return (kHiLoFilterPositionMax - deadZoneSize) / 2.0f;
}

void LpfilterAudioProcessor::setFilterState(float currentFilterPosition) {
	float loCutoff = (kHiLoFilterPositionMax - deadZoneSize) / 2.0f;
	float hiCutoff = (kHiLoFilterPositionMax + deadZoneSize) / 2.0f;
	HiLoFilterState newFilterState;
	if (currentFilterPosition > hiCutoff) {
		newFilterState = kHiLoFilterStateHi;
	}
	else if (currentFilterPosition < loCutoff) {
		newFilterState = kHiLoFilterStateLo;
	}
	else {
		newFilterState = kHiLoFilterStatePassthru;
	}

	if (newFilterState != filterState) {
		filterState = newFilterState;
		resetLastIOData();
		recalculateCoefficients();
	}
}

float LpfilterAudioProcessor::getFilterFrequency() {
	switch (filterState) {
	case kHiLoFilterStateHi: {
								 float relativeFilterPosition = (filterPosition - (kHiLoFilterPositionMax / 2.0f)) / getLoFilterCutoffPosition();
								 float newFrequency = scaleParameterRangeToFrequency(relativeFilterPosition, hiFilterLimit, kHiLoFilterRangeMin);
								 return newFrequency;
	}
	case kHiLoFilterStateLo: {
								 float relativeFilterPosition = filterPosition / getLoFilterCutoffPosition();
								 float newFrequency = scaleParameterRangeToFrequency(relativeFilterPosition, kHiLoFilterRangeMax, loFilterLimit);
								 return newFrequency;
	}
	default:
		return 0.0f;
	}
}

void LpfilterAudioProcessor::recalculateHiCoefficients(const double sampleRate, const float frequency, const float resonance) {
	const float coeffConstant = (float)tan(M_PI * frequency / sampleRate);
	hiCoeffA1 = 1.0f / ((1.0f + resonance * coeffConstant) + (coeffConstant * coeffConstant));
	hiCoeffA2 = -2.0f * hiCoeffA1;
	hiCoeffB1 = 2.0f * hiCoeffA1 * ((coeffConstant * coeffConstant) - 1.0f);
	hiCoeffB2 = hiCoeffA1 * (1.0f - (resonance * coeffConstant) + (coeffConstant * coeffConstant));
}

void LpfilterAudioProcessor::recalculateLoCoefficients(const double sampleRate, const float frequency, const float resonance) {
	const float coeffConstant = (float)(1.0f / tan(frequency / sampleRate));
	loCoeffA1 = 1.0f / (1.0f + (resonance * coeffConstant) + (coeffConstant * coeffConstant));
	loCoeffA2 = 2.0f * loCoeffA1;
	loCoeffB1 = 2.0f * loCoeffA1 * (1.0f - (coeffConstant * coeffConstant));
	loCoeffB2 = loCoeffA1 * (1.0f - (resonance * coeffConstant) + (coeffConstant * coeffConstant));
}

void LpfilterAudioProcessor::recalculateCoefficients() {
	switch (filterState) {
	case kHiLoFilterStateHi:
		recalculateHiCoefficients(getSampleRate(), getFilterFrequency(), filterResonance);
	case kHiLoFilterStateLo:
		recalculateLoCoefficients(getSampleRate(), getFilterFrequency(), filterResonance);
	default:
		break;
	}
}

void LpfilterAudioProcessor::setParameter(int index, float newValue) {
	switch (index) {
	case kHiLoFilterParamFilterPosition:
		filterPosition = kHiLoFilterPositionMax * newValue;
		setFilterState(filterPosition);
		break;
	case kHiLoFilterParamFilterResonance:
		filterResonance = newValue * (kHiLoFilterResonanceMax - kHiLoFilterResonanceMin) + kHiLoFilterResonanceMin;
		break;
	case kHiLoFilterParamHiFilterRange:
		hiFilterLimit = scaleParameterRangeToFrequency(newValue, kHiLoFilterRangeMax, kHiLoFilterRangeMin);
		break;
	case kHiLoFilterParamLoFilterRange:
		loFilterLimit = scaleParameterRangeToFrequency(newValue, kHiLoFilterRangeMax, kHiLoFilterRangeMin);
		break;
	case kHiLoFilterParamDeadZoneSize:
		deadZoneSize = newValue * (kHiLoFilterDeadZoneMax - kHiLoFilterDeadZoneMin) + kHiLoFilterDeadZoneMin;
		break;
	default: break;
	}

	if (filterState != kHiLoFilterStatePassthru) {
		recalculateCoefficients();
	}
}

const String LpfilterAudioProcessor::getParameterName(int index) {
	switch (index) {
	case kHiLoFilterParamFilterPosition: return String("Position");
	case kHiLoFilterParamFilterResonance: return String("Resonance");
	case kHiLoFilterParamHiFilterRange: return String("Hi Filter Limit");
	case kHiLoFilterParamLoFilterRange: return String("Lo Filter Limit");
	case kHiLoFilterParamDeadZoneSize: return String("Dead Zone Size");
	default: return String::empty;
	}
}

const String LpfilterAudioProcessor::getParameterNameForStorage(int index) {
	switch (index) {
	case kHiLoFilterParamFilterPosition: return String("Position");
	case kHiLoFilterParamFilterResonance: return String("Resonance");
	case kHiLoFilterParamHiFilterRange: return String("HiFilterLimit");
	case kHiLoFilterParamLoFilterRange: return String("LoFilterLimit");
	case kHiLoFilterParamDeadZoneSize: return String("DeadZoneSize");
	default: return String::empty;
	}
}

static const String getParameterTextForFrequency(const float frequency) {
	String outText;
	if (frequency > 1000) {
		outText = String(frequency / 1000.0f, PARAM_TEXT_NUM_DECIMAL_PLACES);
		outText.append(String(" kHz"), 4);
	}
	else {
		outText = String(frequency, PARAM_TEXT_NUM_DECIMAL_PLACES);
		outText.append(String(" Hz"), 3);
	}
	return outText;
}


const String LpfilterAudioProcessor::getParameterText(int index) {
	switch (index) {
	case kHiLoFilterParamFilterPosition:
		return String((int)filterPosition);
	case kHiLoFilterParamFilterResonance:
		return String(filterResonance, PARAM_TEXT_NUM_DECIMAL_PLACES);
	case kHiLoFilterParamHiFilterRange:
		return getParameterTextForFrequency(hiFilterLimit);
	case kHiLoFilterParamLoFilterRange:
		return getParameterTextForFrequency(loFilterLimit);
	case kHiLoFilterParamDeadZoneSize:
		return String((int)deadZoneSize);
	default:
		return String::empty;
	}
}

const String LpfilterAudioProcessor::getInputChannelName(int channelIndex) const {
	return String(channelIndex + 1);
}

const String LpfilterAudioProcessor::getOutputChannelName(int channelIndex) const {
	return String(channelIndex + 1);
}

bool LpfilterAudioProcessor::isInputChannelStereoPair(int index) const {
	return true;
}

bool LpfilterAudioProcessor::isOutputChannelStereoPair(int index) const {
	return true;
}

bool LpfilterAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
	return true;
#else
	return false;
#endif
}

bool LpfilterAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
	return true;
#else
	return false;
#endif
}

bool LpfilterAudioProcessor::silenceInProducesSilenceOut() const {
	return false;
}

double LpfilterAudioProcessor::getTailLengthSeconds() const {
	return 0.0;
}

int LpfilterAudioProcessor::getNumPrograms() {
	return 0;
}

int LpfilterAudioProcessor::getCurrentProgram() {
	return 0;
}

void LpfilterAudioProcessor::setCurrentProgram(int index) {
}

const String LpfilterAudioProcessor::getProgramName(int index) {
	return String::empty;
}

void LpfilterAudioProcessor::changeProgramName(int index, const String& newName) {
}

//==============================================================================
void LpfilterAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
	for (int i = 0; i < 2; i++) {
		lastInput1[i] = 0.0f;
		lastInput2[i] = 0.0f;
		lastInput3[i] = 0.0f;
		lastOutput1[i] = 0.0f;
		lastOutput2[i] = 0.0f;
	}

	resetLastIOData();
	recalculateCoefficients();
}

void LpfilterAudioProcessor::releaseResources() {
	// When playback stops, you can use this as an opportunity to free up any
	// spare memory, etc.
}

void LpfilterAudioProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages) {
	for (int channel = 0; channel < getNumInputChannels(); ++channel) {
		switch (filterState) {
		case kHiLoFilterStateHi:
			processHiFilter(buffer.getSampleData(channel), channel, buffer.getNumSamples());
			break;
		case kHiLoFilterStateLo:
			processLoFilter(buffer.getSampleData(channel), channel, buffer.getNumSamples());
			break;
		case kHiLoFilterStatePassthru:
		default:
			break;
		}
	}

	// In case we have more outputs than inputs, we'll clear any output
	// channels that didn't contain input data, (because these aren't
	// guaranteed to be empty - they may contain garbage).
	for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i) {
		buffer.clear(i, 0, buffer.getNumSamples());
	}
}

void LpfilterAudioProcessor::processHiFilter(float *channelData, const int channel, const int numFrames) {
	for (int i = 0; i < numFrames; ++i) {
		lastInput3[channel] = lastInput2[channel];
		lastInput2[channel] = lastInput1[channel];
		lastInput1[channel] = channelData[i];

		channelData[i] = (hiCoeffA1 * lastInput1[channel]) +
			(hiCoeffA2 * lastInput2[channel]) +
			(hiCoeffA1 * lastInput3[channel]) -
			(hiCoeffB1 * lastOutput1[channel]) -
			(hiCoeffB2 * lastOutput2[channel]);

		lastOutput2[channel] = lastOutput1[channel];
		lastOutput1[channel] = channelData[i];
	}
}

void LpfilterAudioProcessor::processLoFilter(float *channelData, const int channel, const int numFrames) {
	for (int i = 0; i < numFrames; ++i) {
		lastInput3[channel] = lastInput2[channel];
		lastInput2[channel] = lastInput1[channel];
		lastInput1[channel] = channelData[i];

		channelData[i] = (loCoeffA1 * lastInput1[channel]) +
			(loCoeffA2 * lastInput2[channel]) +
			(loCoeffA1 * lastInput3[channel]) -
			(loCoeffB1 * lastOutput1[channel]) -
			(loCoeffB2 * lastOutput2[channel]);

		lastOutput2[channel] = lastOutput1[channel];
		lastOutput1[channel] = channelData[i];
	}
}

//==============================================================================
bool LpfilterAudioProcessor::hasEditor() const {
	return false; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor *LpfilterAudioProcessor::createEditor() {
	return new LpfilterAudioProcessorEditor(this);
}

//==============================================================================
void LpfilterAudioProcessor::getStateInformation(MemoryBlock& destData) {
	// You should use this method to store your parameters in the memory block.
	// You could do that either as raw data, or use the XML or ValueTree classes
	// as intermediaries to make it easy to save and load complex data.
	XmlElement xml("HiLoFilterStorage");
	for (int i = 0; i < kHiLoFilterNumParams; i++) {
		xml.setAttribute(getParameterNameForStorage(i), getParameter(i));
	}
	copyXmlToBinary(xml, destData);
}

void LpfilterAudioProcessor::setStateInformation(const void *data, int sizeInBytes) {
	// You should use this method to restore your parameters from this memory block,
	// whose contents will have been created by the getStateInformation() call.
	ScopedPointer<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
	if (xmlState != 0 && xmlState->hasTagName("HiLoFilterStorage")) {
		for (int i = 0; i < kHiLoFilterNumParams; i++) {
			setParameter(i, xmlState->getDoubleAttribute(getParameterNameForStorage(i)));
		}
	}
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
	return new LpfilterAudioProcessor();
}