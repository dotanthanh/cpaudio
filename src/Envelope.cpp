#include "plugin.hpp"

// at least 1ms for attack/decay
const float MIN_DURATION = 1e-3f;
const float MAX_DURATION = 10.f;
const float PARAM_BASE = MAX_DURATION / MIN_DURATION;

struct EnvelopeClock {
	float time = 0.f;

	void step(float timeStep) {
		time += timeStep;
	}

	float getClockTime() {
		return time;
	}

	void reset() {
		time = 0.f;
	}
};

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

	dsp::SchmittTrigger trigger;
	EnvelopeClock clock;

	float lastGateInput = 0.f;
	float gateStartValue = 0.f;
	float gateEndValue = 0.f;
	float openGateDuration = 0.f;
	float lastValue = 0.f;

	Envelope() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		// linear envelope
		configParam(ATTACK_PARAM, 0.f, 1.f, 0.5f, "Attack", " ms", PARAM_BASE, MIN_DURATION * 1000.f);
		configParam(HOLD_PARAM, 0.f, 1.f, 0.f, "Hold ", " ms", PARAM_BASE, MIN_DURATION * 1000.f, -MIN_DURATION);
		configParam(DECAY_PARAM, 0.f, 1.f, 0.f, "Decay", " ms", PARAM_BASE, MIN_DURATION * 1000.f);
		configParam(SUSTAIN_PARAM, 0.f, 1.f, 0.5f, "Sustain", "%", 0.f, 100.f);
		configParam(RELEASE_PARAM, 0.f, 1.f, 0.f, "Release", " ms", PARAM_BASE, MIN_DURATION * 1000.f);
	}

	// y[x] = y[]
	void process(const ProcessArgs& args) override {
		float attackParam = params[ATTACK_PARAM].getValue();
		float holdParam = params[HOLD_PARAM].getValue();
		float decayParam = params[DECAY_PARAM].getValue();
		float sustainParam = params[SUSTAIN_PARAM].getValue();
		float releaseParam = params[RELEASE_PARAM].getValue();

		float attack = attackParam + inputs[ATTACK_INPUT].getVoltage() / 10.f;
		float hold = holdParam + inputs[HOLD_INPUT].getVoltage() / 10.f;
		float decay = decayParam + inputs[DECAY_INPUT].getVoltage() / 10.f;
		float sustain = sustainParam + inputs[SUSTAIN_INPUT].getVoltage() / 10.f;
		float release = releaseParam + inputs[RELEASE_INPUT].getVoltage() / 10.f;

		attack = clamp(attack, 0.f, 1.f);
		hold = clamp(hold, 0.f, 1.f);
		decay = clamp(decay, 0.f, 1.f);
		sustain = clamp(sustain, 0.f, 1.f);
		release = clamp(release, 0.f, 1.f);

		// get the duration for each phase of the envelope
		float attackTime = std::pow(10000.f, attack);
		float holdTime = std::pow(10000.f, hold) - MIN_DURATION * 1000.f;
		float decayTime = std::pow(10000.f, decay);
		float releaseTime = std::pow(10000.f, release);

		// identify trigger signal with low threshold of 0.1V and high threshold of 1V - 2V
		// (due to ringing and overshoot from Gibbs effect)
		bool isTriggered = trigger.process(rescale(inputs[RETRIGGER_INPUT].getVoltage(), 0.1f, 2.f, 0.f, 1.f));
		float gateInput = inputs[GATE_INPUT].getVoltage();
		bool gateIsOpened = gateInput > 1.f;

		// re-trigger if gate signal finished a period, or there is trigger signal while gate is opened
		if ((lastGateInput <= 1.f && gateInput > 1.f) || (isTriggered && gateIsOpened)) {
			gateStartValue = lastValue;
			openGateDuration = 0.f;
			clock.reset();
		}

		clock.step(args.sampleTime * 1000.f);
		float time = clock.getClockTime();
		float value;

		// calculate base for exponential decay function
		float attackBase = getExponentialBase(attack);
		float decayBase = getExponentialBase(decay);
		float releaseBase = getExponentialBase(release);

		if (gateIsOpened) {
			if (time <= attackTime) {
				// do attack
				// attack value approach but never reach 1
				// => use threshold normalization coefficient
				float normalizeCoef = 1.f - (1.f - gateStartValue) * std::pow(attackBase, -1.f);
				value = (1.f - (1.f - gateStartValue) * std::pow(attackBase, -time / attackTime)) / normalizeCoef;
			} else if (time <= attackTime + holdTime) {
				// hold stage, do nothing
			} else {
				// do decay (approaching sustain level)
				float decayDuration = time - holdTime - attackTime;
				value = (1.f - sustain) * std::pow(decayBase, -decayDuration / decayTime) + sustain;
			}

			openGateDuration += args.sampleTime * 1000.f;
			gateEndValue = value;
		} else {
			// do release (approaching 0)
			float releaseDuration = time - openGateDuration;
			value = gateEndValue * std::pow(releaseBase, -releaseDuration / releaseTime);
			gateStartValue = value;
		}

		// keeping last value for re-trigger
		lastValue = value;
		lastGateInput = gateInput;

		value = clamp(value, 0.f, 1.f);
		// unipolar
		outputs[MAIN_OUTPUT].setVoltage(10.f * value);
	}

	float getExponentialBase(float param) {
		// param should be in range [0, 1]
		if (0.f <= param && param <= 1.f) {
			return std::pow(100.f, -param) * 300.f;
		} else {
			// safe check: return natural base
			return std::exp(1);
		}
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
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(36.217, 94.5)), module, Envelope::RELEASE_PARAM));

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