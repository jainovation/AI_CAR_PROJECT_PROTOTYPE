#include <ros/ros.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include "JHPWMPCA9685.h"
#include "JHPWMPCA9685.cpp"
#include <autojoy/ControlMsg.h>
#include <autojoy/JoyMsg.h>
#include "ros_opencv_try/MsgACC.h"

#define THROTTLE_FULL_REVERSE 204
#define THROTTLE_NEUTRAL 307
#define THROTTLE_FULL_FORWARD 409

#define STEERING_CHANNEL 0
#define ESC_CHANNEL 1

#define servoMax  430
#define servoMid  340
#define servoMin  250

PCA9685 *pca9685 = new PCA9685();

int mode = 0;	//mode false -> JoyStick Mode
				//mode true  ->   Auto   Mode
int cnt;

void joyCallback(const autojoy::JoyMsg::ConstPtr& msg)
{
	if(msg->joy_cmd_mode == 1)   
	{
	 	printf("mode change: %d\n", mode);
		cnt++;
		if(cnt > 3)
		{
		 	mode = ~mode;
			cnt = 0;
		}
	}

 	if(mode == 0)
	{
	 	pca9685->setPWM(STEERING_CHANNEL, 0, servoMid + (-90 * msg->joy_cmd_lr));
		usleep(500);

		if(msg->joy_cmd_fb > 0)
		{
			printf("joystick: %lf\n", msg->joy_cmd_fb);
			pca9685->setPWM(ESC_CHANNEL, 0, 290 - (10 * msg->joy_cmd_fb));
			usleep(500);
		}
		else if(msg->joy_cmd_fb < 0)
		{
			printf("joystick: BACK\n");
			pca9685->setPWM(ESC_CHANNEL, 0, 370);
			usleep(500);
		}
		else if(msg->joy_cmd_fb == 0)
		{
			printf("joystick: MID\n");
			pca9685->setPWM(ESC_CHANNEL, 0, THROTTLE_NEUTRAL);
			usleep(500);
		}
	
		if(msg->joy_cmd_br == 1)
		{
			printf("reset\n");
			pca9685->setPWM(ESC_CHANNEL, 0, THROTTLE_NEUTRAL);
			usleep(500);
		}
	}
}

void msgCallback(const autojoy::ControlMsg::ConstPtr& msg)
{
 	if(mode == -1)
	{
	 	/*
		if(msg->control_sig == 1)
		{
			printf("go: %d\n",msg->control_sig);
			pca9685->setPWM(ESC_CHANNEL, 0, 290);
			usleep(500);
		}*/
	    if(msg->control_sig == 0)
		{
			printf("stop: %d\n",msg->control_sig);
			pca9685->setPWM(ESC_CHANNEL, 0, THROTTLE_NEUTRAL);
			usleep(500);
		}
	}
}

int motorMap(int x, int in_min, int in_max, int out_min, int out_max)
{
 	int toReturn = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min ;
	
	// For debugging:
	// printf("MAPPED %d to: %d\n", x, toReturn);

	return toReturn;
}

void openCallback(const ros_opencv_try::MsgACC::ConstPtr& msg)
{
 	if(mode == -1)
	{
	 	pca9685->setPWM(ESC_CHANNEL, 0, 285);
		usleep(500);
		pca9685->setPWM(0, 0, motorMap(msg->acc_cmd, 0, 180, servoMin, servoMax));
	}
}

int main(int argc, char **argv)
{
	ros::init(argc, argv, "motorControl");
	ros::NodeHandle nh;

	int err;
	err = pca9685->openPCA9685();
	if(err < 0) 	printf("Error: %d", pca9685->error);
	else
	{
		printf("PCA9685 Device Address: 0x%02X\n", pca9685->kI2CAddress);
		pca9685->setAllPWM(0,0);
		pca9685->reset();
		pca9685->setPWMFrequency(50);
	}

	usleep(10);
	pca9685->setPWM(ESC_CHANNEL, 0, THROTTLE_NEUTRAL);
	usleep(10);

	ros::Subscriber Motor_sub = nh.subscribe("Motor_msg", 1, msgCallback);
	ros::Subscriber Joy_sub = nh.subscribe("joystick_msg", 1, joyCallback);
	ros::Subscriber Open_sub = nh.subscribe("logic_msg", 1, openCallback);

	ros::spin();
}
