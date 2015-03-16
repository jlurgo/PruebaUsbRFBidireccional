#include <aJSON.h>

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

#define SIZE_BUFFER_ENTRADA 256

const int buttonPin = 2;     // the number of the pushbutton pin
const int ledPin =  3;      // the number of the LED pin
int estadoAnteriorBoton = 0;
int estadoActualBoton = 0;

const char* estados_boton[] = { "suelto", "presionado"};

char buffer_entrada[256];
int ultimo_byte_entrada = 0;

RF24 radio(9,10);

void setup(void)
{ 
  	pinMode(ledPin, OUTPUT);      
  	pinMode(buttonPin, INPUT);     

  	Serial.begin(115200);
	printf_begin();
	blanquearBufferEntrada();
	
	radio.begin();
  	radio.setRetries(15,15);
  	radio.setPayloadSize(8);
	
    radio.openWritingPipe(0xF0F0F0F0D2LL);
    radio.openReadingPipe(1,0xF0F0F0F0E1LL);

  	radio.startListening();
	
  	radio.printDetails();	
}

void blanquearBufferEntrada(void){
	for(int i=0; i<SIZE_BUFFER_ENTRADA; i++){
		buffer_entrada[i] = '\0';	
	}	
}

void loop(void)
{
	estadoActualBoton = digitalRead(buttonPin);

	if(estadoActualBoton != estadoAnteriorBoton)
	{
		aJsonObject *mensaje;
    	mensaje=aJson.createObject();  
    	aJson.addItemToObject(mensaje, "estadoBoton", aJson.createItem(estados_boton[estadoActualBoton]));
		
		char *json = aJson.print(mensaje);
		
		radio.stopListening();
		
		for(int i=0; i<strlen(json); i++){
			bool ok = radio.write((json + i), 1);
		}
                char final = '|';
                bool ok = radio.write(&final, 1);
		radio.startListening();
		
		free(json);
		aJson.deleteItem(mensaje);
	}
	estadoAnteriorBoton = estadoActualBoton;
	
	
	if (radio.available()){
		char char_recibido_radio;
		bool done = false;
		while (!done)
		{
			done = radio.read( &char_recibido_radio, 1);
			if(char_recibido_radio == '\n' || char_recibido_radio == '\r' || char_recibido_radio == '|'){
				aJsonObject *msg = aJson.parse(buffer_entrada);	
				if (msg) {
					aJsonObject *estadoBoton = aJson.getObjectItem(msg, "estadoBoton");
					if (estadoBoton) {
						if(strcmp(estadoBoton->valuestring, "presionado")==0)digitalWrite(ledPin, HIGH);
						if(strcmp(estadoBoton->valuestring, "suelto")==0)digitalWrite(ledPin, LOW);
					}else{
						Serial.println("el mensaje no tiene estadoBoton");
					}				
				}else{
					Serial.println("error al interpretar json");
				}
				blanquearBufferEntrada();
				ultimo_byte_entrada = 0;
				aJson.deleteItem(msg);
			}else{      
				buffer_entrada[ultimo_byte_entrada] = char_recibido_radio;	
				ultimo_byte_entrada++;
			}
		}
	}
}

void serialEvent() {
	while (Serial.available()) {  
		char byte_leido = Serial.read();
		if(byte_leido == '\n' || byte_leido == '\r' || byte_leido == '|'){
			aJsonObject *msg = aJson.parse(buffer_entrada);	
			if (msg) {
				aJsonObject *estadoBoton = aJson.getObjectItem(msg, "estadoBoton");
				if (estadoBoton) {
					if(strcmp(estadoBoton->valuestring, "presionado")==0)digitalWrite(ledPin, HIGH);
					if(strcmp(estadoBoton->valuestring, "suelto")==0)digitalWrite(ledPin, LOW);
				}else{
					Serial.println("el mensaje no tiene estadoBoton");
				}				
			}else{
				Serial.println("error al interpretar json");
			}
			blanquearBufferEntrada();
			ultimo_byte_entrada = 0;
			aJson.deleteItem(msg);
		}else{      
			buffer_entrada[ultimo_byte_entrada] = byte_leido;	
			ultimo_byte_entrada++;
		}
	}
}
