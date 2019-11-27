#include "plugin.hpp"

// fixed base of 50 for exponential control voltage input
const float EXP_BASE = 50.f;

struct VCA : Module {
	enum ParamIds {
		GAIN_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		LINEAR_INPUT,
		EXP_INPUT,
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

	VCA() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(GAIN_PARAM, 0.f, 1.f, 0.5f, "Gain", "%", 0.f, 100.f);
	}

	void process(const ProcessArgs& args) override {
		float gainLevel = params[GAIN_PARAM].getValue();
		float output = inputs[MAIN_INPUT].getVoltage();

		// amplitude modulation using exponential function
		float normalizeFactor = 1.f / (std::pow(EXP_BASE, 1) - 1.f);
		if (inputs[EXP_INPUT].isConnected()) {
			float expInput = inputs[EXP_INPUT].getVoltage() / 10.f;
			output *= normalizeFactor * std::pow(EXP_BASE, expInput);
		}

		// normal (linear) amplitude modulation
		if (inputs[LINEAR_INPUT].isConnected()) {
			output *= inputs[LINEAR_INPUT].getVoltage() / 10.f;
		}

		// apply gain and set output
		outputs[MAIN_OUTPUT].setVoltage(gainLevel * output);
	}
};


struct VCAWidget : ModuleWidget {
	VCAWidget(VCA* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/VCA.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(12.937, 32.5)), module, VCA::GAIN_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(12.7, 56.0)), module, VCA::LINEAR_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(12.7, 75.0)), module, VCA::EXP_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(12.7, 96.5)), module, VCA::MAIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(12.7, 116.5)), module, VCA::MAIN_OUTPUT));
	}
};


Model* modelVCA = createModel<VCA, VCAWidget>("VCA");