#include <ros/ros.h>
#include <sensor_msgs/LaserScan.h>
#include <autojoy/LidarMsg.h>
#include <autojoy/carAccMsg.h>
#include <boost/asio.hpp>
#include <pthread.h>
#include <stdlib.h>

ros::Publisher Speed_pub;
autojoy::carAccMsg msg;

bool cmd = false;

void msgCallback(const sensor_msgs::LaserScan::Ptr& scan)
{
 	std::cout << scan->ranges[0] << std::endl;
 	if(scan->ranges[0] <= 1.9)
	 	cmd = 0;
	else if(scan->ranges[0] >= 2.1)
	 	cmd = 1;
	else if((1.9<scan->ranges[0]) && (scan->ranges[0] < 2.1))
	 	cmd = 2;
}
void *ACC_Pub(void *data)
{
 	ros::Rate loop_rate(20);

	while(ros::ok())
	{
	 	if(cmd == 1)				msg.speed_cmd = 1;
		else if(cmd == 0)			msg.speed_cmd = 0;
		else if(cmd == 2)			msg.speed_cmd = 2;

		Speed_pub.publish(msg);

		loop_rate.sleep();
	}
}
int main(int argc, char** argv)
{
	pthread_t thread_t;
	int status;

	ros::init(argc, argv, "ACC_CAR");
	ros::NodeHandle nh;

	ros::Subscriber ACC_Sub = nh.subscribe("scan", 1000, msgCallback);
	
	Speed_pub = nh.advertise<autojoy::carAccMsg>("carAccMsg",1);

	if(pthread_create(&thread_t, NULL, ACC_Pub, (void*) 0) < 0)
	{
		printf("thread error");
		exit(0);
	}
	ros::spin();

	pthread_join(thread_t, (void **)&status);
}
