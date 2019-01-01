// BlobOfCar.cpp

#include "BlobOfCar.h"


BlobOfCar::BlobOfCar(std::vector<cv::Point> _contour) 
{

    currentContour = _contour;
    currentBoundingRectangle = cv::boundingRect(currentContour);
    cv::Point currentCenter;

    currentCenter.x = (currentBoundingRectangle.x + currentBoundingRectangle.x + currentBoundingRectangle.width) / 2;
    currentCenter.y = (currentBoundingRectangle.y + currentBoundingRectangle.y + currentBoundingRectangle.height) / 2;

    centerPoints.push_back(currentCenter);
    CurrentDiagonalSize = sqrt(pow(currentBoundingRectangle.width, 2) + pow(currentBoundingRectangle.height, 2));
    dblCurrentRectangleRatio = (float)currentBoundingRectangle.width / (float)currentBoundingRectangle.height;
    TrackingStatus = true;
    MatchFoundOrNewBlobOfCar = true;
    NumConsecFramesWithoutAMatch = 0;
}


void BlobOfCar::predictNextPosition(void) 
{
    int numPositions = (int)centerPoints.size();
    if (numPositions == 1) 
    {
        NextAnticipatedPosition.x = centerPoints.back().x;
        NextAnticipatedPosition.y = centerPoints.back().y;

    } 
    else if (numPositions == 2) 
    {
        int deltaX = centerPoints[1].x - centerPoints[0].x;
        int deltaY = centerPoints[1].y - centerPoints[0].y;

        NextAnticipatedPosition.x = centerPoints.back().x + deltaX;
        NextAnticipatedPosition.y = centerPoints.back().y + deltaY;

    } 
    else if (numPositions == 3) 
    {
        int sumOfXChanges = ((centerPoints[2].x - centerPoints[1].x) * 2) +
            ((centerPoints[1].x - centerPoints[0].x) * 1);
        int deltaX = (int)std::round((float)sumOfXChanges / 3.0);
        int sumOfYChanges = ((centerPoints[2].y - centerPoints[1].y) * 2) +
            ((centerPoints[1].y - centerPoints[0].y) * 1);
        int deltaY = (int)std::round((float)sumOfYChanges / 3.0);
        NextAnticipatedPosition.x = centerPoints.back().x + deltaX;
        NextAnticipatedPosition.y = centerPoints.back().y + deltaY;

    } 
    else if (numPositions == 4) 
    {
        int sumOfXChanges = ((centerPoints[3].x - centerPoints[2].x) * 3) +
            ((centerPoints[2].x - centerPoints[1].x) * 2) +
            ((centerPoints[1].x - centerPoints[0].x) * 1);
        int deltaX = (int)std::round((float)sumOfXChanges / 6.0);
        int sumOfYChanges = ((centerPoints[3].y - centerPoints[2].y) * 3) +
            ((centerPoints[2].y - centerPoints[1].y) * 2) +
            ((centerPoints[1].y - centerPoints[0].y) * 1);
        int deltaY = (int)std::round((float)sumOfYChanges / 6.0);
        NextAnticipatedPosition.x = centerPoints.back().x + deltaX;
        NextAnticipatedPosition.y = centerPoints.back().y + deltaY;

    } 
    else if (numPositions >= 5) 
    {
        int sumOfXChanges = ((centerPoints[numPositions - 1].x - centerPoints[numPositions - 2].x) * 4) +
            ((centerPoints[numPositions - 2].x - centerPoints[numPositions - 3].x) * 3) +
            ((centerPoints[numPositions - 3].x - centerPoints[numPositions - 4].x) * 2) +
            ((centerPoints[numPositions - 4].x - centerPoints[numPositions - 5].x) * 1);
        int deltaX = (int)std::round((float)sumOfXChanges / 10.0);
        int sumOfYChanges = ((centerPoints[numPositions - 1].y - centerPoints[numPositions - 2].y) * 4) +
            ((centerPoints[numPositions - 2].y - centerPoints[numPositions - 3].y) * 3) +
            ((centerPoints[numPositions - 3].y - centerPoints[numPositions - 4].y) * 2) +
            ((centerPoints[numPositions - 4].y - centerPoints[numPositions - 5].y) * 1);
        int deltaY = (int)std::round((float)sumOfYChanges / 10.0);
        NextAnticipatedPosition.x = centerPoints.back().x + deltaX;
        NextAnticipatedPosition.y = centerPoints.back().y + deltaY;
    } else {
        // THE PROGRAM SHOULD NEVER REACH THIS POINT
    }

}

