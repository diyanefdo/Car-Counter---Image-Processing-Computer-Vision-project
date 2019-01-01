// BlobOfCar.h

#ifndef MY_BLOBOFCAR
#define MY_BLOBOFCAR

#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

class BlobOfCar 
{

public:
    std::vector<cv::Point> currentContour;
    cv::Rect currentBoundingRectangle;
    std::vector<cv::Point> centerPoints;
    double CurrentDiagonalSize;
    double CurrentRectangleRatio;
    bool MatchFoundOrNewBlobOfCar;
    bool TrackingStatus;
    int NumConsecFramesWithoutAMatch;
    cv::Point NextAnticipatedPosition;

    BlobOfCar(std::vector<cv::Point> _contour);
    void predictNextPosition(void);

};

#endif    // MY_BLOBOFCAR

