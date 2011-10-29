#include "stdafx.h"
#include "FaceTracking.h"

// Function to detect and return the coordiantes of the largest face in an image
CvRect* detect_and_draw( IplImage* img, char* cascade_name )
{
    // Create memory for calculations
    static CvMemStorage* storage = 0;

    // Create a new Haar classifier
    static CvHaarClassifierCascade* cascade = 0;

    int scale = 1;

    // Create a new image based on the input image
    IplImage* temp = cvCreateImage( cvSize(img->roi->width/scale,img->roi->height/scale), 8, 3 );
    int i;

    // Load the HaarClassifierCascade
	if( cascade == 0)
	{
		cascade = (CvHaarClassifierCascade*)cvLoad( cascade_name, 0, 0, 0 );
	}
    
    // Allocate the memory storage
    storage = cvCreateMemStorage(0);

    // Clear the memory storage which was used before
    cvClearMemStorage( storage );

	//creates rectangle to return
	CvRect* large = &cvRect(0,0,2,2);
	//return large;

    // Find whether the cascade is loaded, to find the faces. If yes, then:
    if( cascade )
    {

        // There can be more than one face in an image. So create a growable sequence of faces.
        // Detect the objects and store them in the sequence
        CvSeq* faces = cvHaarDetectObjects( img, cascade, storage,
                                            1.1, 2, CV_HAAR_DO_CANNY_PRUNING,
                                            cvSize(40, 40) );

        // Loop the number of faces found.
        for( i = 0; i < (faces ? faces->total : 0); i++ )
        {
           // Create a new rectangle for drawing the face
            CvRect* r = (CvRect*)cvGetSeqElem( faces, i );
			
			if((r->height + r->width) > (large->height + large->width))
			{
				large = r;
			}
        }
    }
	return large;

    // Release the temp image created.
    cvReleaseImage( &temp );
}

void add_dots(IplImage* gray, vector<Point2f>* points, CvRect* face)
{
	Point2f testPoint;
	points->clear();

	//points
	for(int x = face->x; x < (face->x + face->width); x+=10)
	{
		for(int y = (face->y) ; y < (face->y + face->height/50); y+=10)
		{
			testPoint.x = (float)x;
			testPoint.y = (float)y;
			points->push_back(testPoint);
		}
	}

	for(int x = face->x; x < (face->x + face->width); x+=10)
	{
		for(int y = (face->y + face->height/2); y < (face->y + face->height); y+=10)
		{
			testPoint.x = (float)x;
			testPoint.y = (float)y;
			points->push_back(testPoint);
		}
	}
}

vector<Point2f> track_dots(IplImage* gray, IplImage* prevGray, vector<Point2f> points, CvRect* faceBox)
{
	//expands region to search for dots
	faceBox->x -= faceBox->width/2;
	if(faceBox->x < 0)
	{
		faceBox->x = 0;
	}
	faceBox->width *= 2;
	if(faceBox->x + faceBox->width > gray->width)
	{
		faceBox->width = gray->width - faceBox->x;
	}
	faceBox->y -= faceBox->height/2;
	if(faceBox->y < 0)
	{
		faceBox->y = 0;
	}
	faceBox->height *= 2;
	if(faceBox->y + faceBox->height > gray->height)
	{
		faceBox->height = gray->height - faceBox->y;
	}

	cvSetImageROI(gray,*faceBox);
	cvSetImageROI(prevGray,*faceBox);

	vector<Point2f> tempPoints;
	vector<uchar> status;
    vector<float> err;
	TermCriteria termcrit(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,1,0.3);
	Size winSize(20,20);

    calcOpticalFlowPyrLK(prevGray, gray, points, tempPoints, status, err, winSize, 0, termcrit, 0);

	cvResetImageROI(gray);
	cvResetImageROI(prevGray);
	return tempPoints;
}

void draw_dots( IplImage* img, vector<Point2f> points, Scalar colour)
{
	vector<Point2f>::iterator iter;
	for(iter = points.begin(); iter != points.end(); ++iter) 
	{
		cvCircle(img, iter[0], 3, colour, -1, 8);
	}

	cvShowImage("output", img);
}

int calc_angle(vector<Point2f> points, float Avg[2])
{
	Avg[0] = 0;
	Avg[1] = 0;

	int size = points.size();

	if (size == 0)
	{
		return 0;
	}
		
	for(int i = 0; i < size; i++)
	{
		Avg[0] += points.at(i).x;
		Avg[1] += points.at(i).y;
	}

	Avg[0] /= size;
	Avg[1] /= size;

	return 1;
}

void refresh_dots(IplImage* img, IplImage* gray, vector<Point2f> points[], boost::mutex* faceMutex, CvRect* faceBox)
{
	faceMutex->lock();
	// Call the function to detect and draw the face positions
	cvSetImageROI(img,cvRect(0,0, img->width, img ->height));
	faceBox = detect_and_draw(img, "C:/University/OpenCV2.2/data/haarcascades/haarcascade_frontalface_alt.xml");
	//shrinks facebox
	faceBox->width /= 2;
	faceBox->x += faceBox->width/2;
	faceBox->height /= 2;
	faceBox->y += faceBox->height/2;
	
	//add dots to image
	add_dots(gray, &points[FACE], faceBox);
	cvSetImageROI(img,cvRect(0,0, img->width, img ->height));
	faceMutex->unlock();
}