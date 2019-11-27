#include "plugin.hpp"


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
		configParam(GAIN_PARAM, 0.f, 1.f, 0.f, "");
	}

	void process(const ProcessArgs& args) override {
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