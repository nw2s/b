
#include <Arduino.h>
#include <Reset.h>

String inputString = "";         
boolean stringComplete = false;
boolean debug = false;


void setup() 
{
	Serial.begin(19200);
	Serial.println("Starting...");

	inputString.reserve(200);
}

void loop() 
{
	if (millis() % 1000 == 0)
	{
		if (debug) Serial.println(String(millis()));
	}
	
	
	if (stringComplete)
	{
		if (inputString == "ERASEANDRESET")
		{
			Serial.println("Received command: ERASEANDRESET");
			initiateReset(1);
			tickReset();
		}
		else if (inputString == "DEBUG ON")
		{
			Serial.println("Received command: " + inputString);
			debug = true;
		}
		else if (inputString == "DEBUG OFF")
		{
			Serial.println("Received command: " + inputString);
			debug = false;
		}
		else
		{
			Serial.println("Unknown command: " + inputString);
		}
		
		inputString = "";
		stringComplete = false;
	}
}

void serialEvent() 
{
	while (Serial.available()) 
	{
    	char c = (char)Serial.read(); 

	    if (c == '\n') 
		{
	    	stringComplete = true;
	    } 
		else
		{
			inputString += c;			
		}
  	}
}


