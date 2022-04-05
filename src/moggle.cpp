#include "plugin.hpp"


struct Moggle : Module {
	enum ParamId {
		VAL1_PARAM,
		VAL2_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		MORPHAMT_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		CVOUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	Moggle() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(VAL1_PARAM, -10.f, 10.f, 0.f, "set value 1");
		configParam(VAL2_PARAM, -10.f, 10.f, 0.f, "set value 2");
		configInput(MORPHAMT_INPUT, "morph amount cv input");
		configOutput(CVOUT_OUTPUT, "output");
	}

	void process(const ProcessArgs& args) override {
        if (outputs[CVOUT_OUTPUT].isConnected()) {
            outputs[CVOUT_OUTPUT].setVoltage(
                clamp(params[VAL1_PARAM].getValue() + (params[VAL2_PARAM].getValue() - params[VAL1_PARAM].getValue()) * (inputs[MORPHAMT_INPUT].getVoltage() / 10.f), -10.f, 10.f)
            );
        }
	}
};


struct MoggleWidget : ModuleWidget {
	MoggleWidget(Moggle* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Moggle.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.574, 36.218)), module, Moggle::VAL1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.574, 71.609)), module, Moggle::VAL2_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.074, 53.914)), module, Moggle::MORPHAMT_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.32, 102.37)), module, Moggle::CVOUT_OUTPUT));
	}
};


Model* modelMoggle = createModel<Moggle, MoggleWidget>("Moggle");
