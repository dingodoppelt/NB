#include "plugin.hpp"
#include "variableshapeosc.h"
#define MAX_POLYPHONY 4

using namespace daisysp;

struct VarTriSaw : Module {
	enum ParamId {
		GAINPARAM_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		VOCT_INPUT,
		AFTIN_INPUT,
		GAININ_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUTSAW_OUTPUT,
		OUTSQR_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	VariableShapeOscillator SQRosc[MAX_POLYPHONY], SAWosc[MAX_POLYPHONY];
	float aft_amt[MAX_POLYPHONY];
	float gain = 1.f;

	VarTriSaw() {
		float pi_halves = std::atan(1) * 2;
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(GAINPARAM_PARAM, 0.f, 1.f, 1.f, "gain");
		configInput(VOCT_INPUT, "v/oct");
		configInput(AFTIN_INPUT, "aftertouch");
		configInput(GAININ_INPUT, "poly gain in");
		configOutput(OUTSAW_OUTPUT, "pwm triSaw");
		configOutput(OUTSQR_OUTPUT, "pwm square");
		for (int i = 0; i < MAX_POLYPHONY; i++) {
			SAWosc[i].Init(APP->engine->getSampleRate());
			SQRosc[i].Init(APP->engine->getSampleRate());
			SAWosc[i].SetWaveshape(0);
			SQRosc[i].SetWaveshape(1);
			aft_amt[i] = sin(pi_halves * i / MAX_POLYPHONY); // atan(1) = pi / 4 so we get values between 0 and 1
		}
	}

	void process(const ProcessArgs& args) override {
		int AFTchannels = inputs[AFTIN_INPUT].getChannels();
		int OCTchannels = inputs[VOCT_INPUT].getChannels();
		int GAINchannels = inputs[VOCT_INPUT].getChannels();
		int channels = std::max(AFTchannels, OCTchannels);
		outputs[OUTSAW_OUTPUT].setChannels(channels);
		outputs[OUTSQR_OUTPUT].setChannels(channels);
		for(int i = 0; i < channels; i++) {
			float freq = dsp::FREQ_C4 * powf(2.f, inputs[VOCT_INPUT].getPolyVoltage(std::min(OCTchannels, i)));
			float pw = inputs[AFTIN_INPUT].getPolyVoltage(std::min(AFTchannels, i)) / 10.f;
			gain = inputs[GAININ_INPUT].isConnected() ? 11.f - powf(10.f - inputs[GAININ_INPUT].getPolyVoltage(std::min(GAINchannels, i) + 1.f), params[GAINPARAM_PARAM].getValue()) : 10.f;
			SAWosc[i].SetFreq(freq);
			SAWosc[i].SetPW((pw + 1.f) / 2.f);
			SQRosc[i].SetFreq(freq);
			SQRosc[i].SetPW( (pw * aft_amt[i] + 1.f) / 2.f );
			outputs[OUTSAW_OUTPUT].setVoltage(SAWosc[i].Process() * gain, i);
			outputs[OUTSQR_OUTPUT].setVoltage(SQRosc[i].Process() * gain, i);
		}
	}
};


struct VarTriSawWidget : ModuleWidget {
	VarTriSawWidget(VarTriSaw* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/VarTriSaw.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(25.176, 78.609)), module, VarTriSaw::GAINPARAM_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.345, 35.714)), module, VarTriSaw::VOCT_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.345, 57.383)), module, VarTriSaw::AFTIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.588, 78.609)), module, VarTriSaw::GAININ_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(9.939, 103.269)), module, VarTriSaw::OUTSAW_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.494, 103.108)), module, VarTriSaw::OUTSQR_OUTPUT));
	}
};


Model* modelVarTriSaw = createModel<VarTriSaw, VarTriSawWidget>("VarTriSaw");
