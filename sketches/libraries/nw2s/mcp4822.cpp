// ----------------------------------------------------------------------------
// max4822.cpp
//
// Provides an SPI based interface to the MCP4822 dual voltage output digital 
// to analog converter.
//
// Author: Will Dickson, IO Rodeo Inc.
// This code is licensed via Apaache 2.0
// ----------------------------------------------------------------------------


#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#ifdef _SAM3XA_

#include <SPI.h>
#include "mcp4822.h"

#define ACTIVE      0b0001000000000000
#define SHUTDOWN    0b0000000000000000
#define GAIN_2X     0b0000000000000000
#define GAIN_1X     0b0010000000000000
#define DAC_A_WRITE 0b0000000000000000
#define DAC_B_WRITE 0b1000000000000000

MCP4822::MCP4822() {
}

// ----------------------------------------------------------------------------
// MCP4822::MCP4822
//
// Constructor
// ----------------------------------------------------------------------------
MCP4822::MCP4822(int csPin, int ldacPin) {
    // Configure chip select and latch pins
    cs = csPin;
    ldac = ldacPin;
}

// ----------------------------------------------------------------------------
// MCP4822::MCP4822
//
// initialization function
// ----------------------------------------------------------------------------
void MCP4822::begin() {
    pinMode(cs,OUTPUT);
    pinMode(ldac,OUTPUT);
    digitalWrite(cs,HIGH);
    digitalWrite(ldac,HIGH);
    // Set to default configuration
    setGain1X_AB();
}

// ---------------------------------------------------------------------------
// MCP4822::getCmdWrd
//
// Gets command work for writing value to given channel
// ---------------------------------------------------------------------------
int MCP4822::getCmdWrd(int dac, int value) {
    int cmdWrd = 0;
    switch (dac) {
        case MCP4822_DAC_A:
            cmdWrd = DAC_A_WRITE;
            break;
        case MCP4822_DAC_B:
            cmdWrd = DAC_B_WRITE;
            break;
        default:
            return 0;
    } 
    cmdWrd |= gain[dac];
    cmdWrd |= ACTIVE;
    cmdWrd |= (0b0000111111111111 & value);
    return cmdWrd;
}

// ----------------------------------------------------------------------------
// MCP4822::setValue
//
// Set the output value of the given dac channel
// ----------------------------------------------------------------------------
void MCP4822::setValue(int dac, int value) {
    int cmdWrd;
    uint8_t byte0;
    uint8_t byte1;
    
    // Enable SPI communications 
    digitalWrite(cs,LOW);
    // Send command word
    cmdWrd = getCmdWrd(dac,value);
    byte0 = cmdWrd >> 8;
    byte1 = cmdWrd & 0b0000000011111111;
    SPI.transfer(byte0);
    SPI.transfer(byte1);
    // Disable SPI communications
    digitalWrite(cs,HIGH);
    // Latch value
    digitalWrite(ldac,LOW);
    digitalWrite(ldac,HIGH);
}

// ---------------------------------------------------------------------------
// MCP4822::setValue_A
//
// Set the output value of dac A to the given value
// ---------------------------------------------------------------------------
void MCP4822::setValue_A(int value) {
    setValue(MCP4822_DAC_A, value);
}

// ----------------------------------------------------------------------------
// MCP4822::setValue_B
//
// Set the output value of dac B to the given value
// ----------------------------------------------------------------------------
void MCP4822::setValue_B(int value) {
    setValue(MCP4822_DAC_B,value);
}

// ---------------------------------------------------------------------------
// MCP4822::setValue_AB
//
// Set the output value of dac A and B to the given values. Latch them in at 
// the same time.
// ---------------------------------------------------------------------------
void MCP4822::setValue_AB(int value_A, int value_B) {
    int cmdWrd;
    uint8_t byte0;
    uint8_t byte1;
    // Enable SPI communications and send command word
    digitalWrite(cs,LOW);
    // Send command word for DAC A
    cmdWrd = getCmdWrd(MCP4822_DAC_A,value_A);
    byte0 = cmdWrd >> 8;
    byte1 = cmdWrd & 0b0000000011111111;
    SPI.transfer(byte0);
    SPI.transfer(byte1);
    // Toggle CS 
    digitalWrite(cs,HIGH);
    digitalWrite(cs,LOW);
    // Send command word for DAC A
    cmdWrd = getCmdWrd(MCP4822_DAC_B,value_B);
    byte0 = cmdWrd >> 8;
    byte1 = cmdWrd & 0b0000000011111111;
    SPI.transfer(byte0);
    SPI.transfer(byte1);
    // Disable SPI communications
    digitalWrite(cs,HIGH);
    // Latch value
    digitalWrite(ldac,LOW);
    digitalWrite(ldac,HIGH);

}

// ---------------------------------------------------------------------------
// MCP4822::setGain2X
//
// Set the gain of the given channel to 2X
// ---------------------------------------------------------------------------
void MCP4822::setGain2X(int dac) {
    if ((dac == MCP4822_DAC_A) || (dac == MCP4822_DAC_B)) {
        gain[dac] = GAIN_2X;
    }
}

// ----------------------------------------------------------------------------
// MCP4822::setGain2X_A
//
// Set the gain of dac A to 2X
// ----------------------------------------------------------------------------
void MCP4822::setGain2X_A() {
    setGain2X(MCP4822_DAC_A);
}

// ----------------------------------------------------------------------------
// MCP4822::setGain2X_B
//
// Set the gain of dac B to 2X
// ----------------------------------------------------------------------------
void MCP4822::setGain2X_B() {
    setGain2X(MCP4822_DAC_B);
}

// ----------------------------------------------------------------------------
// MCP4822::setGain2X_AB
//
// Set the gain of dac A and B to 2X
// ----------------------------------------------------------------------------
void MCP4822::setGain2X_AB() {
    setGain2X_A();
    setGain2X_B();
}

// ---------------------------------------------------------------------------
// MCP4822::setGain1X
//
// Set the gain of the given channel to 1X
// ----------------------------------------------------------------------------
void MCP4822::setGain1X(int dac) {
    if ((dac == MCP4822_DAC_A) || (dac == MCP4822_DAC_B)) {
        gain[dac] = GAIN_1X;
    }
}

// ----------------------------------------------------------------------------
// MCP4822::setGain1X_A
//
// Set the gain of dac A to 1X
// ----------------------------------------------------------------------------
void MCP4822::setGain1X_A() {
    setGain1X(MCP4822_DAC_A);
}

// ----------------------------------------------------------------------------
// MCP4822::setGain1X_B
//
// Set the gain of dac B to 1X
// ----------------------------------------------------------------------------
void MCP4822::setGain1X_B() {
    setGain1X(MCP4822_DAC_B);
}

// ----------------------------------------------------------------------------
// MCP4822::setGain1X_AB
//
// Set the gain of dac A and B to 1X
// ----------------------------------------------------------------------------
void MCP4822::setGain1X_AB() {
    setGain1X_A();
    setGain1X_B();
}

// ----------------------------------------------------------------------------
// MCP4822::off
//
// Turn off the given dac
// ----------------------------------------------------------------------------
void MCP4822::off(int dac) {
    int cmdWrd = 0;
    uint8_t byte0;
    uint8_t byte1;
    
    // Create shutdown cmd word. 
    switch (dac) {
        case MCP4822_DAC_A:
            cmdWrd = DAC_A_WRITE;
            break;
        case MCP4822_DAC_B:
            cmdWrd = DAC_B_WRITE;
            break;
        default:
            return;
    } 
    cmdWrd |= SHUTDOWN;
    // Enable SPI communications 
    digitalWrite(cs,LOW);
    // Send command word
    byte0 = cmdWrd >> 8;
    byte1 = cmdWrd & 0b0000000011111111;
    SPI.transfer(byte0);
    SPI.transfer(byte1);
    // Disable SPI communications
    digitalWrite(cs,HIGH);
}

// ----------------------------------------------------------------------------
// MCP4822::off_A
//
// Turn off dac A
// ----------------------------------------------------------------------------
void MCP4822::off_A() {
    off(MCP4822_DAC_A);
}

// ----------------------------------------------------------------------------
// MCP4822::off_B
//
// Turn off dac B
// ----------------------------------------------------------------------------
void MCP4822::off_B() {
    off(MCP4822_DAC_B);
}

// ----------------------------------------------------------------------------
// MCP4822::off_AB
//
// Turn off dac A and B
// ----------------------------------------------------------------------------
void MCP4822::off_AB() {
    off_A();
    off_B();
}

#endif