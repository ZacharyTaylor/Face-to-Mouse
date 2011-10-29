#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "cv.h"

#include <boost/thread.hpp>  
#include <boost/signals2/dummy_mutex.hpp>

using namespace cv;
using namespace std;

enum {FACE};
enum {TOPLEFT, TOPRIGHT, BOTTOMLEFT, BOTTOMRIGHT};

CvRect* detect_and_draw( IplImage* img, char* cascade_name );

void add_dots(IplImage* gray, vector<Point2f>* points, CvRect* face);

vector<Point2f> track_dots(IplImage* gray, IplImage* prevGray, vector<Point2f> points, CvRect* faceBox);

void draw_dots( IplImage* img, vector<Point2f> points, Scalar colour);

int calc_angle(vector<Point2f> points, float Avg[2]);

void refresh_dots(IplImage* img, IplImage* gray, vector<Point2f> points[], boost::mutex* faceMutex, CvRect* faceBox);

