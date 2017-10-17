#include <msp430FR5994.h>
//Keep track of total bytes to expect
unsigned int totalBytes = 0;
//Keep track of the current byte
unsigned int byteCount = 0;
int duty1,duty2,duty3;
int main(void)
{
	//Disable High Impedance mode
    PM5CTL0 &= ~LOCKLPM5;
	//Select the PWM feature of pins 3.3, 3.4, 3.5
    P3SEL0 |= (BIT4 + BIT5 +BIT6);
	//Set the aforementioned pins as outputs
    P3DIR|= BIT4 + BIT5 +BIT6;

	//Initialise Timer module to act in the Reset/Set configuration
    TB0CCTL3 += OUTMOD_7;
    TB0CCTL4 += OUTMOD_7;
    TB0CCTL5 += OUTMOD_7;
	//Initialise CCR registers to 0
    TB0CCR3 = 0;
    TB0CCR4 = 0;
    TB0CCR5 = 0;
	//Initialise Clock to be based off SMClk and in up mode
    TB0CTL = TASSEL_2 + MC_1 ;
	//Set Period
    TB0CCR0 = 255;



    WDTCTL = WDTPW | WDTHOLD;               // Stop Watchdog

    // Configure GPIO
    P2SEL0 &= ~(BIT0 | BIT1);
    P2SEL1 |= BIT0 | BIT1;                  // USCI_A0 UART operation

    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings


    // Startup clock system with max DCO setting ~8MHz
    CSCTL0_H = CSKEY_H;                     // Unlock CS registers
    CSCTL1 = DCOFSEL_3 | DCORSEL;           // Set DCO to 8MHz
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;   // Set all dividers
    CSCTL0_H = 0;                           // Lock CS registers

    // Configure USCI_A0 for UART mode
    UCA0CTLW0 = UCSWRST;                    // Put eUSCI in reset
    UCA0CTLW0 |= UCSSEL__SMCLK;             // CLK = SMCLK
    // Baud Rate calculation
    // 8000000/(16*9600) = 52.083
    // Fractional portion = 0.083
    // User's Guide Table 21-4: UCBRSx = 0x04
    // UCBRFx = int ( (52.083-52)*16) = 1
    UCA0BRW = 52;                           // 8000000/16/9600
    UCA0MCTLW |= UCOS16 | UCBRF_1 | 0x4900;
    UCA0CTLW0 &= ~UCSWRST;                  // Initialize eUSCI
    UCA0IE |= UCRXIE;                       // Enable USCI_A0 RX interrupt

    __bis_SR_register(LPM3_bits | GIE);     // Enter LPM3, interrupts enabled
    __no_operation();                       // For debugger
}
//Set Duty cycles based on the received values
void setPWMDuty(int duty, int color)
{
    switch(color){
        case 1:
		//Subtract 255, as we need to invert our voltage, since the pins should be at a lower potential relative 
		//to the common annode
            TB0CCR3 = (duty);
            duty1=duty;
            break;
        case 2:
            TB0CCR4 = (duty);
            duty2=duty;
            break;
        case 3:
            TB0CCR5 = (duty);
            duty3=duty;
            break;
    }




}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=EUSCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(EUSCI_A0_VECTOR))) USCI_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch(__even_in_range(UCA0IV, USCI_UART_UCTXCPTIFG))
    {
        case USCI_NONE: break;
        case USCI_UART_UCRXIFG:
            while(!(UCA0IFG&UCTXIFG));
            if(byteCount == 0){
				//Enter this block if it is the first byte
                totalBytes = UCA0RXBUF;
                if(totalBytes<5){
					//Enter this block if we do not have the minimum 5 bytes (meaning we are the end of the message)
                    UCA0TXBUF = UCA0RXBUF;
                }else{
					//Immediately transfer the size of the next byte set, so that the next processor can begin processing,
					//while we are working with out own values.
                    UCA0TXBUF = UCA0RXBUF - 0x03;
                }
                byteCount++;
            }else if(byteCount<4 && totalBytes>=5){
                setPWMDuty(UCA0RXBUF,byteCount);
                byteCount++;
            }else{
				//Enter this block once we are transmitting the remaining RGB bytes.
				//We do not take into account any sort of stop byte, as we are assuming that inputs
				//will be perfect
                UCA0TXBUF = UCA0RXBUF;
                byteCount++;
            }
            if(byteCount == totalBytes){
				//Reset the byte count, and wait for the next message. LED's will hold their values until
				//a new message is received
                byteCount = 0;
            }

            __no_operation();
            break;
        case USCI_UART_UCTXIFG: break;
        case USCI_UART_UCSTTIFG: break;
        case USCI_UART_UCTXCPTIFG: break;
        default: break;
    }
}
