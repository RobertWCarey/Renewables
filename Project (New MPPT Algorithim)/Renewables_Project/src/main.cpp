#include <Arduino.h>
#include "PWM.h"

// Processor Frequency
int32_t clkFreq = 16000000;

// Constants
const int pin_PWM = 10;
const int ADC_Vout = A0; // Buck-Boost output pin
const int ADC_Vpv = A4;  // PV output voltage pin
const int defaultDuty = 50;
const int vref = 18;
const int32_t defaultFreq = 100000; //frequency (in Hz)

// Globals
int Duty = defaultDuty;
int32_t Freq = defaultFreq;
String Command;

// Gets value to set analogWrite function
int getAWrite(int32_t freq, int duty);
// Updates the duty cycle for pin_PWM
void setDutyCycle(int duty);
// Updates the frequency for pin_PWM
void setFreq(int32_t freq);

void setup()
{
  Serial.begin(9600);
  //initialize all timers except for 0, to save time keeping functions
  InitTimersSafe();
  //sets the frequency for the specified pin
  bool success = SetPinFrequencySafe(pin_PWM, defaultFreq);
  if (success)
  {
    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH);
    pinMode(pin_PWM, OUTPUT);
  }
  pinMode(ADC_Vout, INPUT);
  pinMode(ADC_Vpv, INPUT);
  analogWrite(pin_PWM, getAWrite(defaultFreq, defaultDuty));
}

int getAWrite(int32_t freq, int duty)
{
  int32_t x = clkFreq / (2 * freq);

  return (x * duty) / 100;
}

void setDutyCycle(int duty)
{
  Duty = duty;
  analogWrite(pin_PWM, getAWrite(Freq, duty));
}

void setFreq(int32_t freq)
{
  Freq = freq;
  SetPinFrequencySafe(pin_PWM, freq);
  analogWrite(pin_PWM, getAWrite(freq, Duty));
}

unsigned long period = 1000;
bool toggle = 0;
unsigned long waitTime = 0;
double Vout_adjV = 0;
double Vpv_adjV = 0;
double Vout_CV = 0;
double Vpv_CV = 0;
double newP, oldP;
double V1 = 16;
double V2 = 21;
int largeStep = 10;
int smallStep = 1;

void peturb(double newP)
{
}

void loop()
{
  // time_now = millis();
  while (millis() > waitTime)
  {
    // toggle LED to indicate sample time
    DDRB |= (1 << PORTB5);
    if (toggle)
      PORTB |= (1 << PORTB5);
    else
      PORTB &= ~(1 << PORTB5);

    toggle = !toggle;

    Vout_CV = analogRead(ADC_Vout) * 0.0049;
    Vout_adjV = Vout_CV / 0.18;
    Vpv_CV = analogRead(ADC_Vpv) * 0.0049;
    Vpv_adjV = Vpv_CV / 0.18;
    Serial.print("Out:");
    Serial.println(Vout_adjV);
    Serial.print("PV:");
    Serial.println(Vpv_adjV);

    if (Vpv_adjV < V1)
    {
      Duty = Duty - largeStep;
    }
    else if (Vpv_adjV > V2)
    {
      Duty = Duty + largeStep;
    }
    else //peturb
    {
      newP = Vout_adjV * Vpv_adjV;

      if (oldP > newP)
        Duty = Duty - smallStep;
      else
        Duty = Duty + smallStep;

      oldP = newP;
    }

    if (Duty > 90)
      Duty = 90;
    else if (Duty < 10)
      Duty = 10;

    setDutyCycle(Duty);

    waitTime = millis() + period;
  }
}
