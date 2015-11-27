#include "fsm.h"

// Checks if the timer is timeout
uint8_t timerTimeout(unsigned int *t) {
    if (*t == 0) {
        *t = timer_disabled;
        return 1;
    }
    else {
        return 0;
    }
}

// Checks if timer is active
uint8_t timerActive(unsigned int *t) {
    if (*t != 0 && *t != timer_disabled) {
        return 1;
    }
    else {
        return 0;
    }
}

// Decrement timer if active
void timerDec(unsigned int *t) {
    if (timerActive(t)) {
        *t -= 1;
    }
}

// Timer decrement, called every FSM calls
void timerStart(fsm_timer_t *timer) {
    timerDec(&((*timer).button));
    timerDec(&((*timer).buttonHold));
    timerDec(&((*timer).load));
}

// Change FSM state function
void changeState(uint8_t *state, uint8_t state_mask, uint8_t state_new) {
    *state = *state & ~(state_mask);
    *state = *state | state_new;
}

// Set button timeout on press
void fsmSetButtonTimeout(fsm_input_t *input, fsm_input_t *lastInput,
    fsm_timer_t *timer) {
    if ((*input).button == input_button_on &&
        (*lastInput).button == input_button_off) {
        (*timer).button = button_interval;
    }
}

// Set hold button timeout on long press
void fsmSetButtonHoldTimeout(fsm_input_t *input, fsm_input_t *lastInput,
    fsm_timer_t *timer) {
    if ((*input).button == input_button_on &&
        (*lastInput).button == input_button_off) {
        (*timer).buttonHold = button_hold_interval;
    }
    if ((*input).button == input_button_off &&
        (*lastInput).button == input_button_on) {
        (*timer).buttonHold = timer_disabled;
    }
}

// Clear timeouts if not caught by FSM state
void fsmClearUncaughtTimeouts(fsm_timer_t *timer) {
    timerTimeout(&((*timer).button));
    timerTimeout(&((*timer).buttonHold));
    timerTimeout(&((*timer).load));
}

// Save current input as last input
void fsmSaveInput(fsm_input_t *lastInput, fsm_input_t *input) {
    (*lastInput).button = (*input).button;
    (*lastInput).load = (*input).load;
    (*lastInput).source = (*input).source;
}

// Main FSM function
void fsm(fsm_output_t *output, fsm_input_t *input) {
    static fsm_input_t lastInput;
    static fsm_timer_t timer = { timer_disabled, timer_disabled, timer_disabled };
    // Initial FSM state
    static uint8_t state = state_main_off | state_torch_off;

    timerStart(&timer);

    // Main FSM realization
    if ((state & state_main_mask) == state_main_off) {
        // FSM state change
        if (timerTimeout(&(timer.button))) {
            timer.load = load_interval;
            changeState(&state, state_main_mask, state_main_ready);
        }
        else if ((*input).source == input_source_on &&
            lastInput.source == input_source_off) {
            changeState(&state, state_main_mask, state_main_charge);
        }
        else if ((*input).load == input_load_on &&
            lastInput.load == input_load_off) {
            changeState(&state, state_main_mask, state_main_active);
        }

    }
    else if ((state & state_main_mask) == state_main_ready) {
        // FSM state change
        if (timerTimeout(&(timer.buttonHold))) {
            changeState(&state, state_main_mask, state_main_off);
        }
        else if (timerTimeout(&(timer.load))) {
            changeState(&state, state_main_mask, state_main_off);
        }
        else if ((*input).source == input_source_on &&
            lastInput.source == input_source_off) {
            changeState(&state, state_main_mask, state_main_charge);
        }
        else if ((*input).load == input_load_on &&
            lastInput.load == input_load_off) {
            changeState(&state, state_main_mask, state_main_active);
        }
        // Button hold timer activation
        fsmSetButtonHoldTimeout(input, &lastInput, &timer);

    }
    else if ((state & state_main_mask) == state_main_active) {
        // FSM state change
        if (timerTimeout(&(timer.buttonHold))) {
            changeState(&state, state_main_mask, state_main_off);
        }
        else if ((*input).source == input_source_on &&
            lastInput.source == input_source_off) {
            changeState(&state, state_main_mask, state_main_charge);
        }
        else if ((*input).load == input_load_off &&
            lastInput.load == input_load_on) {
            timer.load = load_interval;
            changeState(&state, state_main_mask, state_main_ready);
        }
        // Button hold timer activation
        fsmSetButtonHoldTimeout(input, &lastInput, &timer);

    }
    else if ((state & state_main_mask) == state_main_charge) {
        if ((*input).source == input_source_off &&
            lastInput.source == input_source_on) {
            changeState(&state, state_main_mask, state_main_off);
        }

    }

    // Torch FSM realization
    if ((state & state_torch_mask) == state_torch_off) {
        // FSM state change
        if (timerActive(&(timer.button))) {
            if ((*input).button == input_button_on &&
                lastInput.button == input_button_off) {
                timer.button = timer_disabled;
                changeState(&state, state_torch_mask, state_torch_on);
            }
        }
        else {
            // Button timer activation
            fsmSetButtonTimeout(input, &lastInput, &timer);
        }

    }
    else if ((state & state_torch_mask) == state_torch_on) {
        // FSM state change
        if (timerActive(&(timer.button))) {
            if ((*input).button == input_button_on &&
                lastInput.button == input_button_off) {
                timer.button = timer_disabled;
                changeState(&state, state_torch_mask, state_torch_off);
            }
        }
        else {
            // Button timer activation
            fsmSetButtonTimeout(input, &lastInput, &timer);
        }

    }

    // Output stage
    // Main FSM output
    if ((state & state_main_mask) == state_main_off) {
        // Set output
        (*output).indicator = output_indicator_off;
        (*output).charge_in = output_charge_in_off;
        (*output).charge_out = output_charge_out_off;

    }
    else if ((state & state_main_mask) == state_main_ready) {
        // Set output
        (*output).indicator = output_indicator_on;
        (*output).charge_in = output_charge_in_off;
        (*output).charge_out = output_charge_out_off;

    }
    else if ((state & state_main_mask) == state_main_active) {
        // Set output
        (*output).indicator = output_indicator_on;
        (*output).charge_in = output_charge_in_off;
        (*output).charge_out = output_charge_out_on;


    }
    else if ((state & state_main_mask) == state_main_charge) {
        // Set output
        (*output).indicator = output_indicator_blink;
        (*output).charge_in = output_charge_in_on;
        (*output).charge_out = output_charge_out_off;

    }

    // Torch FSM output
    if ((state & state_torch_mask) == state_torch_off) {
        // Set output
        (*output).torch = output_torch_off;


    }
    else if ((state & state_torch_mask) == state_torch_on) {
        // Set output
        (*output).torch = output_torch_on;

    }

    // Clear uncaught timeouts
    fsmClearUncaughtTimeouts(&timer);
    // Copy input to last input
    fsmSaveInput(&lastInput, input);
}
