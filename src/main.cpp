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
                  | [E]2               A3[C] |
                  |              20/SDA[F]   |
                  |~[B]3   +--------+  A2[C] |
                  |        | SamD21 |        |
                  |~[B]4   |  G18   |  A1[C] |
                  |        |        |        |
                  |~[ ]5   +--------+  A0[C] |
                  |                          |
                  |~[A]6               13[A]~|
                  |        +--------+        |
                  | [D]7   | Prog   |  12[A]~|
                  |        | Header |        |
                  |~[B]8   +--------+  11[A]~|
                  |                          |
                  |~[B]9               10[A]~|
                  +--------------------------+
                  Sparkfun SAMD21 Mini Breakout
 */

//------------------------------------------------------------------
// [A]		Foot Switches
//------------------------------------------------------------------
// Connect commons to GND
#define FOOTSWITCH_1_PIN (6)
#define FOOTSWITCH_2_PIN (10)
#define FOOTSWITCH_3_PIN (11)
#define FOOTSWITCH_4_PIN (12)
#define FOOTSWITCH_5_PIN (13)

//------------------------------------------------------------------
// [B]		Rotary Potentiometers
//------------------------------------------------------------------
// Connect + to VCC and - to GND
#define ROTARY_1_PIN (3)
#define ROTARY_2_PIN (4)
#define ROTARY_3_PIN (8)
#define ROTARY_4_PIN (9)

//------------------------------------------------------------------
// [C]		Slide Potentiometers
//------------------------------------------------------------------
// Connect + to VCC and - to GND
#define SLIDE_1_PIN (A0)
#define SLIDE_2_PIN (A1)
#define SLIDE_3_PIN (A2)
#define SLIDE_4_PIN (A3)


//------------------------------------------------------------------
// [D]		Rotary Encoder
//------------------------------------------------------------------
// Connect + to VCC and - to GND
#define ENCODER_A_PIN (9)//(1)
#define ENCODER_B_PIN (8)//(0)
#define ENCODER_BTN_PIN (7)//(7)

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

#define DISPLAY_CLOCK (13) //(23)//(11)
#define DISPLAY_DATA (11) //(22)//(12)
#define DISPLAY_CS (10) //(13)
// #define DISPLAY_DC (A0)

/*
SSD1309 128X64_NONAME2 https://github.com/olikraus/u8g2/wiki/u8g2setupcpp#ssd1309-128x64_noname2

SPI full framebuffer, size = 1024 bytes
U8G2_SSD1309_128X64_NONAME2_F_4W_SW_SPI(rotation, clock, data, cs, dc[, reset])

I2C full framebuffer, size = 1024 bytes
U8G2_SSD1309_128X64_NONAME2_F_SW_I2C(rotation, clock, data [, reset])
 */

/*
SSD1309 128X64_NONAME0 https://github.com/olikraus/u8g2/wiki/u8g2setupcpp#ssd1309-128x64_noname0

SPI full framebuffer, size = 1024 bytes
U8G2_SSD1309_128X64_NONAME0_F_4W_SW_SPI(rotation, clock, data, cs, dc [, reset])

I2C full framebuffer, size = 1024 bytes
U8G2_SSD1309_128X64_NONAME0_F_SW_I2C(rotation, clock, data [, reset])

*/

// #define ARDUINO_SAMD_ZERO
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
const int footSwitchPins[5] = {6, 10, 11, 12, 13};
int footSwitchState[5] = {0, 0, 0, 0, 0};
// DigitalCC footSwitches[] = {
//   {6, MIDI_CC::Effects_1, channel},
//   {10, MIDI_CC::Effects_2, channel},
//   {11, MIDI_CC::Effects_3, channel},
//   {12, MIDI_CC::Effects_4, channel},
//   {13, MIDI_CC::Effects_5, channel},
// };

// Rotary Potentiometer
// AnalogCC knobs[] = {
//   {3, MIDI_CC::General_Purpose_Controller_1, channel},
//   {4, MIDI_CC::General_Purpose_Controller_2, channel},
//   {8, MIDI_CC::General_Purpose_Controller_3, channel},
//   {9, MIDI_CC::General_Purpose_Controller_4, channel}
// };

// Slide Potentiometers
// AnalogCC sliders[] = {
//   {A0, MIDI_CC::General_Purpose_Controller_5, channel},
//   {A1, MIDI_CC::General_Purpose_Controller_6, channel},
//   {A2, MIDI_CC::General_Purpose_Controller_7, channel},
//   {A3, MIDI_CC::General_Purpose_Controller_8, channel}
// };

// SSD1309 OLED Display
// U8G2_SSD1309_128X64_NONAME2_1_4W_SW_SPI u8g2(U8G2_R0, DISPLAY_CLOCK, DISPLAY_DATA, DISPLAY_CS, DISPLAY_DC);
// U8G2_SSD1309_128X64_NONAME0_F_SW_I2C u8g2(U8G2_R0, DISPLAY_SCL, DISPLAY_SDA);
U8G2_SSD1309_128X64_NONAME0_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, DISPLAY_SCL, DISPLAY_SDA);
#define fontName u8g2_font_5x7_tf
// #define fontName u8g2_font_chroma48medium8_8r
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
SerialMenu filePickMenu("Backing Tracks", "/", filePick, enterEvent);

// implementing the handler here after filePick is defined...
result filePick(eventMask event, navNode &nav, prompt &item){
  if (nav.root->navFocus == (navTarget *)&filePickMenu){
    SerialUSB.println("::play::" + filePickMenu.selectedFolder + filePickMenu.selectedFile);
  }
  switch (event)
  {
  case enterEvent:
    u8g2.clearDisplay();
    u8g2.clearBuffer();
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

bool navEnabled = true;

result displayTest(eventMask event, navNode &nav, prompt &item){
  switch (event)
  {
    case enterEvent:
      navEnabled = false;
      SerialUSB.println("display");
      // u8g2.setFont(u8g2_font_ncenB14_tr);

      u8g2.drawStr(0, 24, "Hello World!");

    case exitEvent:
      navEnabled = true;
  }

  return proceed;
}

MENU(mainMenu, "Guitarix Pedalboard Menu", Menu::doNothing, Menu::noEvent, Menu::wrapStyle,
  SUBMENU(filePickMenu),
  OP("Stop Backing Track", stopAudio, anyEvent),
  OP("Looper", Menu::doNothing, anyEvent),
  OP("Tuner", Menu::doNothing, anyEvent),
  OP("Display Test", displayTest, anyEvent),
  EXIT("<Back\r\n")
);

// Encoder /////////////////////////////////////
ClickEncoder clickEncoder(ENCODER_A_PIN, ENCODER_B_PIN, ENCODER_BTN_PIN, 4);
ClickEncoderStream encStream(clickEncoder, 1);
void timerIsr() { clickEncoder.service(); }
void TC4_Handler() { Adafruit_ZeroTimer::timerHandler(4); }

// idx_t web_tops[MAX_DEPTH];
// PANELS(webPanels, {0, 0, 80, 100});
// jsonFmt<serialOut> jsonOut(SerialUSB, web_tops);

serialIn serial(SerialUSB);
MENU_INPUTS(in, &serial, &encStream);
MENU_OUTPUTS(out, MAX_DEPTH, U8G2_OUT(u8g2, colors, fontX, fontY, offsetX, offsetY, {0, 0, U8_Width / fontX, U8_Height / fontY}), SERIAL_OUT(SerialUSB));

NAVROOT(nav, mainMenu, MAX_DEPTH, in, out);


void setup(){
  SerialUSB.begin(115200);
  while(!SerialUSB);
  SerialUSB.println("Start");

  Wire.begin();

  // bank.add(knobs, Bank::CHANGE_ADDRESS);
  // bank.add(sliders, Bank::CHANGE_ADDRESS);
  // filePickMenu.begin();

  // LEDs
  leds.begin();
  leds.setBrightness(30);
  // leds.setPixelColor(0, 0, 255, 0);

  /********************* Timer #4, 8 bit, one callback with adjustable period */
  zt4.configure(TC_CLOCK_PRESCALER_DIV64,     // prescaler
                TC_COUNTER_SIZE_32BIT,        // bit width of timer/counter
                TC_WAVE_GENERATION_MATCH_PWM // match style
  );
  zt4.setPeriodMatch(500, 100, 0);                                 // 1 match, channel 0
  zt4.setCallback(true, TC_CALLBACK_CC_CHANNEL0, timerIsr);        // set DAC in the callback
  zt4.enable(true);

  //u8g2.setI2CAddress(0x78);  //if DC is pulled low
  // u8g2.setI2CAddress(0x7A);  //if DC is pulled high
  // u8g2.setBusClock(1500000);
  u8g2.begin();
  u8g2.setFont(fontName);
}

uint8_t modifying = 0;
uint8_t currentState = 0;

void loop(){

  // scanI2c();
  leds.show();
  // nav.poll();

  // nav.doInput();
  // if (nav.changed(0))
  // { //only draw if menu changed for gfx device
  //   //change checking leaves more time for other tasks
    u8g2.firstPage();
    do
      if (navEnabled) nav.poll();
    while (u8g2.nextPage());
  // }

  // Read footswitches
  for (uint8_t i = 0; i < 5; i++){
    currentState = digitalRead(footSwitchPins[i]);

    // Set LED color if the effect is turned on or off or blue for modifying
    if (modifying == i && currentState == LOW){
      // Set LED to blue so we know which effect we are modifying
      leds.setPixelColor(i, 0, 0, 255);
    }
    else if (currentState == LOW){
      leds.setPixelColor(i, 0, 255, 0);
    }
    else{
      leds.setPixelColor(i, 255, 0, 0);
    }

    // Set MIDI Channel if an effect is turned on by foot switch
    if (footSwitchState[i] == HIGH && currentState == LOW)
    {
      SerialUSB.println(i);
      bank.setBankSetting(i);
      modifying = i;
    }

    footSwitchState[i] = currentState;
  }

    // Refresh the button (check whether the button's state has changed since last time, if so, send it over MIDI)
    MIDI_Controller.refresh();
}
