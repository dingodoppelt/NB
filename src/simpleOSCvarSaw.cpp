
/*
 * simple variable saw oscillator Cardinal/VCVRack module for use with an AKAI EWI breath controller
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
#include "variablesawosc.h"

struct SimpleOSCvarSaw : Module {
	enum ParamId {
		TRANSP_PARAM,
		VOICINGSEL_PARAM,
		OCTAVE_PARAM,
		TUNE_PARAM,
		UNISONSPREAD_PARAM,
		PWMAMT_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		OCTCVIN_INPUT,
		AFTIN_INPUT,
		PWIN_INPUT,
		// MWIN_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUTPORT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	float getVoicingRatio(int noteNum) {
        float ratio = (440.0 * powf(2.0, (float)(noteNum - 69) / 12)) / (440.0 * powf(2.0, (float)(12 - 69) / 12));
        return ratio;
    }

    VariableSawOscillator osc[4];
    float freq = 440.f;
    float out, aft, tune, bendfactor, transp, spread = 0.f;
    float bendrange = 2.f;
    float octave = 1.f;
    float voicing[6][4] = { {1.f, getVoicingRatio(7), getVoicingRatio(2), getVoicingRatio(0)},
                            {1.f, getVoicingRatio(8), getVoicingRatio(3), getVoicingRatio(1)},
                            {1.f, getVoicingRatio(9), getVoicingRatio(4), getVoicingRatio(1)},
                            {1.f, getVoicingRatio(7), getVoicingRatio(5), getVoicingRatio(1)},
                            {1.f, getVoicingRatio(9), getVoicingRatio(6), getVoicingRatio(3)},
                            {1.f, getVoicingRatio(8), getVoicingRatio(4), getVoicingRatio(0)} };

	SimpleOSCvarSaw() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(TRANSP_PARAM, -6.f, 6.f, 0.f, "transpose by semitones");
		configParam(VOICINGSEL_PARAM, 0.f, 5.f, 1.f, "select voicing");
		configParam(OCTAVE_PARAM, -2.f, 2.f, 0.f, "transpose by octave");
		configParam(TUNE_PARAM, -0.1f, 0.1f, 0.f, "master tune offset");
		configParam(UNISONSPREAD_PARAM, 0.f, 1.0f, 0.f, "detune spread");
		configParam(PWMAMT_PARAM, -1.f, 1.0f, 0.f, "pwm amount");
		configInput(OCTCVIN_INPUT, "frequency input");
		configInput(AFTIN_INPUT, "aftertouch/breath controller input");
		configInput(PWIN_INPUT, "pitch wheel/bend sensor input");
		// configInput(MWIN_INPUT, "mod wheel/bite sensor input (NOT IMPLEMENTED)");
		configOutput(OUTPORT_OUTPUT, "output port");
        for (int i=0; i<4; i++){
            osc[i].Init(APP->engine->getSampleRate());
        };
	}

	void process(const ProcessArgs& args) override {
        if (outputs[OUTPORT_OUTPUT].isConnected()) {
            out = 0;
            freq = dsp::FREQ_C4 * powf(2.f, inputs[OCTCVIN_INPUT].getVoltage()) * octave * transp * tune;
            aft = inputs[AFTIN_INPUT].isConnected() ? (inputs[AFTIN_INPUT].getVoltage() / 10.f) : params[PWMAMT_PARAM].getValue();
            tune = powf(2.f, params[TUNE_PARAM].getValue());
            octave = powf(2.f, params[OCTAVE_PARAM].getValue());
            bendfactor = powf(2.f, ((inputs[PWIN_INPUT].getVoltage() / 5.f) * bendrange) / 12.f);
            transp = powf(2.f, params[TRANSP_PARAM].getValue() / 12.f);
            spread = params[UNISONSPREAD_PARAM].getValue();
            for(int i = 0; i < 4; i++) {
                osc[i].SetFreq(freq * (bendfactor < 0.99f ? voicing[(int)(params[VOICINGSEL_PARAM].getValue())][i] : bendfactor * powf(2.f, (spread * i) / 50.f) ));
                osc[i].SetPW(aft / 2.f + 0.5f);
                out += osc[i].Process();
            }
            outputs[OUTPORT_OUTPUT].setVoltage(5.f * out);
        } else {
            outputs[OUTPORT_OUTPUT].setVoltage(0.f);
        }
	}
};

struct SimpleOSCvarSawWidget : ModuleWidget {
	SimpleOSCvarSawWidget(SimpleOSCvarSaw* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/simpleOSCvarSaw.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackSnapKnob>(mm2px(Vec(9.071, 27.697)), module, SimpleOSCvarSaw::TRANSP_PARAM));
		addParam(createParamCentered<RoundBlackSnapKnob>(mm2px(Vec(20.32, 27.697)), module, SimpleOSCvarSaw::VOICINGSEL_PARAM));
		addParam(createParamCentered<RoundBlackSnapKnob>(mm2px(Vec(32.084, 27.697)), module, SimpleOSCvarSaw::OCTAVE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(32.084, 47.473)), module, SimpleOSCvarSaw::TUNE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(32.084, 67.249)), module, SimpleOSCvarSaw::UNISONSPREAD_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(20.32, 67.249)), module, SimpleOSCvarSaw::PWMAMT_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.071, 47.473)), module, SimpleOSCvarSaw::OCTCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.071, 67.249)), module, SimpleOSCvarSaw::AFTIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.071, 87.024)), module, SimpleOSCvarSaw::PWIN_INPUT));
		// addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.084, 87.024)), module, SimpleOSCvarSaw::MWIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.32, 108.773)), module, SimpleOSCvarSaw::OUTPORT_OUTPUT));
	}
};


Model* modelSimpleOSCvarSaw = createModel<SimpleOSCvarSaw, SimpleOSCvarSawWidget>("simpleOSCvarSaw");
