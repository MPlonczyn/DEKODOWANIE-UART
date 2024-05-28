#include "uart.h"
#include "servo.h"
#include "command_decoder.h"
#include "string.h"

int main(){
	
	char cStringBufor[RECIEVER_SIZE];
	UART_InitWithInt(9600);
	ServoInit(50);
	
	while(1){
		if(sRxBuffer.eStatus == READY){
			Reciever_GetStringCopy(cStringBufor);
			DecodeMsg(cStringBufor);
			
			if((ucTokenNr != 0) && (asToken[0].eType == KEYWORD)){
				
				switch(asToken[0].uValue.eKeywordCode){
					
					case CL:
						ServoCallib();
						break;
					
					case GT:
						ServoGoTo(asToken[1].uValue.uiValue);
						break;
				}
			}
		}
	}
}
