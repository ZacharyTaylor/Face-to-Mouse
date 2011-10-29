#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "cv.h"
#include <cmath>

#include <boost/thread.hpp>  
#include <boost/signals2/dummy_mutex.hpp>

#define BASE_SENSITIVITY 30 //pixels moved on camera to pixels moved on screen when adaptive scale = 1
#define MAXIMUM_SCALE_MOVEMENT 50 //largest movement in pixels taken to be a person moving. Anything larger taken to be a glitch and ignored
#define SCALE_FRAMES 8 //number of previous movements to consider when calculating scale
static int cx = GetSystemMetrics(SM_CXSCREEN)/2; //center of screen in x
static int cy = GetSystemMetrics(SM_CYSCREEN)/2; //center of screen in y
#define MOUSE_DETECT 1 //number of pixels mouse must stay within when wanting to click
#define CLICK_DETECT 20 //number of frames mouse must stay still to click
#define EDGE_OFFSET 1 //number of pixels to edge of screen before mouse stops (cannot be 0 without altering edge_scale)
#define EDGE_SCALE 0.5 // changes sensitivy as mouse approaches edge of screen (Very sensitve keep in range 0.3 to 1)
#define BASE_SCALE 2 // changes sensitivy as mouse moves faster (Very sensitve keep in range 1 to 2)
#define RESET_SPEED 100 //sets minimum value for a reset to center

using namespace cv;
using namespace std;

//calculates where mouse will be positioned
void set_position(Point2f *position, Point2f prevPos, float angle[2], float angleAvg[2], float scale);

//clicks the mouse when its been idle
int click_mouse(Point2f position, Point2f prevPos);

//ensures mouse stays on screen
void mouse_bounds(Point2f *position);

//updates storage of movements angles
void update_angle_store(float angle[2], float angleOld[2], float angleStore[SCALE_FRAMES][2]);

//edge scale factor. This slows mouse down as it gets near edge of screen for easy selection of menus scales scale by between 0 and 2
// the scaling factors are alot more arbitrary then they appear
float edge_scale(Point2f prevPos, float scale);

//scales mouse sensitivy based on resent movement. Large movements = increased sensitvity
float main_scale(float angleStore[SCALE_FRAMES][2]);