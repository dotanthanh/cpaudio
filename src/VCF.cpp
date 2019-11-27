#include "plugin.hpp"


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

	VCF() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(CUTOFF_PARAM, 0.f, 1.f, 0.f, "");
		configParam(TYPE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(FREQ_PARAM, 0.f, 1.f, 0.f, "");
		configParam(RESONANCE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(DRIVE_PARAM, 0.f, 1.f, 0.f, "");
	}

	void process(const ProcessArgs& args) override {
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