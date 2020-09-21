/*
 * Animated GIFs Display Code for SmartMatrix and 32x32 RGB LED Panels
 *
 * Uses SmartMatrix Library for Teensy 3.1 written by Louis Beaudoin at pixelmatix.com
 *
 * Written by: Craig A. Lindley
 *
 * Copyright (c) 2014 Craig A. Lindley
 * Refactoring by Louis Beaudoin (Pixelmatix)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * This example displays 32x32 GIF animations loaded from a SD Card connected to the Teensy 3.1
 * The GIFs can be up to 32 pixels in width and height.
 * This code has been tested with 32x32 pixel and 16x16 pixel GIFs, but is optimized for 32x32 pixel GIFs.
 *
 * Wiring is on the default Teensy 3.1 SPI pins, and chip select can be on any GPIO,
 * set by defining SD_CS in the code below
 * Function     | Pin
 * DOUT         |  11
 * DIN          |  12
 * CLK          |  13
 * CS (default) |  15
 *
 * Wiring for ESP32 follows the default for the ESP32 SD Library, see: https://github.com/espressif/arduino-esp32/tree/master/libraries/SDis is on the default Teensy 3.1 SPI pins, and chip select can be on any GPIO,
 *
 * This code first looks for .gif files in the /gifs/ directory
 * (customize below with the GIF_DIRECTORY definition) then plays random GIFs in the directory,
 * looping each GIF for DISPLAY_TIME_SECONDS
 *
 * This example is meant to give you an idea of how to add GIF playback to your own sketch.
 * For a project that adds GIF playback with other features, take a look at
 * Light Appliance and Aurora:
 * https://github.com/CraigLindley/LightAppliance
 * https://github.com/pixelmatix/aurora
 *
 * If you find any GIFs that won't play properly, please attach them to a new
 * Issue post in the GitHub repo here:
 * https://github.com/pixelmatix/AnimatedGIFs/issues
 */

/*
 * CONFIGURATION:
 *  - If you're using SmartLED Shield V4 (or above), uncomment the line that includes <SmartMatrixShieldV4.h>
 *  - update the "SmartMatrix configuration and memory allocation" section to match the width and height and other configuration of your display
 *  - Note for 128x32 and 64x64 displays with Teensy 3.2 - need to reduce RAM:
 *    set kRefreshDepth=24 and kDmaBufferRows=2 or set USB Type: "None" in Arduino,
 *    decrease refreshRate in setup() to 90 or lower to get good an accurate GIF frame rate
 *  - Set the chip select pin for your board.  On Teensy 3.5/3.6, the onboard microSD CS pin is "BUILTIN_SDCARD"
 *  - For ESP32 and large panels, you don't need to lower the refreshRate, but you can lower the frameRate (number of times the refresh buffer
 *    is updaed with new data per second), giving more time for the CPU to decode the GIF.
 *    Use matrix.setMaxCalculationCpuPercentage() or matrix.setCalcRefreshRateDivider()
 */

#define NEOMATRIX
#include "GifAnim_Impl.h"
// openGifFilenameByIndex computes this for us, steal the value back
// without changing the calling interface which did not contain returning
// the pathname.
extern char pathname[];

// If the matrix is a different size than the GIFs, allow panning through the GIF
// while displaying it, or bouncing it around if it's smaller than the display
int offsetx_default;
int offsety_default;
// If the display is not square but the gifs are close to square, allow stretching
// the gif in the X and/or Y direction (10 for 1X, 15 for 1.5X)
int factx_default;
int facty_default;

// GifAnim_Impl uses these globals for each frame displays
// These default to the _default value for each new gif.
int OFFSETX;
int OFFSETY;
int FACTX;
int FACTY;

// player allows changing offsets and size for each gif after it started playing.
int offsetx_now;
int offsety_now;
int factx_now;
int facty_now;


int num_files;

// Setup method runs once, when the sketch starts
void setup() {
    // Wait before serial on teensy
#ifdef KINETISK
    delay(6000);
#endif

    
    // If the matrix is a different size than the GIFs, allow panning through the GIF
    // while displaying it, or bouncing it around if it's smaller than the display
    offsetx_default = 0;
    offsety_default = 0;
    // If the display is not square but the gifs are close to square, allow stretching
    // the gif in the X and/or Y direction (10 for 1X, 15 for 1.5X)
    factx_default   = 10;
    facty_default   = 10;

    if (kMatrixWidth == 24 && kMatrixHeight == 32) offsetx_default = -4;


    Serial.println("Starting AnimatedGIFs Sketch");
    sav_setup();

    // GifAnim_Impl uses these globals for each frame displays
    OFFSETX = offsetx_default;
    OFFSETY = offsety_default;
    FACTX   = factx_default;
    FACTY   = facty_default; 

    // Seed the random number generator
    // This breaks SmartMatrix output on ESP32
    //randomSeed(analogRead(14));

    #if defined(ESP8266)
	Serial.println();
	Serial.print( F("Heap: ") ); Serial.println(system_get_free_heap_size());
	Serial.print( F("Boot Vers: ") ); Serial.println(system_get_boot_version());
	Serial.print( F("CPU: ") ); Serial.println(system_get_cpu_freq());
	Serial.print( F("SDK: ") ); Serial.println(system_get_sdk_version());
	Serial.print( F("Chip ID: ") ); Serial.println(system_get_chip_id());
	Serial.print( F("Flash ID: ") ); Serial.println(spi_flash_get_id());
	Serial.print( F("Flash Size: ") ); Serial.println(ESP.getFlashChipRealSize());
	Serial.print( F("Vcc: ") ); Serial.println(ESP.getVcc());
	Serial.println();
    #endif
    #if ENABLE_SCROLLING == 1
	matrix.addLayer(&scrollingLayer); 
    #endif

    // for ESP32 we need to allocate SmartMatrix DMA buffers after initializing
    // the SD card to avoid using up too much memory
    // Determine how many animated GIF files exist
    num_files = enumerateGIFFiles(GIF_DIRECTORY, true);

    if(num_files < 0) {
#if ENABLE_SCROLLING == 1
        scrollingLayer.start("No gifs directory", -1);
#endif
        die("No gifs directory");
    }

    if(!num_files) {
#if ENABLE_SCROLLING == 1
        scrollingLayer.start("Empty gifs directory", -1);
#endif
        die("Empty gifs directory");
    }
    Serial.print("Index of files: 0 to ");
    Serial.println(num_files);
    // At least on teensy, due to some framework bug it seems, early
    // serial output gets looped back into serial input
    // Hence, flush input.
    while(Serial.available() > 0) { char t = Serial.read(); t=t; }
}

void adjust_gamma(float change) {
#ifdef NEOMATRIX
    matrix_gamma += change;
    matrix->precal_gamma(matrix_gamma);
    Serial.print("Change gamma to: "); 
    Serial.println(matrix_gamma); 
#else
    Serial.println("Gamma changing not supported in SmartMatrix lib"); 
#endif
}

void gifname_offset_ratio() {
    screenClearCallback();
    Serial.print(pathname);
    Serial.print(" : FactX: ");
    Serial.print(factx_now);
    Serial.print(", FactY: ");
    Serial.print(facty_now);
    Serial.print(", OffsetX: ");
    Serial.print(offsetx_now);
    Serial.print(", OffsetY: ");
    Serial.print(offsety_now);
    Serial.println("");
}

void loop() {
    static unsigned long lastTime = millis();
    static int index = FIRSTINDEX;
    static int8_t new_file = 1;
    static uint16_t frame = 0;
    // allow stalling on a picture if requested
    static uint32_t longer = 0;
    char readchar;
    // frame by frame display
    static bool debugframe = false;
    bool gotnf = false;
    // clear display before each frame
    static bool clear = false;

    if (Serial.available()) readchar = Serial.read(); else readchar = 0;

    switch(readchar) {
    case 'n': 
	Serial.println("Serial => next"); 
	new_file = 1;  
	index++;
	break;

    case 'p':
	Serial.println("Serial => previous");
	new_file = 1;
	index--;
	break;

    case 'f':
	Serial.println("Serial => debug frames, press 'g' for next frame");
	debugframe = true;
	longer = 3600; //  if frame debugging, keep current gif for 1h
	break;

    case 'g':
	Serial.println("Serial => next frame");
	gotnf = true;
	break;

    case 'c':
	Serial.print("Toggle clear screen: ");
	clear = !clear;;
	Serial.println(clear);
	break;

    case 'x':
	Serial.print("Toggle horizontal zoom down: ");
	switch (factx_now) {
	    case 10: factx_now =  9; break;
	    case  9: factx_now =  8; break;
	    case  8: factx_now =  7; break;
	    case  7: factx_now =  6; break;
	    case  6: factx_now = 15; break;
	    case 15: factx_now = 12; break;
	    case 12: factx_now = 10; break;
	}
	Serial.println(factx_now);
	gifname_offset_ratio();
	break;

    case 'X':
	Serial.print("Toggle horizontal zoom up: ");
	switch (factx_now) {
	    case  9: factx_now = 10; break;
	    case  8: factx_now =  9; break;
	    case  7: factx_now =  8; break;
	    case  6: factx_now =  7; break;
	    case 15: factx_now =  6; break;
	    case 12: factx_now = 15; break;
	    case 10: factx_now = 12; break;
	}
	Serial.println(factx_now);
	gifname_offset_ratio();
	break;

    case 'y':
	Serial.print("Toggle vertical zoom down: ");
	switch (facty_now) {
	    case 10: facty_now =  9; break;
	    case  9: facty_now =  8; break;
	    case  8: facty_now =  7; break;
	    case  7: facty_now =  6; break;
	    case  6: facty_now = 15; break;
	    case 15: facty_now = 12; break;
	    case 12: facty_now = 10; break;
	}
	Serial.println(facty_now);
	gifname_offset_ratio();
	break;

    case 'Y':
	Serial.print("Toggle vertical zoom up: ");
	switch (facty_now) {
	    case  9: facty_now = 10; break;
	    case  8: facty_now =  9; break;
	    case  7: facty_now =  8; break;
	    case  6: facty_now =  7; break;
	    case 15: facty_now =  6; break;
	    case 12: facty_now = 15; break;
	    case 10: facty_now = 12; break;
	}
	Serial.println(facty_now);
	gifname_offset_ratio();
	break;


    case 'h':
	Serial.print("shift -4 horizontally: ");
	offsetx_now -= 4;
	Serial.println(offsetx_now);
	gifname_offset_ratio();
	break;

    case 'H':
    case 'j':
	Serial.print("shift +4 horizontally: ");
	offsetx_now += 4;
	Serial.println(offsetx_now);
	gifname_offset_ratio();
	break;

    case 'v':
	Serial.print("shift -4 vertically:: ");
	offsety_now -= 4;
	Serial.println(offsety_now);
	gifname_offset_ratio();
	break;

    case 'V':
    case 'b':
	Serial.print("shift +4 vertically:: ");
	offsety_now += 4;
	Serial.println(offsety_now);
	gifname_offset_ratio();
	break;

    case '+': adjust_gamma(+0.2); break;

    case '-': adjust_gamma(-0.2); break;

    // = allows staying on a single picture for up to 1H instead of a few seconds
    case '=':
	longer = longer?0:3600;
	Serial.print("Image display time: "); 
	Serial.println(longer + DISPLAY_TIME_SECONDS); 
	break;

    case '\n':
    case '\r':
	break;

    default:
	// BUG: this does not work for index '0', just type '1', and 'p'
	if (readchar) {
	    while ((readchar >= '0') && (readchar <= '9')) {
		new_file = 10 * new_file + (readchar - '0');
		readchar = 0;
		if (Serial.available()) readchar = Serial.read();
	    }

	    if (new_file) {
		Serial.print("Got new file via serial ");
		Serial.println(new_file);
		index = new_file;
	    } else {
		Serial.print("Got unhandled serial char ");
		Serial.println(readchar);
	    }
	}
    }

    if (debugframe) {
	if (! gotnf) return;
    }

    if (millis() - lastTime > ((DISPLAY_TIME_SECONDS + longer) * 1000)) {
	new_file = 1;
	index++;
    }

    if (new_file) { 
	screenClearCallback();
	offsetx_now = offsetx_default;
	offsety_now = offsety_default;
	factx_now   = factx_default;
	facty_now   = facty_default; 

	frame = 0;
	new_file = 0;
	lastTime = millis();
	if (index >= num_files) index = 0;
	if (index <= -1) index = num_files - 1;
        Serial.print("Fetching file index #");
        Serial.println(index);

        if (openGifFilenameByIndex(GIF_DIRECTORY, index) >= 0) {
            decoder.startDecoding();
        } else {
	    die("FATAL: failed to open file");
	}
    }

    if (clear) screenClearCallback();

    OFFSETX = offsetx_now;
    OFFSETY = offsety_now;
    FACTX   = factx_now;
    FACTY   = facty_now; 

    decoder.decodeFrame();
    frame++;
    if (debugframe) {
	Serial.print("Displayed frame #");
	Serial.print(frame);
	Serial.println(". Press g for next frame");
    }
#if DEBUGLINE
    delay(1000000);
#endif
}

// vim:sts=4:sw=4
