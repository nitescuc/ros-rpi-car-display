#include "ros/ros.h"
#include "sensor_msgs/Joy.h"

#include <pigpiod_if2.h>

int pi;

int remote_mode = 0, pilot_mode = 0;
int red = 16, green = 21, blue = 20;

bool isAutoSteering() { return remote_mode > -1; }
bool isAutoThrottle() { return remote_mode == 1; }

void setConfiguration(int in_pilot_mode, int in_remote_mode)
{
    if (pilot_mode == in_pilot_mode && remote_mode == in_remote_mode) return;

    pilot_mode = in_pilot_mode;
    remote_mode = in_remote_mode;
    
    if ( isAutoSteering() && isAutoThrottle() )
    {
        gpio_write(pi, green, 0);
        gpio_write(pi, red, 1);
        gpio_write(pi, blue, 0);
    }
    if ( isAutoSteering() && !isAutoThrottle() ) {
        gpio_write(pi, red, 0);
        if (pilot_mode == 1) {
            gpio_write(pi, blue, 1);
            gpio_write(pi, green, 0);
        } else {
            gpio_write(pi, blue, 1);
            gpio_write(pi, green, 1);
        }
    }
    if ( !isAutoSteering() && !isAutoThrottle() )
    {
        gpio_write(pi, green, 1);
        gpio_write(pi, red, 0);
        gpio_write(pi, blue, 0);
    }
}

void remote_callback(const sensor_msgs::Joy joy)
{
    setConfiguration(pilot_mode, joy.buttons[0]);
}

void pilot_callback(const sensor_msgs::Joy joy)
{
    setConfiguration(joy.buttons[0], remote_mode);
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "status_led");
    ros::NodeHandle n;

    // Init GPIO
    pi = pigpio_start(NULL, NULL);
    set_mode(pi, red, PI_OUTPUT);
    set_mode(pi, green, PI_OUTPUT);
    set_mode(pi, blue, PI_OUTPUT);

    setConfiguration(0, 0);
    // Handle ROS communication events
    ros::Subscriber remote = n.subscribe("/remote", 1, remote_callback);   
    ros::Subscriber pilot = n.subscribe("/pilot", 1, pilot_callback);

    ros::spin();

    pigpio_stop(pi);

    return 0;
}
