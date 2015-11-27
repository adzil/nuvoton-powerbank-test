#ifndef __FSM_H__
#define __FSM_H__

#include <inttypes.h>

// State definition
#define state_main_offset 0
#define state_main_mask (0x3 << state_main_offset)
#define state_main_off (0x0 << state_main_offset)
#define state_main_ready (0x1  << state_main_offset)
#define state_main_active (0x2 << state_main_offset)
#define state_main_charge (0x3  << state_main_offset)

#define state_torch_offset 2
#define state_torch_mask (0x1 << state_torch_offset)
#define state_torch_off (0x0 << state_torch_offset)
#define state_torch_on (0x1 << state_torch_offset)

// Input definition
#define input_button_off 0
#define input_button_on 1
#define input_load_off 0
#define input_load_on 1
#define input_source_off 0
#define input_source_on 1

// Output definition
#define output_indicator_off 0
#define output_indicator_on 1
#define output_indicator_blink 2
#define output_charge_in_off 0
#define output_charge_in_on 1
#define output_charge_out_off 0
#define output_charge_out_on 1
#define output_torch_off 0
#define output_torch_on 1

#define button_interval 25
#define button_hold_interval 50
#define load_interval 250

// Timer disabled state
#define timer_disabled (unsigned int) -1
    
// Input / Output typedef
typedef struct fsm_input_type {
    uint8_t button;
    uint8_t load;
    uint8_t source;
} fsm_input_t;

typedef struct fsm_output_type {
    uint8_t indicator;
    uint8_t charge_in;
    uint8_t charge_out;
    uint8_t torch;
} fsm_output_t;

typedef struct fsm_timer_type {
    unsigned int button;
    unsigned int buttonHold;
    unsigned int load;
} fsm_timer_t;

// Function prototypes
uint8_t timerTimeout(unsigned int *t);
uint8_t timerActive(unsigned int *t);
void timerDec(unsigned int *t);
void timerStart(fsm_timer_t *timer);
void changeState(uint8_t *state, uint8_t state_mask, uint8_t state_new);
void fsmSetButtonTimeout(fsm_input_t *input, fsm_input_t *lastInput,
    fsm_timer_t *timer);
void fsmSetButtonHoldTimeout(fsm_input_t *input, fsm_input_t *lastInput,
    fsm_timer_t *timer);
void fsmClearUncaughtTimeouts(fsm_timer_t *timer);
void fsmSaveInput(fsm_input_t *lastInput, fsm_input_t *input);
void fsm(fsm_output_t *output, fsm_input_t *input);

#endif // __FSM_H__
