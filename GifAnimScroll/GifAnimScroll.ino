#define BASICARDUINOFS
// Use NeoMatrix API, even if it may use the SmartMatrix backend depending on the CPU
#define NEOMATRIX
#include "GifAnim_Impl.h"


// If the matrix is a different size than the GIFs, allow panning through the GIF
// while displaying it, or bouncing it around if it's smaller than the display
int OFFSETX = 0;
int OFFSETY = 0;
int FACTX = 0;
int FACTY = 0;

uint16_t x, y;

const char *pathname = "/gifs96/trancefamilysf.gif";
//const char *pathname = "/gifs96/trancefamilysf64.gif";
//const char *pathname = "/gifs96/dancer_lady.gif";

// Scroll within big bitmap so that all if it becomes visible or bounce a small one.
// If the bitmap is bigger in one dimension and smaller in the other one, it will
// be both panned and bounced in the appropriate dimensions.
// speed can be 1, 2, or 4
void panOrBounce (uint16_t *x, uint16_t *y, uint16_t sizeX, uint16_t sizeY, uint8_t speed = 1, bool reset = false ) {
    // keep integer math, deal with values 16 times too big
    // start by showing upper left of big bitmap or centering if the display is big
    static int16_t xf;
    static int16_t yf;
    // scroll speed in 1/64th
    static int16_t xfc;
    static int16_t yfc;
    // scroll down and right by moving upper left corner off screen
    // more up and left (which means negative numbers)
    static int16_t xfdir;
    static int16_t yfdir;

    if (reset) {
	xf = max(0, (mw-sizeX)/2) << 6;
	yf = max(0, (mh-sizeY)/2) << 6;
	xfc = 16*speed;
	yfc = 16*speed;
	xfdir = -1;
	yfdir = -1;
    }

    bool changeDir = false;

    // Get actual x/y by dividing by 16.
    *x = xf >> 6;
    *y = yf >> 6;

    // Only pan if the display size is smaller than the pixmap
    // but not if the difference is too small or it'll look bad.
    if (sizeX-mw>2) {
	xf += xfc*xfdir;
	if (xf >= 0)                 { xfdir = -1; changeDir = true ; };
	// we don't go negative past right corner, go back positive
	if (xf <= ((mw-sizeX) << 6)) { xfdir = 1;  changeDir = true ; };
    }
    if (sizeY-mh>2) {
	yf += yfc*yfdir;
	// we shouldn't display past left corner, reverse direction.
	if (yf >= 0)                 { yfdir = -1; changeDir = true ; };
	if (yf <= ((mh-sizeY) << 6)) { yfdir = 1;  changeDir = true ; };
    }
    // only bounce a pixmap if it's smaller than the display size
    if (mw>sizeX) {
	xf += xfc*xfdir;
	// Deal with bouncing off the 'walls'
	if (xf >= (mw-sizeX) << 6) { xfdir = -1; changeDir = true ; };
	if (xf <= 0)           { xfdir =  1; changeDir = true ; };
    }
    if (mh>sizeY) {
	yf += yfc*yfdir;
	if (yf >= (mh-sizeY) << 6) { yfdir = -1; changeDir = true ; };
	if (yf <= 0)           { yfdir =  1; changeDir = true ; };
    }

    if (changeDir) {
	// Add -1, 0 or 1 but bind result to 1 to 1.
	// Let's take 3 is a minimum speed, otherwise it's too slow.
	xfc = constrain(xfc + random(-1*speed, 2*speed), 3*speed, 16*speed);
	yfc = constrain(yfc + random(-1*speed, 2*speed), 3*speed, 16*speed);
    }
}

// Setup method runs once, when the sketch starts
void setup() {
    sav_setup();
    if (sav_newgif(pathname)) delay(100000); // while 1 loop only triggers watchdog on ESP chips
    panOrBounce(&x, &y, 320, 96, 8, true);
}

void loop() {

    panOrBounce(&x, &y, 320, 96, 8);
    OFFSETX = x;
    OFFSETY = y;
    //matrix->clear();
    sav_loop();
    matrix->show();
}

// vim:sts=4:sw=4
