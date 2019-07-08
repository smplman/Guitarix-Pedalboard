/*
https://medium.com/@atippy83/guitarix-the-pi-dle-board-8d6298ca8e42
https://arre234.blogspot.com/2018/02/linux-portable-wifi-guitar-amp-on.html

https://github.com/tttapa/MIDI_controller
https://github.com/tttapa/MIDI_controller/wiki/Library-documentation

https://github.com/alf45tar/Pedalino
*/

#include <Arduino.h>
#include <PluggableUSB.h>

// Shift PWM
// You can choose the latch pin yourself.
const int ShiftPWM_latchPin = 13;

// If your LED's turn on if the pin is low, set this to true, otherwise set it to false.
const bool ShiftPWM_invertOutputs = true;

// You can enable the option below to shift the PWM phase of each shift register by 8 compared to the previous.
// This will slightly increase the interrupt load, but will prevent all PWM signals from becoming high at the same time.
// This will be a bit easier on your power supply, because the current peaks are distributed.
const bool ShiftPWM_balanceLoad = false;

#include <ShiftPWM.h> // include ShiftPWM.h after setting the pins!

unsigned char maxBrightness = 255;
unsigned char pwmFrequency = 75;
int numRegisters = 2;
//int numRGBleds = numRegisters*8/3;
int numRGBleds = 5;

int redVal;
int blueVal;
int greenVal;
int footSwitch;

// MIDI
#include <MIDI_Controller.h> // Include the library

USBMIDI_Interface mi;

using namespace MIDI_CC;
using namespace MCU;

const uint8_t channel = 1;

// Foot switches
//DigitalCC button(2, MIDI_CC::Effects_1, channel);
const int footSwitchPins[] = {2, 3, 4, 5, 6};
int footSwitchState[5] = {0, 0, 0, 0, 0};
DigitalCC footSwitches[] = {
    {2, MIDI_CC::Effects_1, channel},
    {3, MIDI_CC::Effects_2, channel},
    {4, MIDI_CC::Effects_3, channel},
    {5, MIDI_CC::Effects_4, channel},
    {6, MIDI_CC::Effects_5, channel},
};

// Rotary Potentiometer
//Analog rPotentiometer(A0, MIDI_CC::Channel_Volume, 1);
AnalogCC knobs[] = {
    {A11, MIDI_CC::General_Purpose_Controller_1, channel},
    {A10, MIDI_CC::General_Purpose_Controller_2, channel},
    {A9, MIDI_CC::General_Purpose_Controller_3, channel},
    {A8, MIDI_CC::General_Purpose_Controller_4, channel}};

// Slide Potentiometers
//Analog lPotentiometer(A1, MIDI_CC::Pan, 1);

AnalogCC sliders[] = {
    {A0, MIDI_CC::General_Purpose_Controller_5, channel},
    {A1, MIDI_CC::General_Purpose_Controller_6, channel},
    {A2, MIDI_CC::General_Purpose_Controller_7, channel},
    {A3, MIDI_CC::General_Purpose_Controller_8, channel}};

// Create Bank
Bank bank(4);
//BankSelector bankSelector(bank, { 2, 3, 4, 5, 6 });

void setup()
{
  Serial.begin(9600);
  Serial.println("Start");

  // Sets the number of 8-bit registers that are used.
  ShiftPWM.SetAmountOfRegisters(numRegisters);
  ShiftPWM.Start(pwmFrequency, maxBrightness);

  //bank.add(knobs, Bank::CHANGE_CHANNEL);
  //bank.add(sliders, Bank::CHANGE_CHANNEL);
  bank.add(knobs, Bank::CHANGE_ADDRESS);
  bank.add(sliders, Bank::CHANGE_ADDRESS);
}

unsigned long timePress = 0;
unsigned long timePressLimit = 0;
int clicks = 0;
int modifying = 0;
int currentState = 0;

void loop()
{
  // Read pot values
  //  redVal = analogRead(11) * (255./1023.);
  //  greenVal = analogRead(10) * (255./1023.);
  //  blueVal = analogRead(9) * (255./1023.);
  //  ShiftPWM.SetRGB(1,redVal,greenVal,blueVal);
  //  ShiftPWM.SetRGB(1,0,0,255);
  //
  //  footSwitch = digitalRead(2);
  //
  //  if(footSwitch) {
  //    ShiftPWM.SetRGB(0,0,255,0); // Green
  //    checkDoubleClick();
  //  } else {
  //    ShiftPWM.SetRGB(0,255,0,0);
  //  }

  // Read footswitches
  for (int i = 0; i < 5; i++)
  {
    currentState = digitalRead(footSwitchPins[i]);

    // Set LED color if the effect is turned on or off or blue for modifying
    if (modifying == i && currentState == LOW)
    {
      // Set LED to blue so we know which effect we are modifying
      ShiftPWM.SetRGB(i, 0, 0, 255);
    }
    else if (currentState == LOW)
    {
      ShiftPWM.SetRGB(i, 0, 255, 0); // Green for ON
    }
    else
    {
      ShiftPWM.SetRGB(i, 255, 0, 0); // Red for OFF
    }

    // Set MIDI Channel if an effect is turned on by foot switch
    if (footSwitchState[i] == HIGH && currentState == LOW)
    {
      Serial.println(i);
      bank.setBankSetting(i);
      modifying = i;
    }

    footSwitchState[i] = currentState;
  }

  // Refresh the button (check whether the button's state has changed since last time, if so, send it over MIDI)
  MIDI_Controller.refresh();
}

void checkDoubleClick()
{
  if (clicks == 0)
  {
    timePress = millis();
    timePressLimit = timePress + 1000;
    clicks = 1;
  }
  else if (clicks == 1 && millis() < timePressLimit)
  {
    ShiftPWM.SetRGB(0, 0, 0, 255);
    //set variables back to 0
    timePress = 0;
    timePressLimit = 0;
    clicks = 0;
  }
}