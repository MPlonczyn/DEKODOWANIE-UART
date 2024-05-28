#include <LPC210X.H>
#include "uart.h"
#include "string.h"

#define NULL 																			 '\0'

/************ UART ************/
// U0LCR Line Control Register
#define mDIVISOR_LATCH_ACCES_BIT                   0x00000080
#define m8BIT_UART_WORD_LENGTH                     0x00000003

// UxIER Interrupt Enable Register
#define mRX_DATA_AVALIABLE_INTERRUPT_ENABLE        0x00000001
#define mTHRE_INTERRUPT_ENABLE                     0x00000002

// UxIIR Pending Interrupt Identification Register
#define mINTERRUPT_PENDING_IDETIFICATION_BITFIELD  0x0000000F
#define mTHRE_INTERRUPT_PENDING                    0x00000002
#define mRX_DATA_AVALIABLE_INTERRUPT_PENDING       0x00000004

// Setting RxD and TxD on UART0
#define mRXD_ON_P0_1															 0x00000004

/************ Interrupts **********/
// VIC (Vector Interrupt Controller) channels
#define VIC_UART0_CHANNEL_NR  6
#define VIC_UART1_CHANNEL_NR  7

// VICVectCntlx Vector Control Registers
#define mIRQ_SLOT_ENABLE                           0x00000020

////////////// Zmienne globalne ////////////
char cOdebranyZnak = '1';

struct RecieverBuffer sRxBuffer = {"", 0, EMPTY};

void Reciever_PutCharacterToBuffer(char cCharacter){
	
	switch(sRxBuffer.eStatus){
		
		case EMPTY:
			if(cCharacter == TERMINATOR){
				sRxBuffer.eStatus = READY;
				sRxBuffer.cData[sRxBuffer.ucCharCtr] = NULL;
				sRxBuffer.ucCharCtr = 0;
			}
			else if (sRxBuffer.ucCharCtr == (RECIEVER_SIZE - 1)){
				sRxBuffer.eStatus = OVERFLOW;
				sRxBuffer.ucCharCtr = 0;
			}
			else{
				sRxBuffer.eStatus = EMPTY;
				sRxBuffer.cData[sRxBuffer.ucCharCtr] = cCharacter;
				sRxBuffer.ucCharCtr++;
			}
			break;
		
		case READY:
			break;
		
		case OVERFLOW:
			break;
	}
	
}

enum eRecieverStatus eReciever_GetStatus(){
	return sRxBuffer.eStatus;
}

void Reciever_GetStringCopy(char * ucDestination){
	CopyString(sRxBuffer.cData,ucDestination);
	sRxBuffer.eStatus = EMPTY;
}


///////////////////////////////////////////
__irq void UART0_Interrupt (void) {
   // jesli przerwanie z odbiornika (Rx)
   
   unsigned int uiCopyOfU0IIR=U0IIR; // odczyt U0IIR powoduje jego kasowanie wiec lepiej pracowac na kopii

   if      ((uiCopyOfU0IIR & mINTERRUPT_PENDING_IDETIFICATION_BITFIELD) == mRX_DATA_AVALIABLE_INTERRUPT_PENDING) // odebrano znak
   {
      Reciever_PutCharacterToBuffer(U0RBR);
   } 
   
   if ((uiCopyOfU0IIR & mINTERRUPT_PENDING_IDETIFICATION_BITFIELD) == mTHRE_INTERRUPT_PENDING)              // wyslano znak - nadajnik pusty 
   {
      // narazie nic nie wysylamy
   }
	
   VICVectAddr = 0; // Acknowledge Interrupt
}

////////////////////////////////////////////
void UART_InitWithInt(unsigned int uiBaudRate){

   // UART0
   PINSEL0 = PINSEL0 | mRXD_ON_P0_1;                                     // ustawic pina na odbiornik uart0
   U0LCR  |= m8BIT_UART_WORD_LENGTH | mDIVISOR_LATCH_ACCES_BIT; // dlugosc slowa, DLAB = 1
   U0DLL   = ((15000000/16)/uiBaudRate);										// predkosc transmisji
   U0LCR  &= (~mDIVISOR_LATCH_ACCES_BIT);                       // DLAB = 0
   U0IER  |= mRX_DATA_AVALIABLE_INTERRUPT_ENABLE;               // aktywacja zrodla przerwania dla UART0, przez linie Rx

   // INT
   VICVectAddr1  = (unsigned long) UART0_Interrupt;             // set interrupt service routine address
   VICVectCntl1  = mIRQ_SLOT_ENABLE | VIC_UART0_CHANNEL_NR;     // use it for UART 0 Interrupt
   VICIntEnable |= (0x1 << VIC_UART0_CHANNEL_NR);               // Enable UART 0 Interrupt Channel
}
