#ifndef animatedgif_config
#define animatedgif_config

/* GifDecoder needs lzwMaxBits
 * The lzwMaxBits value of 12 supports all GIFs, but uses 16kB RAM
 * lzwMaxBits can be set to 10 or 11 for small displays, 12 for large displays
 * All 32x32-pixel GIFs tested work with 11, most work with 10
 */
const int lzwMaxBits = 12;

//#define NEOMATRIX // Switch to NEOMATRIX backend from native SMARTMATRIX backend
//#define NEOPIXEL_MATRIX  // If NEOMATRIX, use FastLED::NeoMatrix, not SmartMatrix_GFX

// if you want to display a file and display that one first
#define FIRSTINDEX 0
//#define DEBUGLINE 16

// Use Neomatrix API (which in turn could be using SmartMatrix driver)?
// This is defined in main ino that calls sav_loop
#if defined(NEOMATRIX) ||  defined(ARDUINOONPC)
    // This doesn't work due to variables being redefined. Sigh...
    // instead it's included once from AnimatedGIFs.ino
    //#include "neomatrix_config.h"
#else // NEOMATRIX
    #pragma message "Compiling for Native SmartMatrix"
    // CHANGEME, see MatrixHardware_ESP32_V0.h in SmartMatrix/src
    #define GPIOPINOUT 3
    #define ENABLE_SCROLLING  1
    #if defined (ARDUINO)
        //#include <SmartLEDShieldV4.h>  // uncomment this line for SmartLED Shield V4 (needs to be before #include <SmartMatrix3.h>)
        #include <SmartMatrix3.h>
    #elif defined (SPARK)
        #include "application.h"
        #include "SmartMatrix3_Photon_Apa102/SmartMatrix3_Photon_Apa102.h"
    #endif

    // Neomatrix brightness is different (full brightness can use up 40A) and defined
    // to a different value in neomatrix_config.h
    // range 0-255
    const int defaultBrightness = 255;
    const uint8_t kMatrixWidth = gif_size;        // known working: 32, 64, 96, 128
    const uint8_t kMatrixHeight = gif_size;       // known working: 16, 32, 48, 64
    /* SmartMatrix configuration and memory allocation */
    #define COLOR_DEPTH 24                  // known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24
    const uint8_t kRefreshDepth = 24;       // known working: 24, 36, 48
    const uint8_t kDmaBufferRows = 2;       // known working: 2-4
    const uint8_t kPanelType = SMARTMATRIX_HUB75_32ROW_MOD16SCAN; // use SMARTMATRIX_HUB75_16ROW_MOD8SCAN for common 16x32 panels
    //const uint8_t kPanelType = SMARTMATRIX_HUB75_64ROW_MOD32SCAN;
    const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);    // see http://docs.pixelmatix.com/SmartMatrix for options
    const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
    const uint8_t kScrollingLayerOptions = (SM_SCROLLING_OPTIONS_NONE);
#endif

// If the matrix is a different size than the GIFs, set the offset for the upper left corner
// (negative or positive is ok).
extern int OFFSETX;
extern int OFFSETY;
extern int FACTX;
extern int FACTY;

#define DISPLAY_TIME_SECONDS 10
#endif

