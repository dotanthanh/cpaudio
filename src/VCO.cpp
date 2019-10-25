#include "plugin.hpp"


struct VCO : Module {
	enum ParamIds {
		FREQ_PARAM,
		FM_PARAM,
		PWM_PARAM,
		PWIDTH_PARAM,
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
		PATH5729_1_LIGHT,
		NUM_LIGHTS
	};

	VCO() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(FREQ_PARAM, 0.f, 1.f, 0.f, "");
		configParam(FM_PARAM, 0.f, 1.f, 0.f, "");
		configParam(PWM_PARAM, 0.f, 1.f, 0.f, "");
		configParam(PWIDTH_PARAM, 0.f, 1.f, 0.f, "");
	}

	void process(const ProcessArgs& args) override {
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

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.0, 98.5)), module, VCO::PITCH_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(19.8, 98.5)), module, VCO::FM_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(31.6, 98.5)), module, VCO::SYNC_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(43.4, 98.5)), module, VCO::PWM_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.0, 116.5)), module, VCO::SIN_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(19.8, 116.5)), module, VCO::TRI_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(31.6, 116.5)), module, VCO::SAW_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.4, 116.5)), module, VCO::SQR_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(22.25, 18.25)), module, VCO::PATH5729_1_LIGHT));

		// mm2px(Vec(4.0, 7.0))
		addChild(createWidget<Widget>(mm2px(Vec(34.5, 21.5))));
	}
};


Model* modelVCO = createModel<VCO, VCOWidget>("VCO");