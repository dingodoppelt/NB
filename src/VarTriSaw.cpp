#include "plugin.hpp"
#include "variableshapeosc.h"
#define MAX_POLYPHONY 4

using namespace daisysp;

struct VarTriSaw : Module {
	enum ParamId {
		PARAMS_LEN
	};
	enum InputId {
		VOCT_INPUT,
		AFTIN_INPUT,
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

	VarTriSaw() {
		float pi_halves = std::atan(1) * 2;
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configInput(VOCT_INPUT, "v/oct");
		configInput(AFTIN_INPUT, "aftertouch");
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
		int channels = std::max(AFTchannels, OCTchannels);
		outputs[OUTSAW_OUTPUT].setChannels(channels);
		outputs[OUTSQR_OUTPUT].setChannels(channels);
		for(int i = 0; i < channels; i++) {
			float freq = dsp::FREQ_C4 * powf(2.f, inputs[VOCT_INPUT].getPolyVoltage(std::min(OCTchannels, i)));
			float pw = inputs[AFTIN_INPUT].getPolyVoltage(std::min(AFTchannels, i)) / 10.f;
			SAWosc[i].SetFreq(freq);
			SAWosc[i].SetPW((pw + 1.f) / 2.f);
			SQRosc[i].SetFreq(freq);
			SQRosc[i].SetPW( (pw * aft_amt[i] + 1.f) / 2.f );
			outputs[OUTSAW_OUTPUT].setVoltage(SAWosc[i].Process() * 10.f, i);
			outputs[OUTSQR_OUTPUT].setVoltage(SQRosc[i].Process() * 10.f, i);
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

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.32, 35.391)), module, VarTriSaw::VOCT_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.32, 64.663)), module, VarTriSaw::AFTIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(9.914, 101.817)), module, VarTriSaw::OUTSAW_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.469, 101.656)), module, VarTriSaw::OUTSQR_OUTPUT));
	}
};


Model* modelVarTriSaw = createModel<VarTriSaw, VarTriSawWidget>("VarTriSaw");
