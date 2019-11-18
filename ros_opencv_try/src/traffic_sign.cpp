#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <ros/ros.h>
#include <opencv2/highgui/highgui.hpp>

#include "OpenCV_Functions.h"
#include "OpenCV_Functions.cpp"
#include "OpenCV_Utils.h"
#include "OpenCV_Utils.cpp"

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{

	ros::init(argc, argv, "opencv_test");	

	cv::Mat image, image_gray, result;
	image = imread("/home/nvidia/Desktop/traffic_light.png",IMREAD_COLOR);

	cvtColor(image, image_gray, CV_BGR2GRAY ); 
	/* find circle */
	vector<Vec3f> circles;
	cv::HoughCircles(image_gray, circles, CV_HOUGH_GRADIENT, 1, 20, 50, 35, 30, 100); 
	drawHoughCircles(image, image, circles);

	Scalar red_lower(25, 0, 150); // BGR
	Scalar red_upper(80, 80, 255);
	
	Scalar green_lower(10, 100, 10);
	Scalar green_upper(110, 254, 90);

	for(int i=0; i<circles.size(); i++){

		vector<Point> src_pts, dst_pts;

		src_pts.push_back(Point(circles[i][0]-circles[i][2], circles[i][1]-circles[i][2]));
		src_pts.push_back(Point(circles[i][0]+circles[i][2], circles[i][1]-circles[i][2]));
		src_pts.push_back(Point(circles[i][0]-circles[i][2], circles[i][1]+circles[i][2]));
		src_pts.push_back(Point(circles[i][0]+circles[i][2], circles[i][1]+circles[i][2]));

/*		Point pt1 = src_pts[0];
		Point pt2 = src_pts[1];
		Point pt3 = src_pts[2];
		Point pt4 = src_pts[3];
		Scalar Color(255,0,0);
		drawLine(image, image, pt1, pt2, Color, 5);
		drawLine(image, image, pt2, pt4, Color, 5);
		drawLine(image, image, pt3, pt4, Color, 5);
		drawLine(image, image, pt1, pt3, Color, 5);
*/
/*		
		Mat mask2;
		convertColor(result, mask2, COLOR_BGR2HLS);
		inRange(mask2, white_lower, white_upper, mask2);
		white_image = imageCopy(mask2);
*/
	}

	
	cv::imshow("Image",image);
	waitKey(0);

	ros::spin();

	//cv::destroyAllWindows();

	return 0;

}
