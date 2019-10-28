#include "plugin.hpp"


struct VCO : Module {
	enum ParamIds {
		FREQ_PARAM,
		FM_PARAM,
		PWM_PARAM,
		PWIDTH_PARAM,
		SYNC_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		PITCH_INPUT,
		FM_INPUT,
		SYNC_INPUT,
		PWM_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		SIN_OUTPUT,
		TRI_OUTPUT,
		SAW_OUTPUT,
		SQR_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		PITCH_LIGHT,
		NUM_LIGHTS
	};

	// default frequency A4 (tuning standard)
	float baseFreq = dsp::FREQ_A4;
	float phase = 0.f;
	float lastSyncInput = 0.f;
	// for soft sync, reverse sync direction whenever sync input finishes a cycle
	float syncDirection = 1.f;

	VCO() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		// A0 -> A8
		configParam(FREQ_PARAM, -48.f, 48.f, 0.f, "Frequency", " Hz", dsp::FREQ_SEMITONE, dsp::FREQ_A4);
		configParam(FM_PARAM, 0.f, 1.f, 0.f, "FM", "%", 0.f, 100.f);
		configParam(PWM_PARAM, 0.f, 1.f, 0.f, "Pulse width modulation", "%", 0.f, 100.f);
		configParam(PWIDTH_PARAM, 0.f, 1.f, 0.5f, "Pulse width", "%", 0.f, 100.f);
		configParam(SYNC_PARAM, 0.f, 1.f, 1.f, "Sync mode");
	}

	void process(const ProcessArgs& args) override {
		float freqParam = params[FREQ_PARAM].getValue() / 12.f;
		float fmParam = params[FM_PARAM].getValue();
		float pwmParam = params[PWM_PARAM].getValue();
		float pwidthParam = params[PWIDTH_PARAM].getValue();
		bool isHardSync = params[SYNC_PARAM].getValue() > 0;

		float pitch = freqParam;

		if (inputs[PITCH_INPUT].isConnected()) {
			pitch += inputs[PITCH_INPUT].getVoltage();
		}

		if (inputs[FM_INPUT].isConnected()) {
			pitch += fmParam * inputs[FM_INPUT].getVoltage();
		}

		pitch = clamp(pitch, -4.f, 4.f);
        float freq = baseFreq * std::pow(2.f, pitch);
		// Accumulate the phase (or reset partway of sync direction is reversed)
        phase += freq * args.sampleTime * syncDirection;
		// wrap phase in range [-0.5, 0.5]
		phase -= phase >= 0.5f || phase <= -0.5f ? (1.f * syncDirection) : 0.f;

		if (inputs[SYNC_INPUT].isConnected()) {
			float syncInput = inputs[SYNC_INPUT].getVoltage();
			if (lastSyncInput <= 0 && syncInput >= 0) {
				if (isHardSync) {
					phase = 0.f;
				} else {
					// reverse sync direction
					syncDirection *= -1.f; 
				}
			}
			lastSyncInput = syncInput;
		} else {
			syncDirection = 1.f;
		}

        // Audio signals are typically +/-5V
		if (outputs[SIN_OUTPUT].isConnected()) {
        	outputs[SIN_OUTPUT].setVoltage(5.f * sin(phase));
		}
		if (outputs[TRI_OUTPUT].isConnected()) {
  	    	outputs[TRI_OUTPUT].setVoltage(5.f * tri(phase));
		}
		if (outputs[SAW_OUTPUT].isConnected()) {
			outputs[SAW_OUTPUT].setVoltage(5.f * saw(phase));
		}
		if (outputs[SQR_OUTPUT].isConnected()) {
			float pulseWidth = pwidthParam;
			if (inputs[PWM_INPUT].isConnected()) {
				pulseWidth += pwmParam * inputs[PWM_INPUT].getVoltage() / 10.f;
			}
			// avoid DC
			pulseWidth = clamp(pulseWidth, 0.01f, 0.99f);
        	outputs[SQR_OUTPUT].setVoltage(5.f * sqr(phase, pulseWidth));
		}
	}

	// need alias reduction for all these waves here
	float sin(float phase) {
		return std::sin(2.f * M_PI * phase);
	}

	float tri(float phase) {
		bool posDerivative = -0.25f < phase && phase < 0.25f;
		float x = posDerivative ? phase : (phase < 0 ? (phase + 0.5f) : (phase - 0.5f));
		return (posDerivative ? 4.f : -4.f) * x;
	}

	float saw(float phase) {
		return 2.f * phase;
	}

	float sqr(float phase, float pulseWidth) {
		return phase > (pulseWidth * 1.f - 0.5f) ? -1.f : 1.f;
	}
};


struct VCOWidget : ModuleWidget {
	VCOWidget(VCO* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/VCO.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.0, 29.5)), module, VCO::FREQ_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(36.5, 44.5)), module, VCO::FM_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(36.5, 60.5)), module, VCO::PWM_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(36.5, 76.5)), module, VCO::PWIDTH_PARAM));
		addParam(createParam<CKSS>(mm2px(Vec(34.5, 21.5)), module, VCO::SYNC_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.0, 98.5)), module, VCO::PITCH_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(19.8, 98.5)), module, VCO::FM_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(31.6, 98.5)), module, VCO::SYNC_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(43.4, 98.5)), module, VCO::PWM_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.0, 116.5)), module, VCO::SIN_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(19.8, 116.5)), module, VCO::TRI_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(31.6, 116.5)), module, VCO::SAW_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.4, 116.5)), module, VCO::SQR_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(22.25, 18.25)), module, VCO::PITCH_LIGHT));
	}
};


Model* modelVCO = createModel<VCO, VCOWidget>("VCO");