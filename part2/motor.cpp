#include "motor.h"
#include <cstdio>     // snprintf
#include <cstring>    // strlen
#include <fcntl.h>    // open
#include <unistd.h>   // write, close, usleep

#define PWM_FOLDER "/sys/class/pwm/pwmchip0/"


// Write a number (as text) to a sysfs file, like: echo 64000 > pwm0/duty_cycle
static void writeNumberToFile(const char *fileName, long number)
{
    char text[32];
    snprintf(text, sizeof(text), "%ld", number);

    int file = open(fileName, O_WRONLY);
    if (file < 0) {
        perror(fileName);
        return;
    }
    write(file, text, strlen(text));
    close(file);
}


void Motor::init()
{
    // Ask the kernel for pwm0 and pwm1. If they are already there, this just fails quietly.
    writeNumberToFile(PWM_FOLDER "export", 0);
    writeNumberToFile(PWM_FOLDER "export", 1);
    usleep(100000);   // give the kernel a moment to make the pwm0/ and pwm1/ folders

    // Both channels: 7.8 kHz, 0 % duty, on
    writeNumberToFile(PWM_FOLDER "pwm0/duty_cycle", 0);
    writeNumberToFile(PWM_FOLDER "pwm0/period", PWM_PERIOD_NANOSECONDS);
    writeNumberToFile(PWM_FOLDER "pwm0/enable", 1);

    writeNumberToFile(PWM_FOLDER "pwm1/duty_cycle", 0);
    writeNumberToFile(PWM_FOLDER "pwm1/period", PWM_PERIOD_NANOSECONDS);
    writeNumberToFile(PWM_FOLDER "pwm1/enable", 1);
}


// PWM on AIN1 (pwm0), AIN2 (pwm1) stays low
void Motor::setDutyCycle(double percent)
{
    if (percent < 0)   percent = 0;
    if (percent > 100) percent = 100;

    // Duty cycle is given in nanoseconds: percent of the period
    long dutyCycleNanoseconds = (long)(percent / 100.0 * PWM_PERIOD_NANOSECONDS);
    writeNumberToFile(PWM_FOLDER "pwm0/duty_cycle", dutyCycleNanoseconds);
}


void Motor::stop()
{
    setDutyCycle(0);
    writeNumberToFile(PWM_FOLDER "pwm0/enable", 0);
    writeNumberToFile(PWM_FOLDER "pwm1/enable", 0);
}
