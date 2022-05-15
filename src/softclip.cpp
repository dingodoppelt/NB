#include "plugin.hpp"


struct Softclip : Module {
	enum ParamId {
		GAIN_PARAM,
		GAINCVAMT_PARAM,
		HARDN_PARAM,
		HARDNCVAMT_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		GAINCVIN_INPUT,
		HARDNCVIN_INPUT,
		INPUT_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUTPUT_OUTPUT,
		MIXOUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	Softclip() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(GAIN_PARAM, -1.f, 1.f, 0.f, "gain");
		configParam(GAINCVAMT_PARAM, -1.f, 1.f, 0.f, "cv-gain amount");
		configParam(HARDN_PARAM, 0.f, 1.f, 0.f, "knee hardness");
		configParam(HARDNCVAMT_PARAM, 0.f, 1.f, 0.f, "cv-hardness amount");
		configInput(GAINCVIN_INPUT, "gain-cv input");
		configInput(HARDNCVIN_INPUT, "hardness-cv input");
		configInput(INPUT_INPUT, "input");
		configOutput(OUTPUT_OUTPUT, "output");
		configOutput(MIXOUT_OUTPUT, "mixoutput");
        outputs[MIXOUT_OUTPUT].setChannels(1);
	}
	
	float saturate(float inValue, float hardness) {
        hardness = hardness == 1.f ? 0.999 : hardness;
        float c = std::max(-1 * hardness, std::min(hardness, inValue));
        return c + tanh((inValue - c) / (1 - hardness)) * (1 - hardness);
    }

	void process(const ProcessArgs& args) override {
        if (inputs[INPUT_INPUT].isConnected() && (outputs[OUTPUT_OUTPUT].isConnected() || outputs[MIXOUT_OUTPUT].isConnected())) {
            float mix = 0.f;
            int channels = inputs[INPUT_INPUT].getChannels();
            outputs[OUTPUT_OUTPUT].setChannels(channels);
            float gain = powf(8, params[GAIN_PARAM].getValue());
            float hardness = params[HARDN_PARAM].getValue();
            if (inputs[GAINCVIN_INPUT].isConnected()) {
                gain *= params[GAINCVAMT_PARAM].getValue() * powf((inputs[GAINCVIN_INPUT].getVoltage() / 10), 4.f);
            }
            if (inputs[HARDNCVIN_INPUT].isConnected()) {
                hardness *= params[HARDNCVAMT_PARAM].getValue() * (inputs[HARDNCVIN_INPUT].getVoltage() / 10);
            }
            for (int i=0; i < channels; i++) {
                float in = inputs[INPUT_INPUT].getPolyVoltage(i);
                mix += gain * in / channels;
                outputs[OUTPUT_OUTPUT].setVoltage(10.f * saturate(gain * in, hardness), i);
            }
            outputs[MIXOUT_OUTPUT].setVoltage(10.f * saturate(mix, hardness));
        }
	}
};


struct SoftclipWidget : ModuleWidget {
	SoftclipWidget(Softclip* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/softclip.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(28.424, 26.295)), module, Softclip::GAIN_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(12.216, 37.112)), module, Softclip::GAINCVAMT_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(12.454, 68.919)), module, Softclip::HARDN_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(29.235, 78.332)), module, Softclip::HARDNCVAMT_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(26.274, 47.958)), module, Softclip::GAINCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(13.858, 88.448)), module, Softclip::HARDNCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.901, 107.067)), module, Softclip::INPUT_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.739, 107.067)), module, Softclip::OUTPUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.739, 120.067)), module, Softclip::MIXOUT_OUTPUT));
	}
};


Model* modelSoftclip = createModel<Softclip, SoftclipWidget>("softclip");
