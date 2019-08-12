#include <Arduino.h>
#include "PWM.h"

// Processor Frequency
int32_t clkFreq = 16000000;

// Constants
const int pin_PWM = 10;
const int defaultDuty = 50;
const int32_t defaultFreq = 100000; //frequency (in Hz)

// Globals
int Duty = defaultDuty;
int32_t Freq = defaultFreq;
String Command;

// Gets value to set analogWrite function
int getAWrite(int32_t freq,int duty);
// Updates the duty cycle for pin_PWM
void setDutyCycle(int duty);
// Updates the frequency for pin_PWM
void setFreq(int32_t freq);
// Parses any serial commands recieved
bool parseCommand(String com);

void setup()
{
  Serial.begin(9600);
  //initialize all timers except for 0, to save time keeping functions
  InitTimersSafe();
  //sets the frequency for the specified pin
  bool success = SetPinFrequencySafe(pin_PWM, defaultFreq);
   if(success) {
      pinMode(13, OUTPUT);
      digitalWrite(13, HIGH);
      pinMode(pin_PWM,OUTPUT);
   }
  analogWrite(pin_PWM,getAWrite(defaultFreq,defaultDuty));
}

int getAWrite(int32_t freq,int duty)
{
  int32_t x = clkFreq/(2*freq);

  return (x*duty)/100;
}

void setDutyCycle(int duty)
{
  Duty = duty;
  analogWrite(pin_PWM,getAWrite(Freq, duty));
}

void setFreq(int32_t freq)
{
  Freq = freq;
  SetPinFrequencySafe(pin_PWM, freq);
  analogWrite(pin_PWM,getAWrite(freq, Duty));
}

bool parseCommand(String com)
{
  String part1,part2;
  int32_t val;

  part1 = com.substring(0, com.indexOf(' '));
  part2 = com.substring(com.indexOf(' ')+1);

  // Duty Cycle
  if (part1.equalsIgnoreCase("D"))
  {
    val = part2.toInt();
    if (val > 100 || val < 0)
    {
      return false;
    }
    else
    {
      setDutyCycle(val);
      Serial.print("Duty Cycle: ");
      Serial.print(Duty);
      Serial.println("%");
      return true;
    }
  }
  // Frequency
  else if (part1.equalsIgnoreCase("F"))
  {
    val = part2.toInt();
    if (val < 0)
    {
      return false;
    }
    else
    {
      setFreq(val);
      Serial.print("Frequency: ");
      Serial.print(Freq);
      Serial.println("Hz");
      return true;
    }
  }

  return false;
}

void loop()
{
  if (Serial.available())
  {
   char c = Serial.read();
   if (c == '\n')
   {
     if (!parseCommand(Command))
     {
       Serial.println("Invalid Input");
       Serial.println("");
       Serial.println("Valid Inputs:");
       Serial.println("'D 50', Duty Cycle Update");
       Serial.println("'F 10000', Frequency Update");
     }
     Command = "";
   }
   else
   {
     Command += c;
   }
  }
}
