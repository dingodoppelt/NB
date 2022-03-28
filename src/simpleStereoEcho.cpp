
/*
 * simple stereo echo module for Cardinal/VCVRack
 * Copyright (C) 2022 Nils Brederlow
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "plugin.hpp"
#include "delaybuffer.h"

struct SimpleStereoEcho : Module {
	enum ParamId {
		TIMEL_PARAM,
		TIMER_PARAM,
		FDBKL_PARAM,
		FDBKR_PARAM,
		MIX_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		MIXCV_INPUT,
		INPUTM_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUTL_OUTPUT,
		OUTR_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	DelayBuffer delayBufferL;
	DelayBuffer delayBufferR;
    float fdbkL, fdbkR, mix, in, left, right = 0.f;
    float SR = APP->engine->getSampleRate();

	SimpleStereoEcho() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(TIMEL_PARAM, 0.f, 1000.f, 0.f, "delay time L");
		configParam(TIMER_PARAM, 0.f, 1000.f, 0.f, "delay time R");
		configParam(FDBKL_PARAM, 0.f, 1.f, 0.f, "feedback L");
		configParam(FDBKR_PARAM, 0.f, 1.f, 0.f, "feedback R");
		configParam(MIX_PARAM, 0.f, 1.f, 0.f, "mix amount");
		configInput(MIXCV_INPUT, "mix cv-input");
		configInput(INPUTM_INPUT, "mono input signal");
		configOutput(OUTL_OUTPUT, "left output channel");
		configOutput(OUTR_OUTPUT, "right output channel");
		delayBufferL.setSize(SR * 5);
		delayBufferR.setSize(SR * 5);
	}

	void process(const ProcessArgs& args) override {
        if (outputs[OUTL_OUTPUT].isConnected() || outputs[OUTR_OUTPUT].isConnected()) {
            mix = inputs[MIXCV_INPUT].isConnected() ? (inputs[MIXCV_INPUT].getVoltage() / 10.f) : params[MIX_PARAM].getValue();
            in = inputs[INPUTM_INPUT].getVoltage();
            if (mix > 0) {
                delayBufferL.setDelay((long unsigned int)(params[TIMEL_PARAM].getValue() * (SR / 1000)) + 1);
                delayBufferR.setDelay((long unsigned int)(params[TIMER_PARAM].getValue() * (SR / 1000)) + 1);
                fdbkL = params[FDBKL_PARAM].getValue();
                fdbkR = params[FDBKR_PARAM].getValue();
                left = delayBufferL.get();
                right = delayBufferR.get();
                delayBufferL.put(in + (fdbkL * left));
                delayBufferR.put(in + (fdbkR * right));
            }
            outputs[OUTL_OUTPUT].setVoltage(in + (mix * left));
            outputs[OUTR_OUTPUT].setVoltage(in + (mix * right));
            delayBufferL.tick();
            delayBufferR.tick();
        }
	}
};



struct SimpleStereoEchoWidget : ModuleWidget {
	SimpleStereoEchoWidget(SimpleStereoEcho* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/simpleStereoEcho.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.627, 35.128)), module, SimpleStereoEcho::TIMEL_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(29.804, 35.128)), module, SimpleStereoEcho::TIMER_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.627, 53.514)), module, SimpleStereoEcho::FDBKL_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(29.804, 53.514)), module, SimpleStereoEcho::FDBKR_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(20.32, 66.558)), module, SimpleStereoEcho::MIX_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.32, 85.175)), module, SimpleStereoEcho::MIXCV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.738, 112.372)), module, SimpleStereoEcho::INPUTM_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.313, 112.372)), module, SimpleStereoEcho::OUTL_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.694, 112.372)), module, SimpleStereoEcho::OUTR_OUTPUT));
	}
};


Model* modelSimpleStereoEcho = createModel<SimpleStereoEcho, SimpleStereoEchoWidget>("simpleStereoEcho");
