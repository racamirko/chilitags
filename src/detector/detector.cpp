/*******************************************************************************
*   Copyright 2013 EPFL                                                        *
*                                                                              *
*   This file is part of chilitags.                                            *
*                                                                              *
*   Chilitags is free software: you can redistribute it and/or modify          *
*   it under the terms of the Lesser GNU General Public License as             *
*   published by the Free Software Foundation, either version 3 of the         *
*   License, or (at your option) any later version.                            *
*                                                                              *
*   Chilitags is distributed in the hope that it will be useful,               *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of             *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
*   GNU Lesser General Public License for more details.                        *
*                                                                              *
*   You should have received a copy of the GNU Lesser General Public License   *
*   along with Chilitags.  If not, see <http://www.gnu.org/licenses/>.         *
*                                                                              *
*                                                                              *
*                                                                              *
*   13/08/02 - racamirko                                                       *
*       Added video-file support and usage screen.                             *
*                                                                              *
*******************************************************************************/

// This file serves as an illustration of how to use Chilitags


// This header contains the detection part
#include <DetectChilitags.hpp>
// This header provides an easy way to use the tag detection information
#include <Chilitag.hpp>

// CvConvenience provides operators to add and multiply CvPoint2D32f
// and compute distances between them
using namespace CvConvenience; 

// OpenCV goodness for I/O
#include <opencv2/highgui/highgui.hpp>

// C-Style string to int conversion and string formatting.
#include <cstdio>
#include <iostream> // for consol output

void displayUsage();

// Some basic, portable timing
#ifdef WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif
namespace {
	static long GetTime() {
	#ifdef WIN32
		return timeGetTime();
	#else
		struct timeval tTimeval;
		gettimeofday(&tTimeval, 0);
		return tTimeval.tv_sec*1000l + (long) (tTimeval.tv_usec/1000.0f + 0.5f);
	#endif
	}
}

int main(int argc, char* argv[])
{
	// Simple parsing of the parameters related to the image acquisition
	int tXRes = 640;
	int tYRes = 480;
	int tCameraIndex = 0;
    bool inputFromFile = false;
	if (argc > 2) {
		tXRes = std::atoi(argv[1]);
		tYRes = std::atoi(argv[2]);
	}
	if (argc > 3) {
		tCameraIndex = atoi(argv[3]);
	}

    if( tXRes == 0 || argc == 2 ){ // errors, or only one input == filename of video
        inputFromFile = true;
    } else {
        displayUsage();
        return 1;
    }

	// The source of input images
    CvCapture *tCapture = NULL;
    if(inputFromFile){
        tCapture = cvCaptureFromAVI(argv[1]);
    }else{
        tCapture = cvCaptureFromCAM(tCameraIndex);
    }

	if (!tCapture)
	{
		std::cerr << "unable to initialise CVCapture" << std::endl;
		return 1;
	}
    if( inputFromFile ){
        tXRes = (int) cvGetCaptureProperty(tCapture, CV_CAP_PROP_FRAME_WIDTH);
        tYRes = (int) cvGetCaptureProperty(tCapture, CV_CAP_PROP_FRAME_HEIGHT);
        tXRes = (int) cvGetCaptureProperty(tCapture, CV_CAP_PROP_FRAME_WIDTH);
    } else {
        cvSetCaptureProperty(tCapture, CV_CAP_PROP_FRAME_WIDTH, tXRes);
        cvSetCaptureProperty(tCapture, CV_CAP_PROP_FRAME_HEIGHT, tYRes);
        cvSetCaptureProperty(tCapture, CV_CAP_PROP_FPS, tCameraIndex); // a bit unclear
        cvSetCaptureProperty(tCapture, CV_CAP_PROP_MODE, 1);
    }

	// A data structure needed for OpenCv to draw text.
	CvFont mFont;
	cvInitFont(&mFont,CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5);
	cvNamedWindow("DisplayChilitags");

#ifdef WIN32
	timeBeginPeriod(1);
#endif
	// The time at which run was last called, to compute the fps.
	long mPreviousMillisTimestamp(GetTime());

	// The tag detection happens in the DetectChilitags class.
	// All it needs is a pointer to a OpenCv Image, i.e. a IplImage **
	// and a call to its update() method every time the image is updated.
	IplImage *tInputImage;
	chilitags::DetectChilitags tDetectChilitags(&tInputImage);

	// Main loop, exiting when 'q is pressed'
	for (tInputImage = cvQueryFrame(tCapture);
		'q' != cvWaitKey(1);
		tInputImage = cvQueryFrame(tCapture)) {

		// Detect tags on the current image.
		tDetectChilitags.update();

		// The color (magenta) that will be used for all information
		// overlaid on the captured image
		const static CvScalar sColor= CV_RGB(255, 0, 255);

		// These constants will be used be given to OpenCv for drawing with
		// sub-pixel accuracy with fixed point precision coordinates
		static const int scShift = 16;
		static const float scPrecision = 1<<scShift;

		// A buffer for the various string formatting
		char tTextBuffer[256];
		
		// We dont want to draw directly on the input image, so we clone it
		IplImage *tOutputImage = cvCloneImage(tInputImage);

		// We iterate over the 1024 possible tags (from #0 to #1023)
		for (int tTagId = 0; tTagId < 1024; ++tTagId) {

			// The Chilitag class is a convenience handle to acces information
			// related to a given tag.
			// The object itself is lightweight, so we can create and delete it
			// frequently (we don't need to store it as member for example)
			chilitags::Chilitag tTag(tTagId);

			// Chilitag allows us to easily access the two main pieces of data
			// First, the isPresent() method tells us whether the related tag
			// has been detected in the last frame.
			// This is a first and necessary step to access further information
			// about the tag, as a "absent" tag will have obsolete information.
			if (tTag.isPresent()) {
				
				// Second, now that we know that the tag has been updated, the
				// getCorners() method returns the coordinates of the
				// quadrilateral containing the tag on the input picture.
				chilitags::Quad tCorners = tTag.getCorners();

				// We start by drawing this quadrilateral
				for (size_t i = 0; i < chilitags::Quad::scNPoints; ++i) {
					cvLine(
						tOutputImage,
						cvPointFrom32f(scPrecision*tCorners[i]),
						cvPointFrom32f(scPrecision*tCorners[(i+1)%4]),
						sColor, 1, CV_AA, scShift);
				}

				// The quadrilateral is given under the form of a Quad class,
				// which provide a minimal set of geometrical functionalities,
				// such as getCenter()
				CvPoint2D32f tCenter = tCorners.getCenter();

				// We will print the identifier of the tag at its center
				std::sprintf(tTextBuffer, "%d", tTagId);
				cvPutText(tOutputImage, tTextBuffer, cvPoint(tCenter.x, tCenter.y),
					&mFont, sColor);

				// Other points an be computed from the four corners of the Quad.
				// Chilitags are oriented. It means that the points 0,1,2,3 of
				// the Quad coordinates are consistently the top-left, top-right,
				// bottom-right and bottom-left
				// (i.e. clockwise, starting from top-left)
				// Using this, we can compute (an approximation of) the middle of
				// the top side of the tag...
				CvPoint2D32f tTop = 0.5f*(tCorners[0]+tCorners[1]);
				// and of its right side
				CvPoint2D32f tRight = 0.5f*(tCorners[1]+tCorners[2]);

				// We display the length in pixel of these sides
				std::sprintf(tTextBuffer, "The top border is %.1fpx long.",
					dist(tCorners[0], tCorners[1]));
				cvPutText(tOutputImage, tTextBuffer, cvPoint(tTop.x, tTop.y),
					&mFont, sColor);
				std::sprintf(tTextBuffer, "The right border is %.1fpx long.",
					dist(tCorners[1], tCorners[2]));
				cvPutText(tOutputImage, tTextBuffer, cvPoint(tRight.x, tRight.y),
					&mFont, sColor);

				// And we draw a line from the center to the midlle of these sides,
				// to show the orientation of the tag.
				cvLine(tOutputImage,
					cvPointFrom32f(scPrecision*tCenter),
					cvPointFrom32f(scPrecision*tTop),
					sColor, 1, CV_AA, scShift);
				cvLine(tOutputImage,
					cvPointFrom32f(scPrecision*tCenter),
					cvPointFrom32f(scPrecision*tRight),
					sColor, 1, CV_AA, scShift);
			}
		}
		
		// Some stats on the current frame (resolution and framerate)
		long tNewTimeStamp = GetTime();
		std::sprintf(tTextBuffer, "%dx%d@%.1f fps (press q to quit)",
			tOutputImage->width, tOutputImage->height,
			1000.f/(float) (tNewTimeStamp-mPreviousMillisTimestamp));
		cvPutText(tOutputImage, tTextBuffer, cvPoint(32,32), &mFont, sColor);
		mPreviousMillisTimestamp = tNewTimeStamp;

		// Finally...
		cvShowImage("DisplayChilitags", tOutputImage);
	}

#ifdef WIN32
	timeEndPeriod(1);
#endif
	cvDestroyWindow("DisplayChilitags");

	return 0;
}


void displayUsage(){
    std::cout << "Detector v1.0" << std::endl;
    std::cout << "Usage with camera:" << std::endl;
    std::cout << "\t\t./detector [x-res y-res [camera-index]]" << std::endl;
    std::cout << "or with a video file (supported by OpenCV)" << std::endl;
    std::cout << "\t\t./detector <filename>" << std::endl;
}
