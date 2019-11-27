#include <ros/ros.h>
#include "darknet_ros_msgs/BoundingBoxes.h"
#include "darknet_ros_msgs/BoundingBox.h"
#include <pthread.h>
#include <autojoy/TrafficMsg.h>
#include <stdlib.h>

#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <image_transport/image_transport.h>

#include "OpenCV_Functions.h"
#include "OpenCV_Functions.cpp"
#include "OpenCV_Utils.h"
#include "OpenCV_Utils.cpp"

using namespace cv;
using namespace std;

ros::Publisher detect_pub;
autojoy::TrafficMsg msg;
std::string target = "traffic light";
cv::Mat image;

int timer1;
int Xmin, Xmax, Ymin, Ymax;

bool cmd = false;
void timerCallback(const ros::TimerEvent&)
{
	printf("timer1: %d\n",timer1);
	timer1++;
	if(timer1 > 3)		cmd = false;
	if(timer1 > 10000)  timer1 = 0;
}


void traffic_light_detect(const darknet_ros_msgs::BoundingBoxes::ConstPtr& msg)
{
	for(int k=0; k < msg->bounding_boxes.size(); k++)
	{
		if( !msg->bounding_boxes[k].Class.empty() )
		{
		//cout<<msg->bounding_boxes[k].Class<<endl;
			if(msg->bounding_boxes[k].Class.compare(target) == 0)
			{
				printf("==============================================\n");
				printf("Traffic Light Detected!!!\n");
				printf("Probability : %f\n",msg->bounding_boxes[0].probability);
				printf("xmin : %d\n",msg->bounding_boxes[k].xmin);
				printf("ymin : %d\n",msg->bounding_boxes[k].ymin);
				printf("xmax : %d\n",msg->bounding_boxes[k].xmax);
				printf("ymax : %d\n",msg->bounding_boxes[k].ymax);
				printf("==============================================\n");
				//std::cout << "\033[2J\033[1;1H"; // clear terminal

				Xmin = msg->bounding_boxes[k].xmin;
				Ymin = msg->bounding_boxes[k].ymin;
				Xmax = msg->bounding_boxes[k].xmax;
				Ymax = msg->bounding_boxes[k].ymax;

				timer1 = 0;
			}
			else
			{
				Xmin = 0;
				Xmax = 1;
				Ymin = 0;
				Ymax = 1;

				timer1 = 0;
			}
		}
	}
		
	//std::cout << "\033[2J\033[1;1H";
}


void camera_callback(const sensor_msgs::ImageConstPtr& msg)
{
	image = cv_bridge::toCvShare(msg, "bgr8")->image;
	//resize(image, image, Size(640, 360));

	Mat cam_image;
	image.copyTo(cam_image);
	
	/* draw roi */
/*	vector<Point> src_pts;

	src_pts.push_back( Point(Xmin, Ymin) );
	src_pts.push_back( Point(Xmax, Ymin) );
	src_pts.push_back( Point(Xmin, Ymax) );
	src_pts.push_back( Point(Xmax, Ymax) );

	Point pt1 = src_pts[0];
	Point pt2 = src_pts[1];
	Point pt3 = src_pts[2];
	Point pt4 = src_pts[3];
	Scalar Color(255,0,0);
	drawLine(cam_image, cam_image, pt1, pt2, Color, 5);
	drawLine(cam_image, cam_image, pt2, pt4, Color, 5);
	drawLine(cam_image, cam_image, pt3, pt4, Color, 5);
	drawLine(cam_image, cam_image, pt1, pt3, Color, 5);
*/
	Rect rect(Xmin, Ymin, Xmax-Xmin , Ymax-Ymin);
	Mat image_roi = cam_image(rect);

	/* find circle */
/*	Mat image_gray;
	cvtColor(image_roi, image_gray, CV_BGR2GRAY ); 
	vector<Vec3f> circles;
	cv::HoughCircles(image_gray, circles, CV_HOUGH_GRADIENT, 1, 20, 50, 35, 100, 200); 
	drawHoughCircles(image_roi, image_roi, circles);
*/
	Scalar red_lower(0, 0, 0); // hls
	Scalar red_upper(30, 255, 255);
	
	//Scalar red_lower(0, 75, 100); // hls
	//Scalar red_upper(8, 255, 255);

	Scalar green_lower(10, 100, 10);
	Scalar green_upper(110, 254, 90);

	Mat image_hls;
	convertColor(image_roi, image_hls, COLOR_BGR2HLS);
	inRange(image_hls, red_lower, red_upper, image_hls);
	
	imshow("view", image_hls);
	cv::waitKey(30);

	int resultPixel = 0;
	for (int x = 0; x < image_hls.rows; x++)
	{
		for (int y = 0; y < image_hls.cols; y++)
		{
			if (image_hls.at<uchar>(x, y) == 255)     // white : 255
				resultPixel++;
		}
	}

	if(resultPixel > 100)
		cmd = true;
	else
		cmd = false;

}

void *TrafficLight_Detect_pub(void *data)
{
	ros::Rate loop_rate(20);

	while(ros::ok())
	{
		msg.traffic_cmd = cmd;
		detect_pub.publish(msg);

		loop_rate.sleep();
	}
}

int main(int argc, char **argv)
{
	pthread_t thread_t;
	int status;

	ros::init(argc, argv, "ros_traffic_light");
	ros::NodeHandle nh;
	
	ros::Timer timer = nh.createTimer(ros::Duration(0.1), timerCallback);
	ros::Subscriber car_sub = nh.subscribe("/darknet_ros/bounding_boxes",1, traffic_light_detect);

	/* camera subscriber */
	cv::namedWindow("view");
	cv::startWindowThread();
	image_transport::ImageTransport it(nh);
	image_transport::Subscriber sub = it.subscribe("/camera/image_raw", 1, camera_callback);	

	
	detect_pub = nh.advertise<autojoy::TrafficMsg>("Traffic_msg",1);

	if(pthread_create(&thread_t, NULL, TrafficLight_Detect_pub, (void*) 0) < 0)
	{
		printf("thread create error");
		exit(0);
	}
	
	ros::spin();
	
	pthread_join(thread_t, (void **)&status);
}
