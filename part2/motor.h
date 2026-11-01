// Motor class: drives the motor driver with hardware PWM through sysfs (lecture 5.6).
//   pwm0 = GPIO12 = driver AIN1
//   pwm1 = GPIO13 = driver AIN2
// PWM on AIN1 and AIN2 low = the motor turns one way (like Project 4).
#ifndef MOTOR_H
#define MOTOR_H

class Motor
{
public:
    // 128 000 ns = 7.8 kHz, same PWM frequency as Project 4
    static const int PWM_PERIOD_NANOSECONDS = 128000;

    void init();                              // turn on both PWM channels at 0 %
    void setDutyCycle(double percent);        // 0 to 100 %
    void stop();                              // 0 % and turn the PWM channels off
};

#endif
