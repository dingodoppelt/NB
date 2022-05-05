#include "plugin.hpp"


struct Polyfotz : Module {
	enum ParamId {
		TRANSP_PARAM,
		TUNE_PARAM,
		DETUNE_PARAM,
		OCT_SEL_CV_PARAM,
		AFT_RAND_PARAM,
		VOICING_SEL_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		CVIN_INPUT,
		OCTCVIN_INPUT,
		AFTCV_INPUT,
		PW_CV_INPUT,
		VOICINGCV_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		AFT_OUT_OUTPUT,
		POLY_OUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	float freq = 440.f;
	float tune, aftertouch, bendfactor, transp, spread, pitchwheel = 0.f;
	float bendrange = 2.f;
	float octave = 1.f;
	int voicing_select = 0;
    float aft_amt[4] = { 0.2f, -0.4f, 0.6f, -0.9f };
    float detune_amt[4] = { 0.f, -0.1f / 12.f, 0.1f / 12.f, -0.2f / 12.f };
	int voicing[10][4] = { {0, -5, -10, -12},
							{0, -5, -10, -20},
							{0, -3, -8, -19},
							{0, -3, -7, -10},
							{0, -4, -9, -11},
							{0, -3, -8, -11},
							{0, -5, -7, -11},
							{0, -5, -11, -15},
							{0, -3, -6, -9},
							{0, -4, -8, -12} };

	Polyfotz() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(TRANSP_PARAM, -6.f, 6.f, 0.f, "transpose by semitones");
		configParam(TUNE_PARAM, -0.1f, 0.1f, 0.f, "fine tune");
		configParam(DETUNE_PARAM, 0.f, 1.f, 0.f, "detune spread");
		configParam(OCT_SEL_CV_PARAM, -2.f, 2.f, 0.f, "transpose by octave");
		configParam(VOICING_SEL_PARAM, 0.f, 9.f, 0.f, "voicing selection");
		configParam(AFT_RAND_PARAM, -1.f, 1.f, 0.f, "aftertouch randomization");
		configInput(CVIN_INPUT, "V/OCT");
		configInput(OCTCVIN_INPUT, "toggle transpose by octave");
		configInput(PW_CV_INPUT, "pitchwheel");
		configInput(VOICINGCV_INPUT, "voicing selection cv");
		configInput(AFTCV_INPUT, "aftertouch");
		configOutput(POLY_OUT_OUTPUT, "polyphonic");
		configOutput(AFT_OUT_OUTPUT, "polyphonic aftertouch");
	}

	void process(const ProcessArgs& args) override {
		tune = params[TUNE_PARAM].getValue();
		aftertouch = inputs[AFTCV_INPUT].isConnected() ? inputs[AFTCV_INPUT].getVoltage() / 2.f : params[AFT_RAND_PARAM].getValue();
		octave = inputs[OCTCVIN_INPUT].isConnected() ? params[OCT_SEL_CV_PARAM].getValue() * (int)(inputs[OCTCVIN_INPUT].getVoltage() / 10.f) : params[OCT_SEL_CV_PARAM].getValue();
        pitchwheel = inputs[PW_CV_INPUT].getVoltage() / 5.f;
		bendfactor = (pitchwheel * bendrange) / 12.f;
		transp = params[TRANSP_PARAM].getValue() / 12.f;
		spread = params[DETUNE_PARAM].getValue() / .9f + .1f;
		freq = inputs[CVIN_INPUT].getVoltage() + octave + transp + tune;
		voicing_select = inputs[VOICINGCV_INPUT].isConnected() ? ((int)abs(inputs[VOICINGCV_INPUT].getVoltage()) % (int)paramQuantities[VOICING_SEL_PARAM]->getMaxValue()) : (int)(params[VOICING_SEL_PARAM].getValue());
        outputs[POLY_OUT_OUTPUT].setChannels(4);
        outputs[AFT_OUT_OUTPUT].setChannels(4);
		for (int i = 0; i < 4; i++) {
			outputs[POLY_OUT_OUTPUT].setVoltage(freq + (detune_amt[i] * spread) + (bendfactor < 0.f ? (float)voicing[voicing_select][i] / -12.f * pitchwheel : bendfactor), i);
            outputs[AFT_OUT_OUTPUT].setVoltage(aftertouch * aft_amt[i], i);
        }
	}
};


struct PolyfotzWidget : ModuleWidget {
	PolyfotzWidget(Polyfotz* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/polyfotz.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackSnapKnob>(mm2px(Vec(26.295, 31.257)), module, Polyfotz::TRANSP_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(34.234, 42.337)), module, Polyfotz::TUNE_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(33.407, 55.733)), module, Polyfotz::DETUNE_PARAM));
		addParam(createParamCentered<RoundBlackSnapKnob>(mm2px(Vec(22.285, 56.891)), module, Polyfotz::OCT_SEL_CV_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(21.166, 73.538)), module, Polyfotz::AFT_RAND_PARAM));
		addParam(createParamCentered<RoundBlackSnapKnob>(mm2px(Vec(24.616, 93.301)), module, Polyfotz::VOICING_SEL_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.832, 39.774)), module, Polyfotz::CVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.352, 56.188)), module, Polyfotz::OCTCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.004, 73.479)), module, Polyfotz::AFTCV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.777, 87.512)), module, Polyfotz::PW_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.712, 101.311)), module, Polyfotz::VOICINGCV_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.744, 73.538)), module, Polyfotz::AFT_OUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.829, 112.63)), module, Polyfotz::POLY_OUT_OUTPUT));
	}
};


Model* modelPolyfotz = createModel<Polyfotz, PolyfotzWidget>("Polyfotz");
