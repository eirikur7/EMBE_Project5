// Part 2: speed control of the motor, with the PI controller from Project 4.
//   - The encoder count comes from our kernel module (/dev/encoder).
//   - The motor is driven with hardware PWM (sysfs).
//   - The control loop uses usleep() as its clock, like the assignment asks.

#include <cstdio>      // printf, fopen
#include <cstdlib>     // atof, atoi
#include <csignal>     // signal (Ctrl+C)
#include <ctime>       // clock_gettime
#include <unistd.h>    // usleep

#include "encoder.h"
#include "motor.h"
#include "PI_controller.h"

// Same values as Project 4
static const int    CONTROL_PERIOD_MICROSECONDS = 3900;     // 3.9 ms control window
static const double CONTROL_KP = 4.5;
static const double CONTROL_TI = 0.016;                     // [s]
static const double CONTROL_T  = CONTROL_PERIOD_MICROSECONDS / 1000000.0;   // [s]

Encoder       encoder;
Motor         motor;
PI_controller piController(CONTROL_KP, CONTROL_TI, CONTROL_T);


// Set to false by Ctrl+C, so the loop ends and we can stop the motor
static volatile bool keepRunning = true;

static void ctrlCPressed(int signalNumber)
{
    keepRunning = false;
}


// The time right now in seconds
static double timeNowSeconds()
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec + now.tv_nsec / 1000000000.0;
}


int main(int argc, char **argv)
{
    double referenceSpeedRpm = 50;
    if (argc > 1)
        referenceSpeedRpm = atof(argv[1]);

    int runTimeSeconds = 10;
    if (argc > 2)
        runTimeSeconds = atoi(argv[2]);

    const char *logFileName = "log.csv";
    if (argc > 3)
        logFileName = argv[3];

    signal(SIGINT, ctrlCPressed);


    // Set up the hardware and the controller
    encoder.init();
    motor.init();
    piController.init();                 // start from a zero integral
    piController.set_limits(0, 100);     // duty cycle 0 to 100 %

    FILE *logFile = fopen(logFileName, "w");
    fprintf(logFile, "loop_time_us,speed_rpm,duty_percent\n");

    printf("Reference %.0f rpm, running for %d s. Ctrl+C to stop.\n", referenceSpeedRpm, runTimeSeconds);


    double startTime = timeNowSeconds();
    double lastLoopTime = startTime;
    int lastCount = encoder.getCount();

    while (keepRunning && timeNowSeconds() - startTime < runTimeSeconds) {

        // Wait one control window
        usleep(CONTROL_PERIOD_MICROSECONDS);


        // How long did this loop really take? (usleep is never exact)
        double now = timeNowSeconds();
        double loopTimeSeconds = now - lastLoopTime;
        lastLoopTime = now;


        // Speed = how far the shaft turned / how long it took
        int count = encoder.getCount();
        int countsThisLoop = count - lastCount;
        lastCount = count;

        double revolutions = (double)countsThisLoop / Encoder::COUNTS_PER_REVOLUTION;
        double speedRpm = revolutions / (loopTimeSeconds / 60.0);


        // Use the real loop time in the integral, not the 3.9 ms we asked for
        piController.T = loopTimeSeconds;

        // Controller: reference + speed in, duty cycle out
        double dutyCyclePercent = piController.update(referenceSpeedRpm, speedRpm);
        motor.setDutyCycle(dutyCyclePercent);


        // Print and save
        printf("loop %.2f ms   speed %6.1f rpm   duty %5.1f %%\n",
               loopTimeSeconds * 1000, speedRpm, dutyCyclePercent);

        fprintf(logFile, "%.1f,%.1f,%.1f\n", loopTimeSeconds * 1000000, speedRpm, dutyCyclePercent);
    }


    // Done: stop the motor and close everything
    motor.stop();
    encoder.close();
    fclose(logFile);

    printf("Stopped. Measurements saved in %s\n", logFileName);
    return 0;
}
