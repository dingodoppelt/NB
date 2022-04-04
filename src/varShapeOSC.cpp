#include "plugin.hpp"
#include "variableshapeosc.h"

using namespace daisysp;

struct VarShapeOSC : Module {
	enum ParamId {
		TRANSP_PARAM,
		TUNE_PARAM,
		OCT_PARAM,
		DETUNE_PARAM,
		AFT_PARAM,
		VOICING_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		VOCT_INPUT,
		OCTCVIN_INPUT,
		AFTIN_INPUT,
		PW_INPUT,
		VOICINGCV_INPUT,
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

    struct oscs {
        VariableShapeOscillator osc;
        float pwm_amt, pwm, detune = 0.f;
    };

    float getVoicingRatio(int noteNum) {
        float ratio = (440.0 * powf(2.0, (float)(noteNum - 69) / 12)) / (440.0 * powf(2.0, (float)(12 - 69) / 12));
        return ratio;
    }

    oscs vcosqr[4];
    oscs vcosaw[4];
    float freq = 440.f;
    float outSqr, outSaw, aft, tune, bendfactor, transp, spread = 0.f;
    float bendrange = 2.f;
    float octave = 1.f;
    int voicing_select = 0;
    float voicing[6][4] = { {1.f, getVoicingRatio(7), getVoicingRatio(2), getVoicingRatio(0)},
                            {1.f, getVoicingRatio(8), getVoicingRatio(3), getVoicingRatio(1)},
                            {1.f, getVoicingRatio(9), getVoicingRatio(4), getVoicingRatio(1)},
                            {1.f, getVoicingRatio(7), getVoicingRatio(5), getVoicingRatio(1)},
                            {1.f, getVoicingRatio(9), getVoicingRatio(6), getVoicingRatio(3)},
                            {1.f, getVoicingRatio(8), getVoicingRatio(4), getVoicingRatio(0)} };


	VarShapeOSC() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(TRANSP_PARAM, -6.f, 6.f, 0.f, "transpose by semitones");
		configParam(TUNE_PARAM, -0.1f, 0.1f, 0.f, "fine tune");
		configParam(DETUNE_PARAM, 0.f, 1.f, 0.f, "detune spread");
		configParam(OCT_PARAM, -2.f, 2.f, 0.f, "transpose by octave");
		configParam(AFT_PARAM, 0.f, 1.f, 0.f, "aftertouch/breath pressure");
		configParam(VOICING_PARAM, 0.f, 5.f, 0.f, "voicing selection");
		configInput(VOCT_INPUT, "v/oct");
		configInput(AFTIN_INPUT, "aftertouch/breath pressure cv in");
		configInput(PW_INPUT, "pitchwheel/bend sensor cv in");
		configInput(VOICINGCV_INPUT, "voicing selection cv in");
		configOutput(OUTSAW_OUTPUT, "output saw wave");
		configOutput(OUTSQR_OUTPUT, "output square wave");
        for (int i=0; i<4; i++){
            vcosqr[i].osc.Init(APP->engine->getSampleRate());
            vcosaw[i].osc.Init(APP->engine->getSampleRate());
            vcosqr[i].pwm = 0.52f - i / 100;
            vcosaw[i].pwm = 0.52f - i / 100;
            vcosqr[i].osc.SetWaveshape(1);
            vcosaw[i].osc.SetWaveshape(0);
        };
        vcosqr[0].pwm_amt = 0.25f;
        vcosqr[1].pwm_amt = -0.25f;
        vcosqr[2].pwm_amt = 0.45f;
        vcosqr[3].pwm_amt = -0.45f;
        vcosqr[0].detune = 1.f;
        vcosqr[1].detune = 0.9f;
        vcosqr[2].detune = 1.1;
        vcosqr[3].detune = 0.8f;
        vcosaw[0].pwm_amt = 0.5f;
        vcosaw[1].pwm_amt = -0.5f;
        vcosaw[2].pwm_amt = 0.25f;
        vcosaw[3].pwm_amt = -0.25f;
        vcosaw[0].detune = 1.f;
        vcosaw[1].detune = 0.9f;
        vcosaw[2].detune = 1.1;
        vcosaw[3].detune = 0.8f;
	}

	void process(const ProcessArgs& args) override {
        if (outputs[OUTSQR_OUTPUT].isConnected() || outputs[OUTSAW_OUTPUT].isConnected()) {
            outSqr = outSaw = 0.f;
            aft = inputs[AFTIN_INPUT].isConnected() ? powf(inputs[AFTIN_INPUT].getVoltage() / 10.f, 2) : params[AFT_PARAM].getValue();
            tune = powf(2.f, params[TUNE_PARAM].getValue());
            octave = inputs[OCTCVIN_INPUT].isConnected() ? powf(2.f, params[OCT_PARAM].getValue() * (int)(inputs[OCTCVIN_INPUT].getVoltage() / 10.f)) : powf(2.f, params[OCT_PARAM].getValue());
            bendfactor = powf(2.f, ((inputs[PW_INPUT].getVoltage() / 5.f) * bendrange) / 12.f);
            transp = powf(2.f, params[TRANSP_PARAM].getValue() / 12.f);
            spread = params[DETUNE_PARAM].getValue();
            freq = dsp::FREQ_C4 * powf(2.f, inputs[VOCT_INPUT].getVoltage()) * octave * transp * tune;
            voicing_select = inputs[VOICINGCV_INPUT].isConnected() ? (int)(inputs[VOICINGCV_INPUT].getVoltage() / 10.f ) * 6 : (int)(params[VOICING_PARAM].getValue());
            for(int i = 0; i < 4; i++) {
                vcosqr[i].osc.SetFreq(freq * (bendfactor < 0.99f ? voicing[voicing_select][i] : bendfactor * (powf(vcosqr[i].detune, powf(spread, 4)))));
                vcosaw[i].osc.SetFreq(freq * (bendfactor < 0.99f ? voicing[voicing_select][i] : bendfactor * (powf(vcosaw[i].detune, powf(spread, 4)))));
                vcosqr[i].osc.SetPW(vcosqr[i].pwm + (vcosqr[i].pwm_amt * aft));
                vcosaw[i].osc.SetPW(vcosaw[i].pwm + (vcosaw[i].pwm_amt * aft));
                outSqr += vcosqr[i].osc.Process();
                outSaw += vcosaw[i].osc.Process();
            }
            outputs[OUTSAW_OUTPUT].setVoltage(2.f * outSaw);
            outputs[OUTSQR_OUTPUT].setVoltage(2.f * outSqr);
        } else {
            outputs[OUTSAW_OUTPUT].setVoltage(0.f);
            outputs[OUTSQR_OUTPUT].setVoltage(0.f);
        }
	}
};


struct VarShapeOSCWidget : ModuleWidget {
	VarShapeOSCWidget(VarShapeOSC* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/varShapeOSC.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackSnapKnob>(mm2px(Vec(20.176, 28.445)), module, VarShapeOSC::TRANSP_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(33.076, 33.572)), module, VarShapeOSC::TUNE_PARAM));
		addParam(createParamCentered<RoundBlackSnapKnob>(mm2px(Vec(20.011, 50.937)), module, VarShapeOSC::OCT_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(32.58, 50.937)), module, VarShapeOSC::DETUNE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(31.257, 67.144)), module, VarShapeOSC::AFT_PARAM));
		addParam(createParamCentered<RoundBlackSnapKnob>(mm2px(Vec(31.203, 87.843)), module, VarShapeOSC::VOICING_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.119, 35.887)), module, VarShapeOSC::VOCT_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.781, 50.441)), module, VarShapeOSC::OCTCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.607, 64.498)), module, VarShapeOSC::AFTIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(16.485, 81.31)), module, VarShapeOSC::PW_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.173, 92.014)), module, VarShapeOSC::VOICINGCV_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(16.207, 108.034)), module, VarShapeOSC::OUTSAW_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.001, 107.91)), module, VarShapeOSC::OUTSQR_OUTPUT));
	}
};


Model* modelVarShapeOSC = createModel<VarShapeOSC, VarShapeOSCWidget>("varShapeOSC");
