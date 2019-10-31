#include <ros/ros.h>
#include <sensor_msgs/Joy.h>
#include <pthread.h>
#include <autojoy/JoyMsg.h>

ros::Publisher joy_pub;
autojoy::JoyMsg msg;
double cmd_lr;
double cmd_fb;
int cmd_br;

void joyCallback(const sensor_msgs::Joy::ConstPtr& joy)
{
	std::cout << "L/R: " << joy->axes[0] << std::endl;
	std::cout << "U/D: " << joy->axes[1] << std::endl;

	cmd_lr = joy->axes[0];
	cmd_fb = joy->axes[1];
	cmd_br = joy->buttons[2];
}

void *joy_Pub(void *data)
{
	ros::Rate loop_rate(20);

	while(ros::ok())
	{
		msg.joy_cmd_lr = cmd_lr;
		msg.joy_cmd_fb = cmd_fb;
		msg.joy_cmd_br = cmd_br;

		joy_pub.publish(msg);

		loop_rate.sleep();
	}
}
int main(int argc, char **argv)
{
	pthread_t thread_t;
	int status;

	ros::init(argc, argv, "joyStick");
	ros::NodeHandle nh;
	
	ros::Subscriber joy_sub = nh.subscribe<sensor_msgs::Joy>("joy", 10, joyCallback);
	
	joy_pub = nh.advertise<autojoy::JoyMsg>("joystick_msg",10);
	if(pthread_create(&thread_t, NULL, joy_Pub, (void*) 0) < 0)
	{
		printf("thread error");
		exit(0);
	}

	ros::spin();

	pthread_join(thread_t, (void **)&status);
}
