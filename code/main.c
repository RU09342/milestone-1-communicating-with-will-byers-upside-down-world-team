#include <msp430F5529.h>
unsigned int delay=0;
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer


       P1OUT &= ~BIT0;                         // Clear P1.0 output latch for a defined power-on state
       P1DIR |= BIT0;                          // Set P1.0 to output direction

       //choosing pin 2.0 for hardware pwm
       P2SEL |= BIT0;
       P2DIR|= BIT0;

       TA1CCTL1 += OUTMOD_7;
       TA1CTL = TASSEL_2 + MC_1;
       TA1CCR0 = 1000;
       TA1CCR1 = 100;



       P1DIR &= ~BIT1;
       P1OUT |= BIT1;
       P1REN |= BIT1;

    //   TA0CCTL0 = CCIE;                          // CCR0 interrupt enabled
     //  TA0CCR0 = 62500;
     //  TA0CTL = TASSEL_1 + MC_1 + ID_3;         // SMCLK, upmode, clear TAR



    //   __bis_SR_register(LPM0_bits + GIE);
     //  __no_operation();
       while(1){

       }
   }
#pragma vector = TIMER0_A0_VECTOR
__interrupt void clockOverFlow(void){

    P1OUT^=BIT0;

}




