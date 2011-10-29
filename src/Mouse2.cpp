// Mouse.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Mouse2.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int _tmain( int argc, char** argv )
{
	CvCapture* cap = cvCreateCameraCapture(0); // connect to a camera
	IplImage *img = cvQueryFrame(cap); // Create a sample image
	IplImage *threadImg = cvCloneImage(img); //thread image

	IplImage *gray = cvCreateImage( cvSize( img->width, img->height ), IPL_DEPTH_8U, 1 ); //gray image
	IplImage *threadGray = cvCreateImage( cvSize( img->width, img->height ), IPL_DEPTH_8U, 1 );// gray thread image

	IplImage *prevGray = cvCloneImage(gray);// previous gray image needed for lk
	
	cvCvtColor( img, gray, CV_RGB2GRAY ); //fills in gray image
	cvCvtColor( img, threadGray, CV_RGB2GRAY ); //fills in gray image used by thread

	Point2f position, prevPos; //mouse positions

	boost::mutex faceMutex; //lock for thread interfacing

	vector<Point2f> points[1], threadPoints[1]; //points vector can be increased in size for multiple faces or parts of faces

	CvRect* faceBox = &cvRect(0,0,img->width,img->height);  //box around face
	CvRect* threadFaceBox = &cvRect(0,0,img->width,img->height); // copy for thread

	float angleAvg[2]; angleAvg[0] = 0; angleAvg[1] = 0; //average angle
	float angleOld[2]; angleOld[0] = 0; angleOld[1] = 0; //previous angle
	float angle[2]; angle[0] = 0; angle[1] = 0; // angle
	float angleStore[SCALE_FRAMES+1][2]; //stores a measuse of resent movement
	int signal = 0; //disables quick reset during clicking

	//initilizes angle stroage to 0s
	for(int n = 0; n < SCALE_FRAMES; n++)
	{
		angleStore[n][0] = 0;
		angleStore[n][1] = 0;
	}

	int newFace = 0; //if new face input
	float scale = 1; //scales mouse sensitivity

	//initilizes potision and previous position
	position.x = 0; position.y = 0;
	prevPos.x = 0; prevPos.y = 0;

	//Main loop q to exit
	while(cvWaitKey(10) != 'q')
	{

		//if face detection thread has finished
		if(faceMutex.try_lock())
		{
			newFace = 1;
			//swap out old info with thread info
			swap(points, threadPoints);
			swap(faceBox, threadFaceBox);
			//copy in new images for thread to process
			cvCopy(img, threadImg);
			cvCopy(gray, threadGray);
			faceMutex.unlock();
			//set a new face detect thread going
			boost::thread workerThread1( refresh_dots, threadImg, threadGray, threadPoints, &faceMutex, threadFaceBox);
			//refresh_dots( threadImg, threadGray, threadPoints, &faceMutex, threadFaceBox);
		}

		//get prev gray image
		IplImage *prevGray = cvCloneImage(gray);

		//get new image
		img = cvQueryFrame(cap);

		//convert to grey
		cvCvtColor( img, gray, CV_BGR2GRAY );

		//normalise image
		cvEqualizeHist(gray, gray);

		//tracks dots on face
		points[FACE] = track_dots(gray, prevGray, points[FACE],faceBox);

		//draw the dots
		draw_dots(img,points[FACE], Scalar(0,255,0));

		//move last times angle and positon to arrays holding previous info
		swap(angle, angleOld);
		swap(prevPos, position);

		//if face exists calculates angle
		if(calc_angle(points[FACE], angle))
		{
			//increase sensitvity by multiplying by arbitrary number
			angle[0] = BASE_SENSITIVITY*angle[0];
			angle[1] = BASE_SENSITIVITY*angle[1];

			//updates storage array holding previous movements
			update_angle_store(angle, angleOld, angleStore);

			//calculate scale based on resent movement size (lots of large movements = high scale)
			scale = main_scale(angleStore);

			//if using new face data compensates for new position of face
			if(newFace == 1)
			{
				angleAvg[0] = angle[0] - (angleOld[0] - angleAvg[0]);
				angleAvg[1] = angle[1] - (angleOld[1] - angleAvg[1]);
				newFace = 0;
			}
			else if(((abs(angle[0] - angleOld[0])> RESET_SPEED) || (abs(angle[1] - angleOld[1])> RESET_SPEED)) && (signal != 1))
			{
				angleOld[0] = angle[0];
				angleOld[1] = angle[1];

				//angle[0] = angleAvg[0];
				//angle[1] = angleAvg[1];
			}
		}
		//else no face so center mouse
		else
		{
			angleOld[0] = angle[0];
			angleOld[1] = angle[1];

			angle[0] = angleAvg[0];
			angle[1] = angleAvg[1];
		}

		//edge scale factor. This slows mouse down as it gets near edge of screen for easy selection of menus scales scale by between 0 and 2
		scale = edge_scale(prevPos, scale);
			
		//sets position and scales it
		set_position(&position, prevPos, angle, angleAvg, scale);

		//ensures mouse stays on screen
		mouse_bounds(&position);

		//sets the cursor position
		SetCursorPos((int)position.x + cx, (int)position.y + cy);

		//click the mouse if hovering over something
		signal = click_mouse(position, prevPos);
	}

	// Release the images
	cvReleaseImage(&img);
	cvReleaseImage(&threadImg);
	cvReleaseImage(&gray);
	cvReleaseImage(&prevGray);
	cvReleaseImage(&threadGray);

	return 0;
}

