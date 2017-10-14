#include <msp430.h> 

unsigned int ByteCount = 0;
unsigned int nBytes = 0;

#define RED BIT
#define GREEN BIT
#define BLUE BIT
#define BUTTON


float dutycycle(int hex)
{
    float duty;
    duty = (hex/0xFF)*255;
    return(int)duty;
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

    P1DIR |= BIT0;
    P1SEL0 |= BIT0;
    P1OUT |= BIT0;

    TB0CTL |=   TASSEL_2 |ID_2 | MC_1;

    TB0CCR0 = 255; // Count in CCR0 register
    TB0CCR1 = 255;
    TB0CCR2 = 255;
    TB0CCR3 = 255;

    TB0CCTL1 |= OUTMOD_3; // Enable CCR2 interrupt
    TB0CCTL2 |= OUTMOD_3; // Enable CCR2 interrupt
    TB0CCTL3 |= OUTMOD_3; // Enable CCR2 interrupt






    // Configure GPIO
    P2SEL0 &= ~(BIT0 | BIT1);
    P2SEL1 |= BIT0 | BIT1; // USCI_A0 UART operation

    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    // Startup clock system with max DCO setting ~8MHz
    CSCTL0_H = CSKEY >> 8; // Unlock clock registers
    CSCTL1 = DCOFSEL_3 | DCORSEL; // Set DCO to 8MHz
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1; // Set all dividers
    CSCTL0_H = 0; // Lock CS registers

    // Configure USCI_A0 for UART mode
    UCA0CTLW0 = UCSWRST;
    UCA0CTLW0 |= UCSSEL__SMCLK; // CLK = SMCLK
    // Baud Rate calculation
    // 8000000/(16*9600) = 52.083
    // Fractional portion = 0.083
    // UCBRFx = int ( (52.083-52)*16) = 1
    UCA0BR0 = 52; // 8000000/16/9600
    UCA0MCTLW |= UCOS16 | UCBRF_1 | 0x4900;
    UCA0CTLW0 &= ~UCSWRST; // Initialize eUSCI
    UCA0IE |= UCRXIE; // Enable USCI_A0 RX interrupt

    __bis_SR_register(LPM3_bits | GIE); // Enter LPM3, interrupts enabled
    __no_operation(); // For debugger
}

#pragma vector=EUSCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    switch(__even_in_range(UCA0IV, USCI_UART_UCTXCPTIFG))
    {
        case USCI_NONE: break;
        case USCI_UART_UCRXIFG:
            while(!(UCA0IFG & UCTXIFG));
            if(ByteCount == 0){
                UCA0TXBUF = UCA0RXBUF; //echo (add a minus 3)?
                nBytes = UCA0RXBUF;
                ByteCount ++;
            }
            else if(ByteCount == 1){
                ByteCount ++;
                TB0CCR1 = 255 - UCA0RXBUF;
            }
            else if(ByteCount == 2){
                ByteCount ++;
                TB0CCR2 = 255 - UCA0RXBUF;
            }
            else if(ByteCount == 3){
                ByteCount ++;
                TB0CCR3 = 255 - UCA0RXBUF;
            }
            else if(ByteCount > 3){
                UCA0TXBUF = UCA0RXBUF;
                ByteCount ++;
                if(ByteCount == nBytes)
                    ByteCount = 0;
            }
            __no_operation();
            break;
        case USCI_UART_UCTXIFG: break;
        case USCI_UART_UCSTTIFG: break;
        case USCI_UART_UCTXCPTIFG: break;
    }

}
