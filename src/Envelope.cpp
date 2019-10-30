#include "plugin.hpp"

// at least 1ms for attack/decay
const float MIN_DURATION = 1e-3f;
const float MAX_DURATION = 10.f;

struct Envelope : Module {
	enum ParamIds {
		ATTACK_PARAM,
		HOLD_PARAM,
		DECAY_PARAM,
		SUSTAIN_PARAM,
		RELEASE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		ATTACK_INPUT,
		HOLD_INPUT,
		DECAY_INPUT,
		SUSTAIN_INPUT,
		RELEASE_INPUT,
		GATE_INPUT,
		RETRIGGER_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		MAIN_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	float time = 0.f;
	float lastValue = 0.f;

	Envelope() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		// linear envelope
		configParam(ATTACK_PARAM, 0.f, 1.f, 0.5f, "Attack", " ms", 10000.f, MIN_DURATION * 1000);
		configParam(HOLD_PARAM, 0.f, 1.f, 0.f, "Hold ", " ms", 10000.f, MIN_DURATION * 1000);
		configParam(DECAY_PARAM, 0.f, 1.f, 0.5f, "Decay", " ms", 10000.f, MIN_DURATION * 1000);
		configParam(SUSTAIN_PARAM, 0.f, 1.f, 0.f, "Sustain", "%", 0.f, 100.f);
		configParam(RELEASE_PARAM, 0.f, 1.f, 0.5, "Release", " ms", 10000.f, MIN_DURATION * 1000);
	}

	// y[x] = y[]
	void process(const ProcessArgs& args) override {
		float attackParam = params[ATTACK_PARAM].getValue();
		float holdParam = params[HOLD_PARAM].getValue();
		float decayParam = params[DECAY_PARAM].getValue();
		float sustainParam = params[SUSTAIN_PARAM].getValue();
		float releaseParam = params[RELEASE_PARAM].getValue();

		time += args.sampleTime * 1000.f;
		float decayTime = std::pow(10000.f, decayParam);
		float base = std::log(1.f / sustainParam) / decayTime;

		float value;
		if (0.f < time && time < decayTime && inputs[GATE_INPUT].getVoltage() > 0) {
			value = 1.f * std::exp(-base * time);
		} else {
			value =r 1.f;
			time = 0.f;
		}

		// float attack = attackParam + inputs[ATTACK_INPUT].getVoltage() / 10.f;
		// float hold = holdParam + inputs[HOLD_INPUT].getVoltage() / 10.f;
		// float decay = decayParam + inputs[DECAY_INPUT].getVoltage() / 10.f;
		// float sustain = sustainParam + inputs[SUSTAIN_INPUT].getVoltage() / 10.f;
		// float release = releaseParam + inputs[RELEASE_INPUT].getVoltage() / 10.f;
		outputs[MAIN_OUTPUT].setVoltage(value);
	}
};


struct EnvelopeWidget : ModuleWidget {
	EnvelopeWidget(Envelope* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Envelope.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(36.217, 26.5)), module, Envelope::ATTACK_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(36.217, 43.5)), module, Envelope::HOLD_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(36.217, 60.5)), module, Envelope::DECAY_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(36.217, 77.5)), module, Envelope::SUSTAIN_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(36.218, 94.5)), module, Envelope::RELEASE_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(13.0, 26.5)), module, Envelope::ATTACK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(13.0, 43.5)), module, Envelope::HOLD_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(13.0, 60.5)), module, Envelope::DECAY_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(13.0, 77.5)), module, Envelope::SUSTAIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(13.0, 94.5)), module, Envelope::RELEASE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11.0, 116.5)), module, Envelope::GATE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.4, 116.5)), module, Envelope::RETRIGGER_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(39.8, 116.5)), module, Envelope::MAIN_OUTPUT));
	}
};


Model* modelEnvelope = createModel<Envelope, EnvelopeWidget>("Envelope");