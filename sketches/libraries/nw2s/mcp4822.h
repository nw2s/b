// ----------------------------------------------------------------------------
// max4822.h
//
// Provides an SPI based interface to the MCP4822 dual voltage output digital 
// to analog to converter.
//
// Author: Will Dickson, IO Rodeo Inc.
// This code is licensed via Apaache 2.0
// ----------------------------------------------------------------------------
#ifndef _MCP4822_H_
#define _MCP4822_H_

#define MCP4822_NUMCHAN 2

/* I should have done a better job rewriting this guy's code to make a single instance
   represent a single DAC channel, but haven't had time */

enum MCP4822_DAC_CHAN {MCP4822_DAC_A = 0, MCP4822_DAC_B = 1};

class MCP4822 {
private:
    int cs;
    int ldac;
    int gain[MCP4822_NUMCHAN];
    int getCmdWrd(int dac, int value);
public:
    MCP4822();
    MCP4822(int csPin, int ldacPin);
    void begin();
    void setValue(int dac, int value);
    void setValue_A(int value);
    void setValue_B(int value);
    void setValue_AB(int value_A, int value_B);
    void setGain2X(int dac);
    void setGain2X_A();
    void setGain2X_B();
    void setGain2X_AB();
    void setGain1X(int dac);
    void setGain1X_A();
    void setGain1X_B();
    void setGain1X_AB();
    void off_AB();
    void off_A();
    void off_B();
    void off(int dac);
};
#endif