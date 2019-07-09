/*
https://medium.com/@atippy83/guitarix-the-pi-dle-board-8d6298ca8e42
https://arre234.blogspot.com/2018/02/linux-portable-wifi-guitar-amp-on.html

https://github.com/tttapa/MIDI_controller
https://github.com/tttapa/MIDI_controller/wiki/Library-documentation

https://github.com/alf45tar/Pedalino
*/

#include <Arduino.h>
#include <MIDI.h>
#include <MIDI_Controller.h>
//#include <menu.h>
#include <TimerOne.h>
//#include <menuIO/serialIO.h>
//#include <menuIO/chainStream.h>
//#include <menuIO/clickEncoderIn.h>
//#include "SerialMenu.h"

//using namespace Menu;
using namespace MIDI_CC;
using namespace MCU;

#define MAX_DEPTH 3
// #define SERIAL_BUFFER_SIZE 256

// Shift PWM
const int ShiftPWM_latchPin = 13;
const bool ShiftPWM_invertOutputs = true;
const bool ShiftPWM_balanceLoad = false;

#include <ShiftPWM.h> // include ShiftPWM.h after setting the pins!

unsigned char maxBrightness = 255;
unsigned char pwmFrequency = 75;
int numRegisters = 2;
//int numRGBleds = numRegisters*8/3;
int numRGBleds = 5;

uint8_t redVal;
uint8_t blueVal;
uint8_t greenVal;
uint8_t footSwitch;

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

// Serial File Listing /////////////////////////////////////
//result filePick(eventMask event, navNode &nav, prompt &item);

//SerialMenu filePickMenu("Backing Tracks", "/", filePick, enterEvent);

//implementing the handler here after filePick is defined...
// result filePick(eventMask event, navNode &nav, prompt &item)
// {
//   if (nav.root->navFocus == (navTarget *)&filePickMenu)
//   {
//     Serial.println(":play:" + filePickMenu.selectedFolder + filePickMenu.selectedFile);
//   }
//   return proceed;
// }

//result stopAudio(eventMask event, navNode &nav, prompt &item)
//{
//  switch (event)
//  {
//  case enterEvent:
//    Serial.println(":stop");
//  }
//  return proceed;
//}

// Encoder /////////////////////////////////////
// #define encA A1
// #define encB A0
// #define encBtn 2

// ClickEncoder clickEncoder(encA, encB, encBtn, 4);
// ClickEncoderStream encStream(clickEncoder, 1);
// void timerIsr() { clickEncoder.service(); }

//MENU(mainMenu, "Guitarix Pedalboard Menu", Menu::doNothing, Menu::noEvent, Menu::wrapStyle,
//     //SUBMENU(filePickMenu),
//     OP("Stop Backing Track", stopAudio, anyEvent),
//     OP("Sub2", Menu::doNothing, anyEvent),
//     OP("Sub3", Menu::doNothing, anyEvent),
//     EXIT("<Back\r\n"));
//
//serialIn serial(Serial);
//MENU_INPUTS(in,
//            //&encStream,
//            &serial);
//
//MENU_OUTPUTS(out, MAX_DEPTH, SERIAL_OUT(Serial), NONE);
//
//NAVROOT(nav, mainMenu, MAX_DEPTH, in, out);

void setup()
{
  Serial.begin(115200);
  Serial.println("Start");

  // Sets the number of 8-bit registers that are used.
  ShiftPWM.SetAmountOfRegisters(numRegisters);
  ShiftPWM.Start(pwmFrequency, maxBrightness);

  bank.add(knobs, Bank::CHANGE_ADDRESS);
  bank.add(sliders, Bank::CHANGE_ADDRESS);

  // Timer1.initialize(1000);
  // Timer1.attachInterrupt(timerIsr);
  //filePickMenu.begin();
}

int modifying = 0;
int currentState = 0;

void loop()
{
//  nav.poll();

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