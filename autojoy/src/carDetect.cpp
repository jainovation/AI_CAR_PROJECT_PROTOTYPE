#include <ros/ros.h>
#include "darknet_ros_msgs/BoundingBoxes.h"
#include "darknet_ros_msgs/BoundingBox.h"
#include <pthread.h>
#include <autojoy/DetectMsg.h>
#include <stdlib.h>

ros::Publisher detect_pub;
autojoy::DetectMsg msg;
std::string target = "car";

int timer1;

bool cmd = false;
void timerCallback(const ros::TimerEvent&)
{
	printf("timer1: %d\n",timer1);
	timer1++;
	if(timer1 > 3)		cmd = false;
	if(timer1 > 10000)  timer1 = 0;
}


void carDetection(const darknet_ros_msgs::BoundingBoxes::ConstPtr& msg)
{
	if(msg->bounding_boxes[0].Class.compare(target) == 0)
	{
		printf("car Detection!!!\n");
		cmd = true;
		timer1 = 0;
	}
	/*
	else if(msg->bounding_boxes[0].Class.compare(target) != 0)
	{
		printf("I can't detected\n");
		cmd = false;
	}*/
	
	//std::cout << "\033[2J\033[1;1H";
}

void *Detect_pub(void *data)
{
	ros::Rate loop_rate(20);

	while(ros::ok())
	{
		msg.detect_cmd = cmd;
		detect_pub.publish(msg);

		loop_rate.sleep();
	}
}

int main(int argc, char **argv)
{
	pthread_t thread_t;
	int status;

	ros::init(argc, argv, "carDetect");
	ros::NodeHandle nh;
	
	ros::Timer timer = nh.createTimer(ros::Duration(0.1), timerCallback);
	ros::Subscriber car_sub = nh.subscribe("/darknet_ros/bounding_boxes",1, carDetection);
	
	detect_pub = nh.advertise<autojoy::DetectMsg>("Detect_msg",1);

	if(pthread_create(&thread_t, NULL, Detect_pub, (void*) 0) < 0)
	{
		printf("thread create error");
		exit(0);
	}
	
	ros::spin();
	
	pthread_join(thread_t, (void **)&status);
}
