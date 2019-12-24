#include "arduinoFFT.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for SSD1306 display connected using software SPI (default case):
#define OLED_MOSI   9
#define OLED_CLK   10
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
   
#define SAMPLES            128             //Must be a power of 2
#define SAMPLING_FREQUENCY 8000 //Hz, must be less than 10000 due to ADC
#define graphHeight        25

 
arduinoFFT FFT = arduinoFFT();
 
unsigned int sampling_period_us;
unsigned long microseconds;
 
double vRealR[SAMPLES];
double vImagR[SAMPLES];
double vRealL[SAMPLES];
double vImagL[SAMPLES];

int line1_i = 00;
int line2_i = 20;
int mapSize_i = graphHeight/SAMPLING_FREQUENCY;
 
void setup() {
    Serial.begin(115200);

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if(!display.begin(SSD1306_SWITCHCAPVCC)) {
      Serial.println(F("SSD1306 allocation failed"));
      for(;;); // Don't proceed, loop forever
    }
    //Serial.begin(921600);

    // Show initial display buffer contents on the screen --
    // the library initializes this with an Adafruit splash screen.
    //display.invertDisplay(true);
    
    // Clear the buffer
    display.clearDisplay();

    // Draw a single pixel in white
    //drawText("Hi Dave", 0, 0);
    //drawText("Peak: ", 0, 22);
    //display.drawPixel(10, 10, SSD1306_WHITE);
    //display.display();
 
    sampling_period_us = round(1000000*(1.0/SAMPLING_FREQUENCY));
}
 
void loop() {
   
    /*SAMPLING*/
    for(int i=0; i<SAMPLES; i++)
    {
        microseconds = micros();    //Overflows after around 70 minutes!
     
        vRealR[i] = analogRead(0);
        vImagR[i] = 0;
        vRealL[i] = analogRead(1);
        vImagL[i] = 0;
     
        while(micros() < (microseconds + sampling_period_us)){
        }
    }
 
    /*FFT*/
    FFT.Windowing(vRealR, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vRealR, vImagR, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vRealR, vImagR, SAMPLES);
    double peakR = FFT.MajorPeak(vRealR, SAMPLES, SAMPLING_FREQUENCY);

    
    FFT.Windowing(vRealL, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vRealL, vImagL, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vRealL, vImagL, SAMPLES);
    double peakL = FFT.MajorPeak(vRealL, SAMPLES, SAMPLING_FREQUENCY);
 
    /*PRINT RESULTS*/
    //Serial.print("*\n");
    //Serial.println(peak);     //Print out what frequency is the most dominant.
    display.clearDisplay();
    //drawText("Hi Dave", 0, 0);
    drawText("Peak R: ", 0, line1_i);
    drawText(String(int(peakR)), 80, line1_i);
    drawText("Peak L: ", 0, line2_i);
    drawText(String(int(peakL)), 80, line2_i);
        
    for(int i=0; i<(SAMPLES/2); i++)
    {
        /*View all these three lines in serial terminal to see which frequencies has which amplitudes*/
         
        //Serial.print((i * 1.0 * SAMPLING_FREQUENCY) / SAMPLES, 1);
        //Serial.print(" ");
        //Serial.println(vReal[i], 1);    //View only this line in serial plotter to visualize the bins
        //Serial.print(i);
        //Serial.print(":");       
        //Serial.print(vReal[i]);
        //Serial.print("\n");
        //0 to SAMPLE_FREQUENCY
        //38 to 63
        //int mapped = int(10*(63-38)*(vRealR[i]/SAMPLING_FREQUENCY))+38;
        int mappedR = int(-10*(63-38)*(vRealR[i]/SAMPLING_FREQUENCY))+65;
        int mappedL = int(-10*(63-38)*(vRealL[i]/SAMPLING_FREQUENCY))+65;
        display.drawPixel(i, mappedL, SSD1306_WHITE);
        display.drawPixel(i+64, mappedR, SSD1306_WHITE);
        //display.drawPixel(i, 38, SSD1306_WHITE);
        //display.drawPixel(i, 63, SSD1306_WHITE);
    }
 
    //delay(10);  //Repeat the process every second OR:
    //while(1);       //Run code once
    display.display();
    delay(100);
}

void drawText(String word, int x, int y)
{
  //display.clearDisplay();

  display.setTextSize(2);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(x, y);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  for (unsigned int i=0; i < word.length(); i++)
  {
    display.write(word.charAt(i));  
  }

  //display.display();
}

void testdrawchar(void) {
  display.clearDisplay();

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  // Not all the characters will fit on the display. This is normal.
  // Library will draw what it can and the rest will be clipped.
  for(int16_t i=0; i<256; i++) {
    if(i == '\n') display.write(' ');
    else          display.write(i);
  }

  display.display();
  delay(2000);
}
