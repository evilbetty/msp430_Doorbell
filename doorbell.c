#include "msp430.h"
#include "binary_data.h"        // Include our audio data

#define MCLK                    8000000
#define SAMPLES_PER_SECOND      8000
#define BUTTON                  BIT3
#define PIN_SPEAKER             BIT2

// Get the size of our data array
const unsigned long binary_data_size = sizeof( binary_data ) / sizeof( binary_data[0] );

// Placed outside main for control with interrupts
int counter = 1024;    // Counter for data location
int playnow = 0;    // Bool for activating audio

volatile unsigned char sample;


int main(void) {
    // Disable WDT and run with 8Mhz MCLK
    WDTCTL = WDTPW + WDTHOLD;
    DCOCTL = CALDCO_8MHZ;
    BCSCTL1 = CALBC1_8MHZ;

    // PIN2 as timer output
    P1SEL |= PIN_SPEAKER;
    P1DIR |= PIN_SPEAKER;

    // Button as input, output high for pullup, and pullup on
    P1DIR &= ~BUTTON;
    P1OUT |= BUTTON;
    P1REN |= BUTTON;
    // P1 interrupt enabled with P1.3 as trigger (raising edge)
    P1IE |= BUTTON;
    P1IES |= BUTTON;
    // P1.3 interrupt flag cleared.
    P1IFG &= ~BUTTON;

    //  Timer 0 for output PWM (for 8bit values)
    TA0CTL = TASSEL_2 | MC_1;
    TA0CCR0 = (256) - 1;
    TA0CCTL1 |= OUTMOD_7;

    // Timer 1 to interrupt at sample speed
    TA1CTL = TASSEL_2 | MC_1;
    TA1CCR0 = MCLK / SAMPLES_PER_SECOND - 1;
    TA1CCTL0 |= CCIE;

    _BIS_SR(GIE);       // Enable global interrupts

	while(1){
        
        if (playnow == 1)
        {
            if ( counter <= binary_data_size ) sample = binary_data[counter++];
            else playnow = 0;
        }
        else sample = 0;  // If i put this line audio never runs even when playnow = 1  ?!?
        LPM0;
    }
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void T1A0_ISR(void)
{
    // Output sample and wake mcu to get next sample ready
    TA0CCR1 = sample;
    LPM0_EXIT;
}

#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
    // Set playnow var when button pressed (and a start value)
    counter = 1024;     // Our audio wastes a whole KB of begin silence
    playnow = 1;
    P1IFG &= ~BUTTON;   // Clearing IFG. This is required.
}

