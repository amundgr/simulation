#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cmath>
#include <opencv2/opencv.hpp>
#include <chrono>

double sigmoid(float distance, float radius, float rolloff)
{
    return 1 - (1 / (1 + exp(rolloff * (-distance + (radius * 2)))));
}

class Ball
{
public:
    double pos_x;
    double pos_y;
    double force;
    double autonation_factor;
    int radius;

    double velocity_x;
    double velocity_y;
    double current_applied_force_x;
    double current_applied_force_y;
    double drag_coefficient;
    Ball(float pos_x, float pos_y, double force, double autonation_factor, int radius, double drag_coefficient)
    {
        this->pos_x = pos_x;
        this->pos_y = pos_y;
        this->force = force;
        this->autonation_factor = autonation_factor;
        this->radius = radius;
        this->drag_coefficient = drag_coefficient;
        this->velocity_x = 0;
        this->velocity_y = 0;
        this->current_applied_force_x = 0;
        this->current_applied_force_y = 0;
    }

    double direction_x;
    double direction_y;
    double force_x;
    double force_y;
    double distance;
    double scaled_force;

    // Remove entire force calulation and replace with predefined applied force
    void apply_force(Ball *ball, int force_threshold)
    {
        this->current_applied_force_x = 0;
        this->current_applied_force_y = 0;

        this->direction_x = ball->pos_x - this->pos_x;
        this->direction_y = ball->pos_y - this->pos_y;

        this->distance = sqrt(pow(this->direction_x, 2) + pow(this->direction_y, 0.5));

        this->scaled_force = sigmoid(this->distance, this->radius, 1) * this->force;

        this->force_x = (this->direction_x / this->distance) * this->scaled_force;
        this->force_y = (this->direction_y / this->distance) * this->scaled_force;

        if (isnan(this->force_x))
        {
            this->force_x = force_threshold * (rand() % 2 - 0.5);
        }
        else if (abs(this->force_x) > force_threshold)
        {
            this->force_x = force_threshold * (this->force_x / abs(this->force_x));
        }
        if (isnan(this->force_y))
        {
            this->force_y = force_threshold * (rand() % 2 - 0.5);
        }
        else if (abs(this->force_y) > force_threshold)
        {
            this->force_y = force_threshold * (this->force_y / abs(this->force_y));
        }

        //std::cout << "Scaled force: " << this->distance << std::endl;
        //std::cout << "Actual force: " << this->scaled_force << std::endl;

        this->current_applied_force_x -= this->force_x;
        this->current_applied_force_y -= this->force_y;

        ball->current_applied_force_x += this->force_x;
        ball->current_applied_force_y += this->force_y;
    }

    double delta_x;
    double delta_y;

    void step(double step_size, int max_x, int max_y)
    {
        this->current_applied_force_y += 10; //Gravity
        this->velocity_x += this->current_applied_force_x - (this->velocity_x * this->drag_coefficient);
        this->velocity_y += this->current_applied_force_y - (this->velocity_y * this->drag_coefficient);
        this->delta_x = this->velocity_x * step_size;
        this->delta_y = this->velocity_y * step_size;

        //this->puke_info();

        this->pos_x += this->delta_x;
        this->pos_y += this->delta_y;

        this->current_applied_force_x = 0;
        this->current_applied_force_y = 0;

        if (this->pos_x < (0 + this->radius))
        {
            this->pos_x = this->radius;
            this->velocity_x = -this->velocity_x;
        }
        else if ((this->pos_x + this->radius) > max_x)
        {
            this->pos_x = max_x - this->radius;
            this->velocity_x = -this->velocity_x;
        }

        if (this->pos_y < (0 + this->radius))
        {
            this->pos_y = this->radius;
            this->velocity_y = -this->velocity_y;
        }
        else if ((this->pos_y + this->radius) > max_y)
        {
            this->pos_y = max_y - this->radius;
            this->velocity_y = -this->velocity_y;
        }
    }

    void puke_info()
    {
        std::cout << "Pos: " << this->pos_x << ", " << this->pos_y << " | ";
        std::cout << "Delta: " << this->delta_x << ", " << this->delta_y << " | ";
        std::cout << "Force: " << this->current_applied_force_x << ", " << this->current_applied_force_y << " | ";
        std::cout << "Vel: " << this->velocity_x << ", " << this->velocity_y << std::endl;
    }
};

class Display
{
public:
    int size_x;
    int size_y;
    Ball *ball_array;
    int number_of_balls;
    cv::Mat display;

    Display(int size_x, int size_y, Ball *ball_array, int number_of_balls)
    {
        this->size_x = size_x;
        this->size_y = size_y;
        this->ball_array = ball_array;
        this->number_of_balls = number_of_balls;
    }

    void draw_display()
    {
        this->display = cv::Mat::zeros(cv::Size(this->size_x, this->size_y), CV_8UC1);
        for (int i = 0; i < this->number_of_balls; i++)
        {
            cv::circle(this->display, cv::Point((int)this->ball_array[i].pos_x, (int)this->ball_array[i].pos_y),
                       this->ball_array[i].radius, cv::Scalar(255, 255, 255), cv::FILLED, 8, 0);
        }
    }

    void step(double step_size)
    {
        for (int i = 0; i < (this->number_of_balls - 1); i++)
        {
            for (int j = i + 1; j < this->number_of_balls; j++)
            {
                this->ball_array[i].apply_force(&this->ball_array[j], 20);
            }
        }
        for (int i = 0; i < this->number_of_balls; i++)
        {
            this->ball_array[i].step(step_size, this->size_x, this->size_y);
        }
    }

    void show_image()
    {
        cv::imshow("Simulation", this->display);
        //cv::waitKey(0);
    }
};

int main(int argc, char **argv)
{

    int itterations = 1000;
    double step_size = 0.1;

    int number_of_balls = 30;
    int ball_radius = 15;
    double force = 100;
    double af = 2;
    double dc = 0.2;
    int display_size_x = 300;
    int display_size_y = 300;
    Ball *ball_array = (Ball *)malloc(sizeof(Ball) * number_of_balls);

    srand(time(NULL));
    double random_pos_x, random_pos_y;
    for (int i = 0; i < number_of_balls; i++)
    {
        random_pos_x = (double)(rand() % (display_size_x - ball_radius * 2) + 1 + ball_radius);
        random_pos_y = (double)(rand() % (display_size_y - ball_radius * 2) + 1 + ball_radius);
        //std::cout << random_pos_x << ", " << random_pos_y << std::endl;
        ball_array[i] = Ball(random_pos_x, random_pos_y, force, af, ball_radius, dc);
    }

    //ball_array[0] = Ball(100 - ball_radius, 100, force, af, ball_radius, dc);
    //ball_array[1] = Ball(100 + ball_radius, 100, force, af, ball_radius, dc);
    Display display = Display(display_size_x, display_size_y, ball_array, number_of_balls);
    display.draw_display();
    display.show_image();
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < itterations; i++)
    {
        //std::cout << i << std::endl;
        display.step(step_size);
        display.draw_display();
        display.show_image();
        cv::waitKey(100);
    }
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << duration.count() / itterations << std::endl;
    cv::waitKey(0);
    return 0;
}
