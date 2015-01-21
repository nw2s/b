/*  PCA9685 LED library for Arduino
    Copyright (C) 2012 Kasper Skårhøj    <kasperskaarhoj@gmail.com> 

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "PCA9685.h"

PCA9685::PCA9685() {}

void PCA9685::begin(int i2cAddress, int dim) 
{
	_i2cAddress = PCA9685_I2C_BASE_ADDRESS | (i2cAddress & B00111111);
	this->dim = dim;
}

bool PCA9685::init() 
{

	delay(1);
	writeRegister(PCA9685_MODE1, (byte)0x01);	// reset the device

	delay(1);
	bool isOnline;
	if (readRegister(PCA9685_MODE1)==0x01)	{
		isOnline = true;
	} else {
		isOnline = false;
	}
	writeRegister(PCA9685_MODE1, (byte)B10100000);	// set up for auto increment
	writeRegister(PCA9685_MODE2, (byte)0x10);	// set to output
	
	return isOnline;
}

void PCA9685::setLEDOn(int ledNumber) {
	writeLED(ledNumber, 0x1000, 0);
}

void PCA9685::setLEDOff(int ledNumber) {
	writeLED(ledNumber, 0, 0x1000);
}

void PCA9685::setLEDDimmed(int ledNumber, int amount) 
{		
	writeLED(ledNumber, 0, 0 + ((amount * 10) / this->dim));
}

void PCA9685::writeLED(int ledNumber, word LED_ON, word LED_OFF) {	// LED_ON and LED_OFF are 12bit values (0-4095); ledNumber is 0-15
	if (ledNumber >=0 && ledNumber <= 15)	{
		
		Wire1.beginTransmission(_i2cAddress);
		Wire1.write(PCA9685_LED0 + 4*ledNumber);

		Wire1.write(lowByte(LED_ON));
		Wire1.write(highByte(LED_ON));
		Wire1.write(lowByte(LED_OFF));
		Wire1.write(highByte(LED_OFF));
		
		Wire1.endTransmission();
	}
}


//PRIVATE
void PCA9685::writeRegister(int regAddress, byte data) {
	Wire1.beginTransmission(_i2cAddress);
	Wire1.write(regAddress);
	Wire1.write(data);
	Wire1.endTransmission();
}

word PCA9685::readRegister(int regAddress) {
	word returnword = 0x00;
	Wire1.beginTransmission(_i2cAddress);
	Wire1.write(regAddress);
	Wire1.endTransmission();
	Wire1.requestFrom((int)_i2cAddress, 1);
    
	//Wait for our 2 bytes to become available
	while (Wire1.available()) 
	{
		returnword |= Wire1.read(); 
    }
    
	Serial.println("---- " + String(_i2cAddress) + " " + String(returnword));
	
	return returnword;
}