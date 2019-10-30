#include <ros/ros.h>
#include <sensor_msgs/LaserScan.h>
#include <boost/asio.hpp>
#include <hls_lfcd_lds_driver/lfcd_laser.h>
#include <hls_lfcd_lds_driver/stopMsg.h>
#include <pthread.h>

#define RAD2DEG(x) ((x)*180./M_PI)

ros::Publisher stop_pub;
hls_lfcd_lds_driver::stopMsg msg;
int cmd;

using namespace std;

void msgCallback(const sensor_msgs::LaserScan::Ptr& scan)
{
	float cnt = scan->scan_time / scan->time_increment;

	for(int i = 0; i < 360; i++)
	{
		if((i > 175) && (i < 185))
		{
			ROS_INFO("DIRECTION: BACK");
			if(scan->ranges[i] == 0)
			{
				ROS_INFO("I can't detect");
				cmd = 0; // 0: too far or too near
			}
			else if(scan->ranges[i] < 0.1)
			{
				ROS_INFO("stop -> range[%d]: %lf",i,scan->ranges[i]);
				cmd = 1; // 1: stop
			}
			else
			{
				ROS_INFO("Don't stop -> range[%d]: %lf",i,scan->ranges[i]);
				cmd = 2; // 2: go
			}
		}
	}
	cout << "\033[2J\033[1;1H";
}

void *stop_Pub(void *data)
{
	ros::Rate loop_rate(10);
	
	while(ros::ok())
	{
		msg.stop_cmd = cmd;
		stop_pub.publish(msg);

		loop_rate.sleep();
	}
}

int main(int argc, char **argv)
{
	pthread_t thread_t;
	int tresh;
	int status;

	ros::init(argc, argv, "hls_laser_subscriber");
	ros::NodeHandle nh;

	ros::Subscriber hls_laser_sub = nh.subscribe("scan", 1000, msgCallback);
	
	stop_pub = nh.advertise<hls_lfcd_lds_driver::stopMsg>("stop_msg",100);

	if(pthread_create(&thread_t, NULL, stop_Pub, (void*) 0) < 0)
	{
		printf("thread error");
		exit(0);
	}

	ros::spin();

	pthread_join(thread_t, (void **)&status);

	return 0;
}
