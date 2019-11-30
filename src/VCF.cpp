#include "plugin.hpp"

const float CUTOFF_PARAM_BASE = std::pow(2.f, 9.f);
const float CUTOFF_PARAM_MULTIPLIER = std::pow(2.f, 9.f);

struct VCF : Module {
	enum ParamIds {
		CUTOFF_PARAM,
		TYPE_PARAM,
		FREQ_PARAM,
		RESONANCE_PARAM,
		DRIVE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		FREQ_INPUT,
		RESONANCE_INPUT,
		DRIVE_INPUT,
		MAIN_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		MAIN_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	dsp::TBiquadFilter<float> filter;

	VCF() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		// range: (2^4) 16Hz - (2^13) 8192Hz
		// default at ~362Hz
		configParam(CUTOFF_PARAM, 0.f, 1.f, 0.5f, "Cutoff frequency", " Hz", CUTOFF_PARAM_BASE, CUTOFF_PARAM_MULTIPLIER);
		configParam(TYPE_PARAM, 0.f, 1.f, 0.f, "Filter type");
		configParam(FREQ_PARAM, -1.f, 1.f, 0.f, "Cutoff CV level", "%", 0.f, 100.f);
		configParam(RESONANCE_PARAM, 0.f, 1.f, 0.f, "Resonance", "%", 0.f, 100.f);
		configParam(DRIVE_PARAM, 0.f, 1.f, 0.f, "Gain", "", 0.f, 10.f);
	}

	float getFrequencyFromParam(float value) {
		return std::pow(CUTOFF_PARAM_BASE, value) * CUTOFF_PARAM_MULTIPLIER;
	}

	float getQFactorFromResonance(float paramValue) {
		return 0;
	}

	void process(const ProcessArgs& args) override {
		float cutoffParam = params[CUTOFF_PARAM].getValue();
		float freqParam = params[FREQ_PARAM].getValue();
		float typeParam = params[TYPE_PARAM].getValue();
		float driveParam = params[DRIVE_PARAM].getValue();
		float resParam = params[RESONANCE_PARAM].getValue();
		float freqInput = inputs[FREQ_INPUT].getVoltage();
		float driveInput = inputs[DRIVE_INPUT].getVoltage();
		float resInput = inputs[RESONANCE_INPUT].getVoltage();
		float in = inputs[MAIN_INPUT].getVoltage();

		// get type of filter
		dsp::TBiquadFilter::Type type = typeParam > 0 ? dsp::TBiquadFilter::HIGH_PASS : dsp::TBiquadFilter::LOW_PASS;

		// get modulated cutoff frequency (Nyquist)
		float cutoff = cutoffParam + freqParam * freqInput / 10.f;
		cutoff = clamp(cutoff, 0.f, 1.f);
		cutoff = this.getFrequencyFromParam(cutoff) / args.sampleRate;
		cutoff = clamp(cutoff, 0.f, 0.5f);

		// get resonance level / Q factor ()
		// assuming modulating source input is unipolar (0 - 10V)
		float resonance = resParam + resInput / 10.f;
		resonance = clamp(resonance, 0.f, 1.f);
		float qFactor = this.getQFactorFromResonance(resonance);

		// gain for filtered signal (before or after filtering ?)
		float drive = driveParam + driveInput / 10.f;
		float gain = std::pow(10.f, drive);
		// assuming input signal is bipolar (±5V)
		in = gain * in / 5.f;

		filter.setParameters(type, cutoff, qFactor, 1.f);
		float output = filter.process(in);

		outputs[MAIN_OUTPUT].setVoltage(output);
	}
};


struct VCFWidget : ModuleWidget {
	VCFWidget(VCF* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/VCF.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(16.914, 32.5)), module, VCF::CUTOFF_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(37.019, 52.5)), module, VCF::FREQ_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(37.019, 72.5)), module, VCF::RESONANCE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(37.019, 92.5)), module, VCF::DRIVE_PARAM));
		addParam(createParam<CKSS>(mm2px(Vec(34.6335, 25)), module, VCF::TYPE_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(13.802, 52.5)), module, VCF::FREQ_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(13.802, 72.5)), module, VCF::RESONANCE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(13.802, 92.5)), module, VCF::DRIVE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(13.802, 116.5)), module, VCF::MAIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(37.0, 116.5)), module, VCF::MAIN_OUTPUT));

		// mm2px(Vec(4.0, 7.0))
		addChild(createWidget<Widget>(mm2px(Vec(34.633, 25.0))));
	}
};


Model* modelVCF = createModel<VCF, VCFWidget>("VCF");