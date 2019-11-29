#include <ros/ros.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <autojoy/DetectMsg.h>
#include <autojoy/LidarMsg.h>
#include <autojoy/TrafficMsg.h>
#include <autojoy/carAccMsg.h>
#include "ros_opencv_try/MsgACC.h"
#include <autojoy/Motor_msg.h>
#include <pthread.h>

int cmd_detect = 0;
int cmd_lidar = 0;
int cmd_opencv = 0;
int cmd_trafficlight = 0;
int cmd_acc = 0;

ros::Publisher Control_pub;
autojoy::Motor_msg msg;

void cmd_Traffic(const autojoy::TrafficMsg::ConstPtr& msg)
{
	msg->traffic_cmd == true? cmd_trafficlight = 1 : cmd_trafficlight = 0;
}

void cmd_Detect(const autojoy::DetectMsg::ConstPtr& msg)
{
 	msg->detect_cmd == false ? cmd_detect = 0 : cmd_detect = 1;
}

void cmd_Lidar(const autojoy::LidarMsg::ConstPtr& msg)
{
	if(msg->lidar_cmd == 1)
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

void cmd_openCV(const ros_opencv_try::MsgACC::ConstPtr& msg)
{
 	cmd_opencv = msg->acc_cmd;
}

void cmd_Acc(const autojoy::carAccMsg::ConstPtr& msg)
{
 	cmd_acc = msg->speed_cmd;
}

void *dc_Control_cmd(void *data)
{
	ros::Rate loop_rate(20);

	while(ros::ok())
	{
		if(cmd_trafficlight == 0)
		{				
			if((cmd_detect == 1) && (cmd_lidar == 1))
			{
				msg.control_sig = 0;
			}
			else
			{
				msg.control_sig = 1;
			}
		}
		else if(cmd_trafficlight ==1)
		{
			msg.control_sig = 0;
		}
		
		printf("speed_cmd =  %d\n", cmd_acc);
		msg.angle_sig = cmd_opencv;
		msg.cmd_ACC = cmd_acc;

		printf("%d\n", msg.control_sig);
		printf("%d\n", msg.angle_sig);

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
	ros::Subscriber traffic_cmd_sub = nh.subscribe("Traffic_msg", 1, cmd_Traffic);
	ros::Subscriber detect_cmd_sub = nh.subscribe("Detect_msg", 1, cmd_Detect);
	ros::Subscriber lidar_cmd_sub = nh.subscribe("Lidar_msg", 1, cmd_Lidar);
	ros::Subscriber openCV_cmd_sub = nh.subscribe("logic_msg", 1, cmd_openCV);
	ros::Subscriber ACC_cmd_sub = nh.subscribe("carAccMsg", 1, cmd_Acc);

	Control_pub = nh.advertise<autojoy::Motor_msg>("Motor_msg",1);

	if(pthread_create(&thread_t, NULL, dc_Control_cmd, (void*) 0) < 0)
	{
		printf("thread error");
		exit(0);
	}

	ros::spin();

	pthread_join(thread_t, (void **)& status);
}
