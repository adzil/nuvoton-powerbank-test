#include "NUC100Series.h"
#include "cmsis_os.h"
#include "fsm.h"
#include <inttypes.h>

#define PLL_CLOCK           50000000

osThreadId tid_fsmTask;
osThreadId tid_blinkTask;
fsm_input_t input;
fsm_output_t output;
uint8_t blinkState;

void SYS_Init(void) {
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Enable Internal RC 22.1184MHz clock */
    CLK_EnableXtalRC(CLK_PWRCON_OSC22M_EN_Msk);

    /* Waiting for Internal RC clock ready */
    CLK_WaitClockReady(CLK_CLKSTATUS_OSC22M_STB_Msk);

    /* Switch HCLK clock source to Internal RC and HCLK source divide 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLK_S_HIRC, CLK_CLKDIV_HCLK(1));

    /* Enable external XTAL 12MHz clock */
    CLK_EnableXtalRC(CLK_PWRCON_XTL12M_EN_Msk);

    /* Waiting for clock ready */
    CLK_WaitClockReady(CLK_CLKSTATUS_XTL12M_STB_Msk);
    

    /* Set core clock as PLL_CLOCK from PLL */
    CLK_SetCoreClock(PLL_CLOCK);
}

void fsmTask (void const *arg) {
    uint32_t pins;
    
    while (1) {
        // Input sampling
        pins = ~(PA->PIN >> 3) & 0x7;
        input.button = pins & 0x1;
        input.load = (pins >> 1) & 0x1;
        input.source = (pins >> 2) & 0x1;
        // FSM main call
        fsm(&output, &input);
        // Outputs
        // Blink output
        blinkState = output.indicator & 0x2;
        if (!blinkState) {
            pins = (~output.indicator & 0x1) << 12;
            PC->DOUT = PC->PIN & ~(0x1 << 12);
            PC->DOUT = PC->PIN | pins;
        }
        // Static output
        pins = output.charge_in;
        pins |= output.charge_out << 1;
        pins |= output.torch << 2;
        pins = (~pins & 0x7) << 13;
        PC->DOUT = PC->PIN & ~(0x7 << 13);
        PC->DOUT = PC->PIN | pins;
        
        // Wait until next signal (max 20ms)
        osSignalWait(0x1, 20);
    }
}

void blinkTask (void const *arg) {
    while (1) {
        if (blinkState) {
            PC->DOUT = PC->PIN ^ (1 << 12);
        }
        
        osDelay(500);
    }
}

osThreadDef(fsmTask, osPriorityNormal, 1, 0);
osThreadDef(blinkTask, osPriorityNormal, 1, 0);

int main() {
    /* Unlock protected registers */
    SYS_UnlockReg();
    /* Init System, peripheral clock and multi-function I/O */
    SYS_Init();
    /* Lock protected registers */
    SYS_LockReg();
    
    GPIO_SetMode(PC, 12, GPIO_PMD_OUTPUT);
    GPIO_SetMode(PC, 13, GPIO_PMD_OUTPUT);
    GPIO_SetMode(PC, 14, GPIO_PMD_OUTPUT);
    GPIO_SetMode(PC, 15, GPIO_PMD_OUTPUT);
    
    GPIO_SetMode(PA, 3, GPIO_PMD_QUASI);
    GPIO_SetMode(PA, 4, GPIO_PMD_QUASI);
    GPIO_SetMode(PA, 5, GPIO_PMD_QUASI);
    GPIO_SetMode(PA, 2, GPIO_PMD_OUTPUT);
    
    // Turn off LEDS
    PC->DOUT = PC->PIN | (0xf << 12);
    // Drive low on PA3 (key output)
    PA->DOUT = PA->PIN & ~(0x1 << 2);
    // Set on high mode on PA0, PA1, PA2
    PA->DOUT = PA->PIN | (0x7 << 3);
    
    tid_fsmTask = osThreadCreate(osThread(fsmTask), NULL);
    tid_blinkTask = osThreadCreate(osThread(blinkTask), NULL);
    
    while(1) {
        // Control FSM task to delay every 20ms (50Hz)
        osDelay(20);
        osSignalSet(tid_fsmTask, 0x1);
    }
}
