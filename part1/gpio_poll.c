// Part 1, test 2: copy the input pin to the output pin by polling.

#include <gpiod.h>    // GPIO library (libgpiod v2)
#include <stdio.h>    // printf, perror
#include <stdlib.h>   // atoi
#include <unistd.h>   // usleep

#define INPUT_PIN  25   // GPIO25, the encoder signal comes in here
#define OUTPUT_PIN 17   // GPIO17, the copy goes out here

int main(int argc, char **argv)
{
    // How long to sleep between each look, in microseconds
    int sleepTimeMicroseconds = 100;
    if (argc > 1)
        sleepTimeMicroseconds = atoi(argv[1]);


    // Open the GPIO chip (the part of the Pi that controls the pins)
    struct gpiod_chip *chip = gpiod_chip_open("/dev/gpiochip0");
    if (!chip) {
        perror("Open chip failed");
        return 1;
    }


    // Settings for the input pin: it is an input
    struct gpiod_line_settings *inputSettings = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(inputSettings, GPIOD_LINE_DIRECTION_INPUT);

    // Settings for the output pin: it is an output, and starts low
    struct gpiod_line_settings *outputSettings = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(outputSettings, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_output_value(outputSettings, GPIOD_LINE_VALUE_INACTIVE);


    // Make a list of which pin gets which settings
    unsigned int inputPinNumber = INPUT_PIN;
    unsigned int outputPinNumber = OUTPUT_PIN;

    struct gpiod_line_config *pinList = gpiod_line_config_new();
    gpiod_line_config_add_line_settings(pinList, &inputPinNumber, 1, inputSettings);
    gpiod_line_config_add_line_settings(pinList, &outputPinNumber, 1, outputSettings);


    // Ask Linux to give us the pins
    struct gpiod_line_request *pins = gpiod_chip_request_lines(chip, NULL, pinList);
    if (!pins) {
        perror("Request pins failed");
        return 1;
    }


    printf("Copying GPIO%d to GPIO%d, looking every %d us. Ctrl+C to stop.\n",
           INPUT_PIN, OUTPUT_PIN, sleepTimeMicroseconds);

    while (1) {

        // Read the input pin
        enum gpiod_line_value inputValue = gpiod_line_request_get_value(pins, INPUT_PIN);

        // Copy it to the output pin
        gpiod_line_request_set_value(pins, OUTPUT_PIN, inputValue);

        // Sleep a little, then look again
        usleep(sleepTimeMicroseconds);
    }

    // We never get here. Ctrl+C ends the program and Linux gives the pins back.
    return 0;
}
