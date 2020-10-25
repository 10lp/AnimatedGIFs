#define NEOPIXEL_MATRIX
#include "neomatrix_config.h"
#include "GifDecoder.h"

// If the matrix is a different size than the GIFs, allow panning through the GIF
// while displaying it, or bouncing it around if it's smaller than the display
int OFFSETX = 0;
int OFFSETY = 0;
int FACTX = 0;
int FACTY = 0;


#if defined(ARDUINOONPC)
    //const char *pathname = FS_PREFIX "/gifs128x192/Aki5PC6_Running.gif";
    const char *pathname = FS_PREFIX "/gifs128x192/abstract_colorful.gif";
#elif defined(ESP8266)
    // 32x32 GIFs on 24x32 display, hence offset of -4
    OFFSETX = -4;
    const char *pathname = "/gifs/concentric_circles.gif";
#else
    const char *pathname = "/gifs64/200_circlesmoke.gif";
#endif

/* template parameters are maxGifWidth, maxGifHeight, lzwMaxBits
 * 
 * The lzwMaxBits value of 12 supports all GIFs, but uses 16kB RAM
 * lzwMaxBits can be set to 10 or 11 for small displays, 12 for large displays
 * All 32x32-pixel GIFs tested work with 11, most work with 10
 */
GifDecoder<kMatrixWidth, kMatrixHeight, 12> decoder;

#ifdef UNIXFS
    // https://www.programiz.com/c-programming/c-file-input-output
    // https://www.gnu.org/software/libc/manual/html_node/I_002fO-on-Streams.html
    #include <stdio.h>
    #include <stdlib.h>
    #include <cerrno>
    FILE *file;
    bool fileSeekCallback(unsigned long position) { return (fseek(file, position, SEEK_SET) != -1); }
    unsigned long filePositionCallback(void) { return ftell(file); }
    int fileReadCallback(void) { return getc(file); }
    int fileReadBlockCallback(void * buffer, int numberOfBytes) { return fread(buffer, 1, numberOfBytes, file); }
#else
    File file;
    bool fileSeekCallback(unsigned long position) { return file.seek(position); }
    unsigned long filePositionCallback(void) { return file.position(); }
    int fileReadCallback(void) { return file.read(); }
    int fileReadBlockCallback(void * buffer, int numberOfBytes) { return file.read((uint8_t*)buffer, numberOfBytes); }
#endif

void screenClearCallback(void) { matrix->clear(); }
void updateScreenCallback(void) { matrix->show(); }

void drawPixelCallback(int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue) {
  CRGB color = CRGB(matrix->gamma[red], matrix->gamma[green], matrix->gamma[blue]);
  // This works but does not handle out of bounds pixels well (it writes to the last pixel)
  // matrixleds[XY(x+OFFSETX,y+OFFSETY)] = color;
  // drawPixel ensures we don't write out of bounds
  matrix->drawPixel(x+OFFSETX, y+OFFSETY, color);
}

// Setup method runs once, when the sketch starts
void setup() {
    // Wait for teensy to be ready
    delay(3000);
    decoder.setScreenClearCallback(screenClearCallback);
    decoder.setUpdateScreenCallback(updateScreenCallback);
    decoder.setDrawPixelCallback(drawPixelCallback);

    decoder.setFileSeekCallback(fileSeekCallback);
    decoder.setFilePositionCallback(filePositionCallback);
    decoder.setFileReadCallback(fileReadCallback);
    decoder.setFileReadBlockCallback(fileReadBlockCallback);

    Serial.begin(115200);
    Serial.println("Starting AnimatedGIFs Sketch");
    matrix_setup();

    Serial.print(pathname);

#ifdef ARDUINOONPC
    if (file) fclose(file);
    file = fopen(pathname, "r");
#else
    if (file) file.close();
    #ifdef FSOSPIFFS
	file = SPIFFS.open(pathname, "r");
    #else
	file = FFat.open(pathname);
    #endif
#endif
    if (!file) {
        Serial.println(": Error opening GIF file");
#ifdef ARDUINOONPC
        Serial.println(strerror(errno));
#endif
	while (1) { delay(1000); }; // while 1 loop only triggers watchdog on ESP chips
    }
    Serial.println(": Opened GIF file, start decoding");
    decoder.startDecoding();
}

void loop() {
    decoder.decodeFrame();
}

// vim:sts=4:sw=4
