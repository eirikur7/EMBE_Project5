// Measures how fast a copy program copies the input pin to the output pin.

#include <gpiod.h>    // GPIO library (libgpiod v2)
#include <stdio.h>    // printf, fopen
#include <stdlib.h>   // atoi
#include <time.h>     // clock_gettime
#include <unistd.h>   // usleep

#define TEST_SIGNAL_PIN 27   // GPIO27, header pin 13. Wire it to pin 22 (GPIO25)
#define RETURN_PIN      22   // GPIO22, header pin 15. Wire it to pin 11 (GPIO17)


// The time right now in nanoseconds. Same clock as the gpiod event timestamps.
static long long timeNowNanoseconds(void)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (long long)now.tv_sec * 1000000000LL + now.tv_nsec;
}


int main(int argc, char **argv)
{
    int numberOfTests = 1000;
    if (argc > 1)
        numberOfTests = atoi(argv[1]);

    const char *resultFileName = "delays.txt";
    if (argc > 2)
        resultFileName = argv[2];


    // Open the GPIO chip (the part of the Pi that controls the pins)
    struct gpiod_chip *chip = gpiod_chip_open("/dev/gpiochip0");
    if (!chip) {
        perror("Open chip failed");
        return 1;
    }


    // Settings for the test signal pin: it is an output, and starts low
    struct gpiod_line_settings *testSignalSettings = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(testSignalSettings, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_output_value(testSignalSettings, GPIOD_LINE_VALUE_INACTIVE);

    // Settings for the return pin: it is an input, with an event on both edges
    struct gpiod_line_settings *returnSettings = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(returnSettings, GPIOD_LINE_DIRECTION_INPUT);
    gpiod_line_settings_set_edge_detection(returnSettings, GPIOD_LINE_EDGE_BOTH);


    // Make a list of which pin gets which settings
    unsigned int testSignalPinNumber = TEST_SIGNAL_PIN;
    unsigned int returnPinNumber = RETURN_PIN;

    struct gpiod_line_config *pinList = gpiod_line_config_new();
    gpiod_line_config_add_line_settings(pinList, &testSignalPinNumber, 1, testSignalSettings);
    gpiod_line_config_add_line_settings(pinList, &returnPinNumber, 1, returnSettings);


    // Ask Linux to give us the pins
    struct gpiod_line_request *pins = gpiod_chip_request_lines(chip, NULL, pinList);
    if (!pins) {
        perror("Request pins failed");
        return 1;
    }


    // A box that holds the events
    struct gpiod_edge_event_buffer *eventBox = gpiod_edge_event_buffer_new(16);

    FILE *resultFile = fopen(resultFileName, "w");
    if (!resultFile) {
        perror("Open result file failed");
        return 1;
    }


    int testSignalLevel = 0;
    int missedEdges = 0;
    int measuredEdges = 0;
    double shortestDelay = 1e9;
    double longestDelay = 0;
    double sumOfDelays = 0;

    for (int test = 0; test < numberOfTests; test++) {

        // Throw away old events, so we only see the answer to this edge
        while (gpiod_line_request_wait_edge_events(pins, 0) > 0)
            gpiod_line_request_read_edge_events(pins, eventBox, 16);


        // Flip the test pin, and remember when we did it
        testSignalLevel = !testSignalLevel;
        long long startTime = timeNowNanoseconds();

        if (testSignalLevel)
            gpiod_line_request_set_value(pins, TEST_SIGNAL_PIN, GPIOD_LINE_VALUE_ACTIVE);
        else
            gpiod_line_request_set_value(pins, TEST_SIGNAL_PIN, GPIOD_LINE_VALUE_INACTIVE);


        // Wait up to 1 second for the copy to come back
        int gotEdge = gpiod_line_request_wait_edge_events(pins, 1000000000LL);

        if (gotEdge <= 0) {
            missedEdges++;          // it never came back
        } else {
            gpiod_line_request_read_edge_events(pins, eventBox, 16);
            struct gpiod_edge_event *event = gpiod_edge_event_buffer_get_event(eventBox, 0);

            // Delay = when the edge came back - when we sent it
            long long returnTime = (long long)gpiod_edge_event_get_timestamp_ns(event);
            double delayMicroseconds = (returnTime - startTime) / 1000.0;

            fprintf(resultFile, "%.1f\n", delayMicroseconds);

            if (delayMicroseconds < shortestDelay) shortestDelay = delayMicroseconds;
            if (delayMicroseconds > longestDelay)  longestDelay = delayMicroseconds;
            sumOfDelays += delayMicroseconds;
            measuredEdges++;
        }


        // Short pause before the next edge
        usleep(10000);
    }

    fclose(resultFile);


    if (measuredEdges > 0)
        printf("shortest %.1f us   average %.1f us   longest %.1f us   missed %d of %d\n",
               shortestDelay, sumOfDelays / measuredEdges, longestDelay, missedEdges, numberOfTests);
    else
        printf("no edges came back, missed %d of %d. Is the copy program running and wired?\n",
               missedEdges, numberOfTests);

    return 0;
}
