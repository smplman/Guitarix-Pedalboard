/**
  Name: Guitarix-Pedalboard
  Purpose: MIDI Controller for Guitarix

  @author Stephen Peery
  @version 0.5 7/14/19
  @email smp4488_AT_gmail.com
  @github https://github.com/smp4488/Guitarix-Pedalboard
*/

/*
                           +--------+
                  +--------+ Micro  +--------+
                  |        | USB A/B|        |
                  |        +--------+        |
                  | [D]1/TX           VIN[ ] |
                  |                          |
                  | [D]0/RX           GND[ ] |
                  |                          |
                  | [ ]RST           AREF[ ] |
                  |                          |
                  | [ ]GND            VCC[ ] |
                  |              21/SCL[F]   |
                  | [E]2               A3[B] |
                  |              20/SDA[F]   |
                  |~[C]3   +--------+  A2[B] |
                  |        | SamD21 |        |
                  |~[C]4   |  G18   |  A1[B] |
                  |        |        |        |
                  |~[ ]5   +--------+  A0[B] |
                  |                          |
                  |~[A]6               13[A]~|
                  |        +--------+        |
                  | [D]7   | Prog   |  12[A]~|
                  |        | Header |        |
                  |~[C]8   +--------+  11[A]~|
                  |                          |
                  |~[C]9               10[A]~|
                  +--------------------------+
                  Sparkfun SAMD21 Mini Breakout
 */

//------------------------------------------------------------------
// [A]		Foot Switches
//------------------------------------------------------------------
// Connect commons to GND
#define FOOTSWITCH_1_PIN (10)
#define FOOTSWITCH_2_PIN (11)
#define FOOTSWITCH_3_PIN (6)
#define FOOTSWITCH_4_PIN (12)
#define FOOTSWITCH_5_PIN (13)

//------------------------------------------------------------------
// [B]		Rotary Potentiometers
//------------------------------------------------------------------
// Connect + to VCC and - to GND
#define ROTARY_1_PIN (A0)
#define ROTARY_2_PIN (A1)
#define ROTARY_3_PIN (A2)
#define ROTARY_4_PIN (A3)

//------------------------------------------------------------------
// [C]		Slide Potentiometers
//------------------------------------------------------------------
// Connect + to VCC and - to GND
#define SLIDE_1_PIN (A6)
#define SLIDE_2_PIN (A7)
#define SLIDE_3_PIN (A8)
#define SLIDE_4_PIN (A9)

//------------------------------------------------------------------
// [D]		Rotary Encoder
//------------------------------------------------------------------
// Connect + to VCC and - to GND
#define ENCODER_A_PIN (1)
#define ENCODER_B_PIN (0)
#define ENCODER_BTN_PIN (7)

//------------------------------------------------------------------
// [E]		LEDS
//------------------------------------------------------------------
// Connect + to VCC and - to GND
#define LED_DATA_PIN (2)

//------------------------------------------------------------------
// [F]		SSD1309 OLED Display
//------------------------------------------------------------------
// Connect + to VCC and - to GND
#define DISPLAY_SDA (PIN_WIRE_SDA) //(20)
#define DISPLAY_SCL (PIN_WIRE_SCL) //(21)

#include <Arduino.h>
#include <MIDI_Controller.h>
#include <menu.h>
#include <Adafruit_ZeroTimer.h>
#include <Adafruit_NeoPXL8.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <menuIO/u8g2Out.h>
#include <menuIO/serialIO.h>
#include <menuIO/chainStream.h>
#include <menuIO/clickEncoderIn.h>
#include <menuIO/jsonFmt.h>
#include "SerialMenu.h"
#include "watermark.h"

using namespace Menu;
using namespace MIDI_CC;
using namespace MCU;

Adafruit_ZeroTimer zt4 = Adafruit_ZeroTimer(4);

#define NUM_LED 5
int8_t pins[1] = {LED_DATA_PIN};
Adafruit_NeoPXL8 leds(NUM_LED, pins, NEO_GRB);

#define MAX_DEPTH 3

uint8_t redVal;
uint8_t blueVal;
uint8_t greenVal;
uint8_t footSwitch;

// Create Bank
Bank bank(4);
const uint8_t channel = 1;

// Foot switches
const uint8_t footSwitchPins[5] = {10, 11, 6, 12, 13};
uint8_t footSwitchState[5] = {0, 0, 0, 0, 0};
DigitalCC footSwitches[] = {
  {FOOTSWITCH_1_PIN, MIDI_CC::Effects_1, channel},
  {FOOTSWITCH_2_PIN, MIDI_CC::Effects_2, channel},
  {FOOTSWITCH_3_PIN, MIDI_CC::Effects_3, channel},
  {FOOTSWITCH_4_PIN, MIDI_CC::Effects_4, channel},
  {FOOTSWITCH_5_PIN, MIDI_CC::Effects_5, channel},
};

// Rotary Potentiometer
AnalogCC knobs[] = {
  {ROTARY_1_PIN, MIDI_CC::General_Purpose_Controller_1, channel},
  {ROTARY_2_PIN, MIDI_CC::General_Purpose_Controller_2, channel},
  {ROTARY_3_PIN, MIDI_CC::General_Purpose_Controller_3, channel},
  {ROTARY_4_PIN, MIDI_CC::General_Purpose_Controller_4, channel}
};

// Slide Potentiometers
AnalogCC sliders[] = {
  {SLIDE_1_PIN, MIDI_CC::General_Purpose_Controller_5, channel}, //A6,3
  {SLIDE_2_PIN, MIDI_CC::General_Purpose_Controller_6, channel}, //A7,4
  {SLIDE_3_PIN, MIDI_CC::General_Purpose_Controller_7, channel}, //A8,8
  {SLIDE_4_PIN, MIDI_CC::General_Purpose_Controller_8, channel} //A9,9
};

// SSD1309 OLED Display
U8G2_SSD1309_128X64_NONAME0_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, DISPLAY_SCL, DISPLAY_SDA);
// #define fontName u8g2_font_5x7_tf
#define fontName u8g2_font_tinytim_tf
#define fontX 5
#define fontY 9
#define offsetX 0
#define offsetY 0
#define U8_Width 128
#define U8_Height 64

// define menu colors --------------------------------------------------------
//each color is in the format:
//  {{disabled normal,disabled selected},{enabled normal,enabled selected, enabled editing}}
// this is a monochromatic color table
const colorDef<uint8_t> colors[] MEMMODE = {
    {{0, 0}, {0, 1, 1}}, //bgColor
    {{1, 1}, {1, 0, 0}}, //fgColor
    {{1, 1}, {1, 0, 0}}, //valColor
    {{1, 1}, {1, 0, 0}}, //unitColor
    {{0, 1}, {0, 0, 1}}, //cursorColor
    {{1, 1}, {1, 0, 0}}, //titleColor
};

// Serial File Listing /////////////////////////////////////
result filePick(eventMask event, navNode &nav, prompt &item);
SerialMenu filePickMenu("Backing Tracks", "/backing-tracks/", filePick, enterEvent);

// implementing the handler here after filePick is defined...
result filePick(eventMask event, navNode &nav, prompt &item){
  if (nav.root->navFocus == (navTarget *)&filePickMenu){
    SerialUSB.println("::play::" + filePickMenu.selectedFolder + filePickMenu.selectedFile);
  }
  return proceed;
}

// Bank Listing /////////////////////////////////////
result banksPick(eventMask event, navNode &nav, prompt &item);
SerialMenu banksPickMenu("Banks", "/banks/", banksPick, enterEvent);

// implementing the handler here after banksPick is defined...
result banksPick(eventMask event, navNode &nav, prompt &item)
{
  if (nav.root->navFocus == (navTarget *)&banksPickMenu){
    uint8_t bankIdx = banksPickMenu.parentIdx;
    uint8_t programIdx = nav.sel - 1; // Zero Indexed

    MIDI_Controller.MIDI()->send(CONTROL_CHANGE, channel, 0X00, 0X00); // Bank select MSB
    MIDI_Controller.MIDI()->send(CONTROL_CHANGE, channel, 0X20, bankIdx); // Bank select LSB
    MIDI_Controller.MIDI()->send(PROGRAM_CHANGE, channel, programIdx); // Program/Preset change
  }

  return proceed;
}

result stopAudio(eventMask event, navNode &nav, prompt &item){
  switch (event){
    case enterEvent:
      SerialUSB.println("::stop");
  }
  return proceed;
}

/*
0: loading
1: effects
2: looper
3: tuner
4: test
 */

uint8_t mode = 0;
bool navEnabled = false;

result displayTest(eventMask event, navNode &nav, prompt &item){
  switch (event)
  {
    case enterEvent:
      navEnabled = false;
      mode = 4;
      break;
      // nav.idleOn();
      // SerialUSB.println("display");
      // u8g2.setFont(u8g2_font_ncenB14_tr);

      //u8g2.drawStr(0, 24, "Hello World!");

    case exitEvent:
      u8g2.setFont(fontName);
      navEnabled = true;
      break;
      // idleOff();
  }

  return proceed;
}

long prevTime = 0;
uint8_t notePos = 0;
char* notes[7] = {"A","B","C","D","E","F","G"};

void displayTestShow(){
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.setFontPosCenter();
  // Draw flat (b) and sharp (#)
  u8g2.drawStr(22 - (u8g2.getStrWidth("b")/2), U8_Height * .25, "b");
  u8g2.drawStr(106 - (u8g2.getStrWidth("#")/2), U8_Height * .25, "#");

  u8g2.drawCircle(U8_Width/2, U8_Height, 50);

  // Draw note
  // long currentTime = millis();
  if(millis() - prevTime > 2000){ // Change note every 2 seconds
    prevTime = millis();
    if(notePos < 6){
      notePos++;
    } else {
      notePos = 0;
    }
  }

  //Draw note
  u8g2.drawStr(U8_Width / 2 - (u8g2.getStrWidth(notes[notePos]) / 2), U8_Height - 14, notes[notePos]);

  // Draw refernce lines
  // u8g2.drawLine(U8_Width/2, 0, U8_Width/2, U8_Height);
  // u8g2.drawLine(0, U8_Height/2, U8_Width, U8_Height/2);
}

MENU(mainMenu, "Guitarix Pedalboard Menu", Menu::doNothing, Menu::noEvent, Menu::wrapStyle,
  SUBMENU(filePickMenu),
  SUBMENU(banksPickMenu),
  OP("Stop Backing Track", stopAudio, anyEvent),
  OP("Looper", Menu::doNothing, anyEvent),
  OP("Tuner", Menu::doNothing, anyEvent),
  // OP("Display Test", displayTest, anyEvent),
  EXIT("<Back\r\n")
);

// Encoder /////////////////////////////////////
ClickEncoder clickEncoder(ENCODER_A_PIN, ENCODER_B_PIN, ENCODER_BTN_PIN, 4);
ClickEncoderStream encStream(clickEncoder, 1);
void timerIsr() { clickEncoder.service(); }
void TC4_Handler() { Adafruit_ZeroTimer::timerHandler(4); }

serialIn serial(SerialUSB);
MENU_INPUTS(in, &serial, &encStream);
MENU_OUTPUTS(out, MAX_DEPTH, U8G2_OUT(u8g2, colors, fontX, fontY, offsetX, offsetY, {0, 0, U8_Width / fontX, U8_Height / fontY}), SERIAL_OUT(SerialUSB));

NAVROOT(nav, mainMenu, MAX_DEPTH, in, out);

// idx_t web_tops[MAX_DEPTH] = {0};
// PANELS(webPanels, {0, 0, 80, 100});
// jsonFmt<serialOut> jsonOut(SerialUSB, web_tops);

bool loaded = false;

void doDisplay(){
  // Disable SFL if not connected
  mainMenu[0].enabled = (SerialUSB ? enabledStatus : disabledStatus);
  mainMenu[1].enabled = (SerialUSB ? enabledStatus : disabledStatus);

  // TODO: Exit back to main on serial disconnect

  if(!loaded && SerialUSB){
    loaded = true;
    mode = 1;
  }

  nav.doInput();

  if (nav.sleepTask || !navEnabled)
  {
    u8g2.firstPage();
    do {
      /*
      0: loading
      1: effects
      2: looper
      3: tuner
      4: test
      */

      switch (mode)
      {
      case 0:
        // Draw loading logo
        u8g2.drawXBMP(0, 0, watermark_width, watermark_height, watermark_bits);
        break;
      case 1:
        nav.doOutput();
        break;
      case 2:

        break;
      case 3:
        // displayTuner();
        break;
      case 4:
        displayTestShow();
        break;

      default:
        break;
      }
    } while ( u8g2.nextPage() );
  } else {
    //if (nav.changed(0)) {
      u8g2.firstPage();
      u8g2.setFont(fontName);
      u8g2.setFontPosBottom();
      do {
        // nav.doOutput();
        // u8g2.drawXBMP(0, 0, watermark_width, watermark_height, watermark_bits);
      } while(u8g2.nextPage());
    //}
  }
}

uint8_t modifying = 0;

void readFootSwitches(){
  // Read footswitches
  for (uint8_t i = 0; i < 5; i++)
  {
    uint8_t currentState = digitalRead(footSwitchPins[i]);

    // Set LED color if the effect is turned on or off or blue for modifying
    if (modifying == i && currentState == LOW)
    {
      // Set LED to blue so we know which effect we are modifying
      leds.setPixelColor(i, 0, 0, 255);
    }
    else if (currentState == LOW)
    {
      leds.setPixelColor(i, 0, 255, 0); // Green
    }
    else
    {
      leds.setPixelColor(i, 255, 0, 0); // Red
    }

    // Set MIDI Channel if an effect is turned on by foot switch
    if (footSwitchState[i] == HIGH && currentState == LOW)
    {
      // SerialUSB.println(i);
      bank.setBankSetting(i);
      modifying = i;
    }

    footSwitchState[i] = currentState;
  }
}

void setup(){
  SerialUSB.begin(115200);

  bank.add(knobs, Bank::CHANGE_ADDRESS);
  bank.add(sliders, Bank::CHANGE_ADDRESS);

  // LEDs
  leds.begin();
  leds.setBrightness(30);

  /********************* Timer #4, 8 bit, one callback with adjustable period */
  zt4.configure(TC_CLOCK_PRESCALER_DIV64,     // prescaler
                TC_COUNTER_SIZE_32BIT,        // bit width of timer/counter
                TC_WAVE_GENERATION_MATCH_PWM // match style
  );
  zt4.setPeriodMatch(500, 100, 0);                                 // 1 match, channel 0
  zt4.setCallback(true, TC_CALLBACK_CC_CHANNEL0, timerIsr);        // set DAC in the callback
  zt4.enable(true);

  delay(1000);

  u8g2.begin();
  u8g2.setFont(fontName);
}

void loop(){
  leds.show();
  // nav.poll();
  doDisplay();
  readFootSwitches();
  MIDI_Controller.refresh();
}
