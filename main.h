#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cmath>
#include <opencv2/opencv.hpp>

class Ball
{
public:
    int pos_x;
    int pos_y;
    int force;
    int autonation_factor;
    int radius;

    int velocity_x;
    int velocity_y;
    int current_applied_force_x;
    int current_applied_force_y;
    int drag_coefficient;
    float direction_x;
    float direction_y;
    float distance;
    Ball(int pos_x, int pos_y, int force, int autonation_factor, int radius);
    void apply_force(Ball ball, int force_threshold);
    void step(float step_size);
};

class Display
{
public:
    int size_x;
    int size_y;
    Ball *ball_array;
    int number_of_balls;
    cv::Mat display;

    Display(int size_x, int size_y, Ball *ball_array, int number_of_balls);
    void draw_display();
    void step(int step_size);
    void show_image();
};
