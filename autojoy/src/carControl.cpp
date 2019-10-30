#include <ros/ros.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <autojoy/DetectMsg.h>
#include <autojoy/LidarMsg.h>
#include <autojoy/ControlMsg.h>
#include <pthread.h>

#define STEERING_CHANNEL 0
#define ESC_CHANNEL 1

int cmd_detect = 0;
int cmd_lidar = 0;

ros::Publisher Control_pub;
autojoy::ControlMsg msg;

void cmd_Detect(const autojoy::DetectMsg::ConstPtr& msg)
{
	if(msg->detect_cmd == false)
	{
		printf("I can't Detect(Yolo)\n");
		cmd_detect = 0;
	}
	else if(msg->detect_cmd == true)
	{
		printf("Car detect!!!!!!\n");
		cmd_detect = 1;
	}
}

void cmd_Lidar(const autojoy::LidarMsg::ConstPtr& msg)
{
	if(msg->lidar_cmd == 0)
	{
		printf("I can't Detect(LIDAR)\n");
		cmd_lidar = 0;
	}
	else if(msg->lidar_cmd == 1)
	{
		printf("Lidar detect!!!!!!\n");
		cmd_lidar = 1;
	}
	else if(msg->lidar_cmd == 2)
	{
		printf("GOGOGOGOGOGOGO\n");
		cmd_lidar = 2;
	}
}

void *dc_Control_cmd(void *data)
{
	ros::Rate loop_rate(10);

	while(ros::ok())
	{
		if((cmd_detect == 1) && (cmd_lidar == 2))
		{
			msg.control_sig = 1;
		}
		else
		{
			msg.control_sig = 0;
		}
		printf("%d\n", msg.control_sig);
		Control_pub.publish(msg);
		loop_rate.sleep();
	}
}

int main(int argc, char **argv)
{
	pthread_t thread_t;
	int status;

	ros::init(argc, argv, "carControl");
	ros::NodeHandle nh;

	ros::Subscriber detect_cmd_sub = nh.subscribe("Detect_msg", 1, cmd_Detect);
	ros::Subscriber lidar_cmd_sub = nh.subscribe("Lidar_msg", 1, cmd_Lidar);

	Control_pub = nh.advertise<autojoy::ControlMsg>("Motor_msg",1);

	if(pthread_create(&thread_t, NULL, dc_Control_cmd, (void*) 0) < 0)
	{
		printf("thread error");
		exit(0);
	}

	ros::spin();

	pthread_join(thread_t, (void **)& status);
}
