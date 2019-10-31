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

#define THROTTLE_FULL_REVERSE 204
#define THROTTLE_NEUTRAL 307
#define THROTTLE_FULL_FORWARD 409

#define STEERING_CHANNEL 0
#define ESC_CHANNEL 1

#define servoMax  430
#define servoMid  340
#define servoMin  250

PCA9685 *pca9685 = new PCA9685();

void joyCallback(const autojoy::JoyMsg::ConstPtr& msg)
{
	if(msg->joy_cmd_lr > 0)
	{
		printf("joystick: LEFT\n");
		pca9685->setPWM(STEERING_CHANNEL, 0, servoMid + (90 * msg->joy_cmd_lr));
		usleep(500);
	}
	else if(msg->joy_cmd_lr < 0)
	{
		printf("joystick: RIGHT\n");
		pca9685->setPWM(STEERING_CHANNEL, 0, servoMid + (90 * msg->joy_cmd_lr));
		usleep(500);
	}
	else if(msg->joy_cmd_lr == 0)
	{
		printf("joystick: MID\n");
		pca9685->setPWM(STEERING_CHANNEL, 0, 340);
		usleep(500);
	}

	if(msg->joy_cmd_fb > 0)
	{
		printf("joystick: %lf\n", msg->joy_cmd_fb);
		pca9685->setPWM(ESC_CHANNEL, 0, 295 - (5 * msg->joy_cmd_fb));
		usleep(500);
	}
	else if(msg->joy_cmd_fb < 0)
	{
		printf("joystick: BACK\n");
		pca9685->setPWM(ESC_CHANNEL, 0, 340);
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

void msgCallback(const autojoy::ControlMsg::ConstPtr& msg)
{
	if(msg->control_sig == 1)
	{
		printf("go: %d\n",msg->control_sig);
		pca9685->setPWM(ESC_CHANNEL, 0, 290);
		sleep(1);
	}
	else if(msg->control_sig == 0)
	{
		printf("stop: %d\n",msg->control_sig);
		pca9685->setPWM(ESC_CHANNEL, 0, THROTTLE_NEUTRAL);
		sleep(1);
		pca9685->setPWM(ESC_CHANNEL, 0, THROTTLE_NEUTRAL);
		sleep(1);
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
	ros::Subscriber Joy_sub = nh.subscribe("joystick_msg", 10, joyCallback);

	ros::spin();
}
