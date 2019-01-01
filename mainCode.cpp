// main.cpp

#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

#include<iostream>
#include<conio.h>           
#include "BlobOfCar.h"

// Scaler values of required colors
const cv::Scalar SCALAR_BLACK = cv::Scalar(0.0, 0.0, 0.0);
const cv::Scalar SCALAR_WHITE = cv::Scalar(255.0, 255.0, 255.0);
const cv::Scalar SCALAR_YELLOW = cv::Scalar(0.0, 255.0, 255.0);
const cv::Scalar SCALAR_GREEN = cv::Scalar(0.0, 200.0, 0.0);
const cv::Scalar SCALAR_RED = cv::Scalar(0.0, 0.0, 255.0);

// Lists the prototypes of the functions being used  
void matchCurrentFrameCarBlobsToExistingCarBlobs(std::vector<BlobOfCar> &existingCarBlobs, std::vector<BlobOfCar> &currentFrameCarBlobs);
void addBlobToExistingCarBlobs(BlobOfCar &currentFrameBlob, std::vector<BlobOfCar> &existingCarBlobs, int &intIndex);
void addNewBlob(BlobOfCar &currentFrameBlob, std::vector<BlobOfCar> &existingCarBlobs);
double distanceBetweenPoints(cv::Point point1, cv::Point point2);
void drawAndShowContours(cv::Size imageSize, std::vector<std::vector<cv::Point> > contours, std::string strImageName);
void drawAndShowContours(cv::Size imageSize, std::vector<BlobOfCar> CarBlobs, std::string strImageName);
bool checkIfCarBlobsCrossedTheLine(std::vector<BlobOfCar> &CarBlobs, int &intHorizontalLinePosition, int &carCount);
void drawBlobInfoOnImage(std::vector<BlobOfCar> &CarBlobs, cv::Mat &imgFrame2Copy);
void drawCarCountOnImage(int &carCount, cv::Mat &imgFrame2Copy);


int main(void) {

    cv::VideoCapture videoObject;
    
    //The program will use two consecutive frames of images in the video that are compared with
    // to detect movement of cars to create workable blobs
    cv::Mat imageFrame1;
    cv::Mat imageFrame2;

    std::vector<BlobOfCar> CarBlobs;

    // The introduced line on the video file acts as a reference line that counts cars when crossed
    cv::Point crossingLine[2];
    int carCount = 0;
    videoObject.open("CarsDriving.mp4");

    // Error message pops up if unable to open video file. The program then exits straight away
    if (!videoObject.isOpened()) {                                                 
        std::cout << "ERROR: Cannot read video file !" << std::endl << std::endl;      
        _getch();                  
        return(0);                                                              
    }

    // This ensures that the video file has more than 1 frame (Ensures that the file is in fact a video, not a picture)
    if (videoObject.get(CV_CAP_PROP_FRAME_COUNT) < 2) {
        std::cout << "ERROR: Video file must have at least two frames !";
        _getch();                   
        return(0);
    }

    videoObject.read(imageFrame1);
    videoObject.read(imageFrame2);
    int intHorizontalLinePosition = (int)std::round((double)imageFrame1.rows * 0.35);
    crossingLine[0].x = 0;
    crossingLine[0].y = intHorizontalLinePosition;
    crossingLine[1].x = imageFrame1.cols - 1;
    crossingLine[1].y = intHorizontalLinePosition;
    char chCheckForEscKey = 0;
    bool blnFirstFrame = true;
    int frameCount = 2;

    // loop that compares two consecutive frames of images. loop stops if video stops or escape key is pressed
    while (videoObject.isOpened() && chCheckForEscKey != 27) 
    {

        std::vector<BlobOfCar> currentFrameCarBlobs;
        cv::Mat imgFrame1Copy = imageFrame1.clone();
        cv::Mat imgFrame2Copy = imageFrame2.clone();
        cv::Mat imgDifference;
        cv::Mat imgThresh;

        // converts both frames of images to greyscale images
        cv::cvtColor(imgFrame1Copy, imgFrame1Copy, CV_BGR2GRAY);
        cv::cvtColor(imgFrame2Copy, imgFrame2Copy, CV_BGR2GRAY);

        // the images are blurred to reduce noice in the images
        cv::GaussianBlur(imgFrame1Copy, imgFrame1Copy, cv::Size(5, 5), 0);
        cv::GaussianBlur(imgFrame2Copy, imgFrame2Copy, cv::Size(5, 5), 0);

        // difference captures the movement of the car from one position to another helping to identify a blob
        cv::absdiff(imgFrame1Copy, imgFrame2Copy, imgDifference);
        //removes areas of the image with low intensity
        cv::threshold(imgDifference, imgThresh, 30, 255.0, CV_THRESH_BINARY);
        // shows a window of the threshold image
        cv::imshow("imgThresh", imgThresh);
        // creating possible structuring elements for morphological operations
        cv::Mat structuringElement3x3 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
        cv::Mat structuringElement5x5 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
        cv::Mat structuringElement7x7 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 7));
        cv::Mat structuringElement15x15 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(15, 15));

        // every detected blob is dilated to fill intrusions and eroded to remove extrusions
        for (unsigned int i = 0; i < 2; i++) {
            cv::dilate(imgThresh, imgThresh, structuringElement5x5);
            cv::dilate(imgThresh, imgThresh, structuringElement5x5);
            cv::erode(imgThresh, imgThresh, structuringElement5x5);
        }

        
        cv::Mat imgThreshCopy = imgThresh.clone();
        std::vector<std::vector<cv::Point> > contours;
        cv::findContours(imgThreshCopy, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        // shows a window of the contours of the image
        drawAndShowContours(imgThresh.size(), contours, "imgContours");
        std::vector<std::vector<cv::Point> > convexHulls(contours.size());

        // finds the convex hulls (simple polygon that describes the shape of blobs) using the contour points
        for (unsigned int i = 0; i < contours.size(); i++) {
            // built-in function that finds convex hulls (simple most polygon) from the given vector of contours
             cv::convexHull(contours[i], convexHulls[i]);
        }

        // shows a window of the convex hulls of the image
        drawAndShowContours(imgThresh.size(), convexHulls, "imgConvexHulls");

        for (auto &convexHull : convexHulls) 
        {
            BlobOfCar possibleBlob(convexHull);
            
            //checks several phyiscal conditions whether a blob (convex hull) can be regarded as a car
            if (possibleBlob.currentBoundingRectangle.area() > 400 &&
                possibleBlob.CurrentRectangleRatio > 0.2 &&
                possibleBlob.CurrentRectangleRatio < 4.0 &&
                possibleBlob.currentBoundingRectangle.width > 30 &&
                possibleBlob.currentBoundingRectangle.height > 30 &&
                possibleBlob.CurrentDiagonalSize > 60.0 &&
                (cv::contourArea(possibleBlob.currentContour) / (double)possibleBlob.currentBoundingRectangle.area()) > 0.50) {
                currentFrameCarBlobs.push_back(possibleBlob);
            }
        }
        // shows the possible blobs of cars
        drawAndShowContours(imgThresh.size(), currentFrameCarBlobs, "imgCurrentFrameCarBlobs");

        if (blnFirstFrame == true) {
            for (auto &currentFrameBlob : currentFrameCarBlobs) {
                CarBlobs.push_back(currentFrameBlob);
            }
        } else {
            //compares and checks whether the old car blobs exist in the new image frame or new car blobs have emerged
            matchCurrentFrameCarBlobsToExistingCarBlobs(CarBlobs, currentFrameCarBlobs);
        }

        drawAndShowContours(imgThresh.size(), CarBlobs, "imgCarBlobs");
        imgFrame2Copy = imageFrame2.clone();    
        //draws the car number and the reference line on the image      
        drawBlobInfoOnImage(CarBlobs, imgFrame2Copy);
        bool blnAtLeastOneBlobCrossedTheLine = checkIfCarBlobsCrossedTheLine(CarBlobs, intHorizontalLinePosition, carCount);

        if (blnAtLeastOneBlobCrossedTheLine == true) 
            cv::line(imgFrame2Copy, crossingLine[0], crossingLine[1], SCALAR_GREEN, 2);
        else 
            cv::line(imgFrame2Copy, crossingLine[0], crossingLine[1], SCALAR_RED, 2);
        // shows the number of cars that have passed on the image
        drawCarCountOnImage(carCount, imgFrame2Copy);
        cv::imshow("imgFrame2Copy", imgFrame2Copy);


        currentFrameCarBlobs.clear();
        //obtains the image for the next iternation
        imageFrame1 = imageFrame2.clone();           
        
        // reads the next image from the from the video file.
        if ((videoObject.get(CV_CAP_PROP_POS_FRAMES) + 1) < videoObject.get(CV_CAP_PROP_FRAME_COUNT)) {
            videoObject.read(imageFrame2);
        } else {
            std::cout << "end of video\n";
            break;
        }

        blnFirstFrame = false;
        frameCount++;
        chCheckForEscKey = cv::waitKey(1);
    }

    // waits for user to press any button to exit if escape button has not been pressed
    if (chCheckForEscKey != 27) {               
        cv::waitKey(0);                         
    }
   

    return(0);
}

// The following function compares the blobs in the current frames to the existing blobs and determines if current frame
// blobs correspond to existing blobs, or new blobs appear. The function also eliminates existing blobs if a new comparable blob doesn't
// appear in the next several image frames
void matchCurrentFrameCarBlobsToExistingCarBlobs(std::vector<BlobOfCar> &existingCarBlobs, std::vector<BlobOfCar> &currentFrameCarBlobs) 
{
    // obtains the predicted next position of the exisiting set of car blobs
    for (auto &existingBlob : existingCarBlobs) {
        existingBlob.MatchFoundOrNewBlobOfCar = false;
        existingBlob.predictNextPosition();
    }

    // goes through all current car blobs to check if new blob position lies within the specified distance from the predicted position
    for (auto &currentFrameBlob : currentFrameCarBlobs) {
        int intIndexOfLeastDistance = 0;
        double dblLeastDistance = 100000.0;

        for (unsigned int i = 0; i < existingCarBlobs.size(); i++) {
            if (existingCarBlobs[i].TrackingStatus == true) {
                double dblDistance = distanceBetweenPoints(currentFrameBlob.centerPoints.back(), existingCarBlobs[i].NextAnticipatedPosition);
                if (dblDistance < dblLeastDistance) {
                    dblLeastDistance = dblDistance;
                    intIndexOfLeastDistance = i;
                }
            }
        }
        // if blob lies within predicted position existing blob is updated with current blob
        if (dblLeastDistance < currentFrameBlob.CurrentDiagonalSize * 0.5) {
            addBlobToExistingCarBlobs(currentFrameBlob, existingCarBlobs, intIndexOfLeastDistance);
        }
        // if blob lies away from predicted position a completely new blob is added to the vector of blobs
        else {
            addNewBlob(currentFrameBlob, existingCarBlobs);
        }

    }

    // if a blob from the existing set is not updated for 5 consecutive iterations it is removed from the vector of blobs
    // this indicates that the car has passed the frame of the video
    for (auto &existingBlob : existingCarBlobs) {
        if (existingBlob.MatchFoundOrNewBlobOfCar == false) {
            existingBlob.NumConsecFramesWithoutAMatch++;
        }
        if (existingBlob.NumConsecFramesWithoutAMatch >= 5) {
            existingBlob.TrackingStatus = false;
        }
    }

}

// adds and replaces an existing blob of car with an updated version of the same blob
void addBlobToExistingCarBlobs(BlobOfCar &currentFrameBlob, std::vector<BlobOfCar> &existingCarBlobs, int &intIndex) {

    existingCarBlobs[intIndex].currentContour = currentFrameBlob.currentContour;
    existingCarBlobs[intIndex].currentBoundingRectangle = currentFrameBlob.currentBoundingRectangle;
    existingCarBlobs[intIndex].centerPoints.push_back(currentFrameBlob.centerPoints.back());
    existingCarBlobs[intIndex].CurrentDiagonalSize = currentFrameBlob.CurrentDiagonalSize;
    existingCarBlobs[intIndex].CurrentRectangleRatio = currentFrameBlob.CurrentRectangleRatio;
    existingCarBlobs[intIndex].TrackingStatus = true;
    existingCarBlobs[intIndex].MatchFoundOrNewBlobOfCar = true;
}

// adds a completely new blob, in the case a new car appears in the video
void addNewBlob(BlobOfCar &currentFrameBlob, std::vector<BlobOfCar> &existingCarBlobs) {

    currentFrameBlob.MatchFoundOrNewBlobOfCar = true;
    existingCarBlobs.push_back(currentFrameBlob);
}

// returns the distance between two given points in the image
double distanceBetweenPoints(cv::Point point1, cv::Point point2) {

    int intX = abs(point1.x - point2.x);
    int intY = abs(point1.y - point2.y);

    return(sqrt(pow(intX, 2) + pow(intY, 2)));
}

// draws contours of the cars on an image window using the built-in "drawContours" funciton in OpenCV
void drawAndShowContours(cv::Size imageSize, std::vector<std::vector<cv::Point> > contours, std::string strImageName) {
    cv::Mat image(imageSize, CV_8UC3, SCALAR_BLACK);
    cv::drawContours(image, contours, -1, SCALAR_WHITE, -1);
    cv::imshow(strImageName, image);
}

// draws contours of cars on an image window, but requires a vector of BlobOfCar as a parameter
void drawAndShowContours(cv::Size imageSize, std::vector<BlobOfCar> CarBlobs, std::string strImageName) {
    cv::Mat image(imageSize, CV_8UC3, SCALAR_BLACK);
    std::vector<std::vector<cv::Point> > contours;
    for (auto &blob : CarBlobs) {
        if (blob.TrackingStatus == true) {
            contours.push_back(blob.currentContour);
        }
    }

    cv::drawContours(image, contours, -1, SCALAR_WHITE, -1);
    cv::imshow(strImageName, image);
}

// checks if a blob of car crosses the reference line
bool checkIfCarBlobsCrossedTheLine(std::vector<BlobOfCar> &CarBlobs, int &intHorizontalLinePosition, int &carCount) {
    bool blnAtLeastOneBlobCrossedTheLine = false;

    for (auto blob : CarBlobs) {

        if (blob.TrackingStatus == true && blob.centerPoints.size() >= 2) {
            int prevFrameIndex = (int)blob.centerPoints.size() - 2;
            int currFrameIndex = (int)blob.centerPoints.size() - 1;
            if (blob.centerPoints[prevFrameIndex].y > intHorizontalLinePosition && blob.centerPoints[currFrameIndex].y <= intHorizontalLinePosition) {
                carCount++;
                blnAtLeastOneBlobCrossedTheLine = true;
            }
        }

    }
    return blnAtLeastOneBlobCrossedTheLine;
}

// draws the blob info on the screen. draws a rectangle, number and reference line
void drawBlobInfoOnImage(std::vector<BlobOfCar> &CarBlobs, cv::Mat &imgFrame2Copy) {

    for (unsigned int i = 0; i < CarBlobs.size(); i++) {
        if (CarBlobs[i].TrackingStatus == true) {
            cv::rectangle(imgFrame2Copy, CarBlobs[i].currentBoundingRectangle, SCALAR_RED, 2);
            int intFontFace = CV_FONT_HERSHEY_SIMPLEX;
            double dblFontScale = CarBlobs[i].CurrentDiagonalSize / 60.0;
            int intFontThickness = (int)std::round(dblFontScale * 1.0);
            cv::putText(imgFrame2Copy, std::to_string(i), CarBlobs[i].centerPoints.back(), intFontFace, dblFontScale, SCALAR_GREEN, intFontThickness);
        }
    }
}

// draws the total number of cars passed on the edge of the screen
void drawCarCountOnImage(int &carCount, cv::Mat &imgFrame2Copy) {

    int intFontFace = CV_FONT_HERSHEY_SIMPLEX;
    double dblFontScale = (imgFrame2Copy.rows * imgFrame2Copy.cols) / 300000.0;
    int intFontThickness = (int)std::round(dblFontScale * 1.5);
    cv::Size textSize = cv::getTextSize(std::to_string(carCount), intFontFace, dblFontScale, intFontThickness, 0);
    cv::Point ptTextBottomLeftPosition;
    ptTextBottomLeftPosition.x = imgFrame2Copy.cols - 1 - (int)((double)textSize.width * 1.25);
    ptTextBottomLeftPosition.y = (int)((double)textSize.height * 1.25);
    cv::putText(imgFrame2Copy, std::to_string(carCount), ptTextBottomLeftPosition, intFontFace, dblFontScale, SCALAR_GREEN, intFontThickness);

}

