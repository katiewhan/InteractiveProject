/* Program by Katie Han */

/* Window changes color from white to black according to live movement */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

int main(int argc, char *argv[]) {
    // open camera
    CvCapture* video = cvCaptureFromCAM(0);
    
    if(!video) {
        fprintf(stderr, "Cannot open camera.");
        return 1;
    }
    
    // get first two frames from camera video
    IplImage* curr = cvQueryFrame(video);
    IplImage* prev = cvCloneImage(curr);
    curr = cvQueryFrame(video);
    IplImage* next = cvCloneImage(curr);
    float premov = 0; // represents previous movement value
    
    // output window
    cvNamedWindow("Image", CV_GUI_NORMAL | CV_WINDOW_NORMAL);
    //Uncomment this to make window fullscreen
    //cvSetWindowProperty("Image", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
    
    // set up white output image
    int height = 1000;
    int width = 1400;
    CvSize size = cvSize(width, height);
    IplImage* img = cvCreateImage(size, IPL_DEPTH_8U, 1);
    int step = img->widthStep;

    for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
            img->imageData[i*step+j] = 127;
        }
    }
    
    // start infinite loop
    for(;;) {
        // get next frame
        curr = cvQueryFrame(video);

        // break if next frame is null
        if(curr == NULL) {
            break;
        } else {
            // create empty images needed for comparison
            IplImage* dest1 = cvCreateImage(cvSize(prev->width, prev->height), prev->depth, prev->nChannels);
            IplImage* dest2 = cvCreateImage(cvSize(prev->width, prev->height), prev->depth, prev->nChannels);
            IplImage* result = cvCreateImage(cvSize(prev->width, prev->height), prev->depth, prev->nChannels);
            IplImage* gray = cvCreateImage(cvSize(prev->width, prev->height), prev->depth, 1);
            
            // compare three consecutive frames and save difference to dest
            cvAbsDiff(prev, next, dest1);
            cvAbsDiff(next, curr, dest2);
            // get overlapping difference and save to result
            cvAnd(dest1, dest2, result, NULL);
            cvThreshold(result, result, 40, 255, CV_THRESH_BINARY);
            
            // convert image to grayscale and count non zero pixels
            cvConvertImage(result, gray, CV_RGB2GRAY);
            float move = cvCountNonZero(gray);
            float total = result->imageSize;

            float now = move/total; // percentage of movement in frame
            if (now > 0.0001) {
                if (now >= premov) {
                    // if video is moving more than it was previously, make image darker
                    for(int i = 0; i < height; i++) {
                        for(int j = 0; j < width; j++) {
                            // pixel value ranges from -127 to +127
                            if (img->imageData[i*step+j] < (float)0 || img->imageData[i*step+j] > (float)20) {
                                img->imageData[i*step+j] = (img->imageData[i*step+j])-20;
                            }
                        }
                    }
                } else {
                    // if video is moving but less than it was previously, make image lighter
                    for(int i = 0; i < height; i++) {
                        for(int j = 0; j < width; j++) {
                            if (img->imageData[i*step+j] > (float)0 || img->imageData[i*step+j] < (float)-5) {
                                img->imageData[i*step+j] = img->imageData[i*step+j]+5;
                            }
                        }
                    }
                }
                
            } else {
                // if video does not have substantial movement, make image darker
                for(int i = 0; i < height; i++) {
                    for(int j = 0; j < width; j++) {
                        if (img->imageData[i*step+j] > (float)0 || img->imageData[i*step+j] < (float)-5) {
                            img->imageData[i*step+j] = img->imageData[i*step+j]+5;
                        }
                    }
                }
            }
            
            // render output image
            cvShowImage("Image", img);
            
            // set current images and movement value to previous
            prev = cvCloneImage(next);
            next = cvCloneImage(curr);
            premov = now;

            // if any key is pressed, destroy window and quit
            if(cvWaitKey(1) >= 0) {
                cvDestroyAllWindows();
                cvReleaseCapture(&video);
                break;
            }
        }
    }
    return 0;
}