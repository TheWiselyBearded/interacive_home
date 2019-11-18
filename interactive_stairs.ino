#include <FastLED.h>
#include <TimerOne.h>
#include <SPI.h>
#include "Neophob_LPD6803.h"

/*
 * The circuit:
  - LED attached from pin 13 to ground
  - pushbutton attached to pin 2 from +5V
  - 10K resistor attached to pin 2 from ground

  - Note: on most Arduinos there is already an LED on the board
    attached to pin 13.

 * 
 */

#define LED_PIN     6
#define NUM_LEDS    90
#define BRIGHTNESS  64
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define UPDATES_PER_SECOND 400

#define LED_MODULES 50

CRGB leds[NUM_LEDS];
Neophob_LPD6803 strip = Neophob_LPD6803(LED_MODULES);


// constants won't change. They're used here to set pin numbers:
const int buttonPin = 2;       // the number of the pushbutton pin
const int soundsensor = 3;     // Sound impact sensor st_00017.
const int led = 8;            // the number of the LED pin
int PIR_Bottom = 0;          // PIR Out pin 
int PIR_Middle = 1;
int PIR_Top = 4;              

// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status
int sys_state = 0;
int sound_impact  = 0;
int PIR_Bottom_S = 0;       // PIR status       
int PIR_Middle_S = 0;
int PIR_Top_S = 0;        

CRGBPalette16 currentPalette;
TBlendType    currentBlending;
extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

void setup() { 
  // Define inputs.
  pinMode(buttonPin, INPUT);  
  pinMode(soundsensor, INPUT);
  pinMode(PIR_Bottom, INPUT);       
  pinMode(PIR_Top, INPUT);  
  pinMode (led, OUTPUT);
  // Configure LED strips
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
  strip.setCPUmax(50);
  strip.begin(SPI_CLOCK_DIV64);
  strip.show();
  off_state();            // Turn all strips off.
  Serial.begin(9600);  
}

void loop() {  
  sys_state = check_button();

  if (sys_state == 0) {
    off_state();
  } else if (sys_state == 1) {
    music_response();
    // Have musical pattern going off in background for other strip.
    ChangePalettePeriodically();
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* motion speed */
    //Serial.println(startIndex, DEC);
    FillLEDsFromPaletteColors( startIndex);  
    FastLED.show();
    FastLED.delay(500 / UPDATES_PER_SECOND);
  } else if (sys_state == 2) {
    check_motion();    
  }
     
} 

/*
 * if the sound intensity is higher than threshold which is set by us, 
 * then sensor would return the value as 1.
 */
void music_response() {
  sound_impact = digitalRead (soundsensor);                                                                    
  if (sound_impact == 1) {
    //digitalWrite(led, HIGH);    
    uint8_t rgb_chann = random(0, 5);
    if (rgb_chann == 0) { flashColor(Color(255, 0, 0), 10); }
    else if (rgb_chann == 1) { flashColor(Color(0, 255, 0), 10); }
    else if (rgb_chann == 2) { flashColor(Color(0, 0, 255), 10); }
    else if (rgb_chann == 3) { colorWipe(Color(255, 0, 0), 15,0,1); }
    else if (rgb_chann == 4) { colorWipe(Color(0, 0, 255), 15,0,1); }
    //else if (rgb_chann == 3) { rainbowCycle(10,1); }
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1;            /* motion speed */    
    FillLEDsFromPaletteColors( startIndex);
    FastLED.show();
    FastLED.delay(100 / UPDATES_PER_SECOND);
  } else {
    digitalWrite(led, LOW);
    flashColor(Color(0, 0, 0), 50);
  }
}

/*  Black/wipe out everything. */
void off_state() {
  Serial.println("Off state");
  digitalWrite(led, LOW);      
  colorWipe(Color(0, 0, 0), 0,0,0);
  fill_solid( currentPalette, 16, CRGB::Black);
  FastLED.show();
  currentPalette = RainbowStripeColors_p;   
  currentBlending = NOBLEND;
}

void check_motion() {
  PIR_Top_S = digitalRead(PIR_Top);     
  if (PIR_Top_S == HIGH || PIR_Bottom_S == HIGH) {            
    Serial.println("Motion");   
    colorWipe(Color(255, 0, 0), 150,0,2);
    rainbowCycle(50, 2);
//    colorWipe(Color(0,0,0),0,0,2);    
  }  
}

void check_motion_old() {
  PIR_Top_S = digitalRead(PIR_Top);     
//  PIR_Middle_S = digitalRead(PIR_Middle);     
//  PIR_Bottom_S = digitalRead(PIR_Bottom);     

  if (PIR_Top_S == HIGH) {
    Serial.println("Motion");   
    colorWipe(Color(255, 0, 0), 10,1,2);
    rainbowCycle(50,2);    
    colorWipe(Color(0,0,0),0, 0,2);        
    currentPalette = RainbowColors_p;     // 2ND LED STRIP
    currentBlending = LINEARBLEND;
    static uint8_t startIndex = 255;
    startIndex = startIndex - 1;            /* motion speed */    
    FillLEDsFromPaletteColors( startIndex);
    FastLED.show();
    FastLED.delay(100 / UPDATES_PER_SECOND);
    //delay(1000);
  } else if (PIR_Middle_S == HIGH) {
     colorWipe(Color(0, 0, 255), 15,0,2);
     colorWipe(Color(0, 0, 0), 15,0,2);     
    currentPalette = RainbowColors_p;   // 2ND LED STRIP 
    currentBlending = LINEARBLEND;
    static uint8_t startIndex = 255;
    startIndex = startIndex - 1;            /* motion speed */    
    FillLEDsFromPaletteColors( startIndex);
    FastLED.show();
    FastLED.delay(100 / UPDATES_PER_SECOND);
  } else if (PIR_Bottom_S == HIGH) {    
     // 2ND LED STRIP
    currentPalette = RainbowColors_p;   
    currentBlending = LINEARBLEND;
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1;            /* motion speed */    
    FillLEDsFromPaletteColors( startIndex);
    FastLED.show();
    FastLED.delay(100 / UPDATES_PER_SECOND);
    colorWipe(Color(0, 0, 255), 15,0,2);
    colorWipe(Color(0, 0, 0), 15,0,2);
  } else {  
    Serial.println("No motion");
    off_state();  
  }
     
}

/*  Check if push button is pressed at any point. */
int check_button() {
  buttonState = digitalRead(buttonPin);
  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == HIGH) {
    sys_state = (sys_state + 1) % 3;
    Serial.println(sys_state, DEC);   
    digitalWrite(led, HIGH);     // turn LED on.
    delay(250);
    digitalWrite(led, LOW);      // turn LED off
  }
  return sys_state;
}

/*  Takes assigned global palette variable and populates pixels with color. */
void FillLEDsFromPaletteColors( uint8_t colorIndex) {
    uint8_t brightness = 255;   
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}


/*
 * There are several different palettes of colors demonstrated here.
 * 
 * FastLED provides several 'preset' palettes: RainbowColors_p, RainbowStripeColors_p,
 * OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.
 * 
 * Additionally, you can manually define your own color palettes, or you can write
 * code that creates color palettes on the fly.  All are shown here.
 */
 void ChangePalettePeriodically()  {
    uint8_t secondHand = (millis() / 1000) % 60;
    static uint8_t lastSecond = 99;
    
    if( lastSecond != secondHand) {
        lastSecond = secondHand;
        if( secondHand ==  0)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
        if( secondHand == 10)  { currentPalette = RainbowStripeColors_p;   currentBlending = NOBLEND;  }
        if( secondHand == 15)  { currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND; }
        if( secondHand == 20)  { SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND; }
        if( secondHand == 25)  { SetupTotallyRandomPalette();              currentBlending = LINEARBLEND; }
        if( secondHand == 30)  { SetupBlackAndWhiteStripedPalette();       currentBlending = NOBLEND; }
        if( secondHand == 35)  { SetupBlackAndWhiteStripedPalette();       currentBlending = LINEARBLEND; }
        if( secondHand == 40)  { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND; }
        if( secondHand == 45)  { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; }
        if( secondHand == 50)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = NOBLEND;  }
        if( secondHand == 55)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND; }
    }
}

/*  This function fills the palette with totally random colors. */
void SetupTotallyRandomPalette()  {
    for( int i = 0; i < 16; i++) {
        currentPalette[i] = CHSV( random8(), 255, random8());
    }
}

/*
 * This function sets up a palette of black and white stripes,
 * using code.  Since the palette is effectively an array of
 * sixteen CRGB colors, the various fill_* functions can be used
 * to set them up.
 */
void SetupBlackAndWhiteStripedPalette() {
    // 'black out' all 16 palette entries...
    // and set every fourth one to white.
    fill_solid( currentPalette, 16, CRGB::Black);    
    currentPalette[0] = CRGB::White;    
    currentPalette[4] = CRGB::White;
    currentPalette[8] = CRGB::White;
    currentPalette[12] = CRGB::White;    
}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette() {
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;
    
    currentPalette = CRGBPalette16(
                                   green,  green,  black,  black,
                                   purple, purple, black,  black,
                                   green,  green,  black,  black,
                                   purple, purple, black,  black );
}


// This example shows how to set up a static color palette
// which is stored in PROGMEM (flash), which is almost always more
// plentiful than RAM.  A static PROGMEM palette like this
// takes up 64 bytes of flash.
const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black
};



// Additional notes on FastLED compact palettes:
//
// Normally, in computer graphics, the palette (or "color lookup table")
// has 256 entries, each containing a specific 24-bit RGB color.  You can then
// index into the color palette using a simple 8-bit (one byte) value.
// A 256-entry color palette takes up 768 bytes of RAM, which on Arduino
// is quite possibly "too many" bytes.
//
// FastLED does offer traditional 256-element palettes, for setups that
// can afford the 768-byte cost in RAM.
//
// However, FastLED also offers a compact alternative.  FastLED offers
// palettes that store 16 distinct entries, but can be accessed AS IF
// they actually have 256 entries; this is accomplished by interpolating
// between the 16 explicit entries to create fifteen intermediate palette
// entries between each pair.
//
// So for example, if you set the first two explicit entries of a compact 
// palette to Green (0,255,0) and Blue (0,0,255), and then retrieved 
// the first sixteen entries from the virtual palette (of 256), you'd get
// Green, followed by a smooth gradient from green-to-blue, and then Blue.

void rainbow(uint8_t wait) {
  int i, j;   
  for (j=0; j < 96 * 3; j++) {     // 3 cycles of all 96 colors in the wheel
    for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel( (i + j) % 96));
    }  
    strip.show();   // write all the pixels out
    delay(wait);
  }
}

/*
 * Use each pixel as a fraction of the full 96 pixels
 * wheel (thats the i / strip.numPixels() part)
 * Then add in j which makes the colors go around 
 * per pixel the % 96 is to make the wheel cycle around.
 */
void rainbowCycle(uint8_t wait,uint8_t state) {
  int i, j;  
  //for (j=0; j < 96 * 5; j++) {     // 5 cycles of all 96 colors in the wheel
  for (j=0; j < 96; j++) {
    for (i=0; i < strip.numPixels(); i++) {      
      strip.setPixelColor(i, Wheel( ((i * 96 / strip.numPixels()) + j) % 96) );
      if (state == 1) { colorWipe(Color(0, 0, 0), 50,0,0); }
      if (check_button() != state) {  return; }
    }  
    strip.show();   // write all the pixels out
    delay(wait);
  }
}

/*  Fill all dots in at once with specified color */
void flashColor(uint32_t c, uint8_t wait) {
  int i;  
  for (i=0; i < strip.numPixels(); i++) {
      if (check_button() != 1) {  return; }
      strip.setPixelColor(i, c);      
  }
  strip.show();
  delay(wait);
}


/*  Fill dots one after other with specified color  */
void colorWipe(uint32_t c, uint8_t wait, uint8_t reverse_flag, uint8_t state_check) {
  int i;  
  for (i=0; i < strip.numPixels(); i++) {      
      if (reverse_flag == 1) {  strip.setPixelColor(50-i, c); } 
      else {                    strip.setPixelColor(i, c);  }      
      strip.show();
      delay(wait);    
      if (check_button() != state_check) {  return; }
  }  
}

/* 
 * Total of 15 bits, mask each section of bits accordingly.
 * G is bits 10-15, B is 5-10, r 0-5.
 */
unsigned int Color(byte r, byte g, byte b)  {
  //Take the lowest 5 bits of each value and append them end to end
  return( ((unsigned int)g & 0x1F )<<10 | ((unsigned int)b & 0x1F)<<5 | (unsigned int)r & 0x1F);
}


/*
 * Input a value 0 to 127 to get a color value.
 * The colours are a transition r - g -b - back to r
 */
unsigned int Wheel(byte WheelPos) {
  byte r,g,b;
  switch(WheelPos >> 5) {
    case 0:
      r=31- WheelPos % 32;   //Red down
      g=WheelPos % 32;      // Green up
      b=0;                  //blue off
      break; 
    case 1:
      g=31- WheelPos % 32;  //green down
      b=WheelPos % 32;      //blue up
      r=0;                  //red off
      break; 
    case 2:
      b=31- WheelPos % 32;  //blue down 
      r=WheelPos % 32;      //red up
      g=0;                  //green off
      break; 
  }
  return(Color(r,g,b));
}