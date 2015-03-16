#include <aJSON.h>

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(9,10);

void setup(void)
{ 
  	Serial.begin(115200);
	printf_begin();

	radio.begin();
  	radio.setRetries(15,15);
  	radio.setPayloadSize(8);
	
    radio.openWritingPipe(0xF0F0F0F0E1LL);
    radio.openReadingPipe(1,0xF0F0F0F0D2LL);

  	radio.startListening();
	
  	radio.printDetails();	
}

void loop(void)
{
	if (radio.available()){
		char char_recibido_radio;
		bool done = false;
		while (!done)
		{
			done = radio.read( &char_recibido_radio, 1);
			Serial.write(char_recibido_radio);
		}
	}
}

void serialEvent() {
	while (Serial.available()) {  
		char char_recibido_serie = Serial.read();
		//envio radio
		radio.stopListening();
		bool ok = radio.write( &char_recibido_serie, 1);
		radio.startListening();
	}
}
