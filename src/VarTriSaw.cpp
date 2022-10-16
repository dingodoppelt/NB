#include "plugin.hpp"
#include "variableshapeosc.h"

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

	VariableShapeOscillator SQRosc[4], SAWosc[4];
	float aft_amt[4] = { .6f, .3f, .7f, .8f };

	VarTriSaw() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configInput(VOCT_INPUT, "v/oct");
		configInput(AFTIN_INPUT, "aftertouch");
		configOutput(OUTSAW_OUTPUT, "pwm triSaw");
		configOutput(OUTSQR_OUTPUT, "pwm square");
		for (int i = 0; i < 4; i++) {
			SAWosc[i].Init(APP->engine->getSampleRate());
			SQRosc[i].Init(APP->engine->getSampleRate());
			SAWosc[i].SetWaveshape(0);
			SQRosc[i].SetWaveshape(1);
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
			SAWosc[i].SetPW(pw);
			SQRosc[i].SetFreq(freq);
			SQRosc[i].SetPW(pw * aft_amt[i % 4]);
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
