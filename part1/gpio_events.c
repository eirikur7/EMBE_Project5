// Part 1, test 3: copy the input pin to the output pin using gpiod events.

#include <gpiod.h>    // GPIO library (libgpiod v2)
#include <stdio.h>    // printf, perror

#define INPUT_PIN  25   // GPIO25, the encoder signal comes in here
#define OUTPUT_PIN 17   // GPIO17, the copy goes out here

int main(void)
{
    // Open the GPIO chip (the part of the Pi that controls the pins)
    struct gpiod_chip *chip = gpiod_chip_open("/dev/gpiochip0");
    if (!chip) {
        perror("Open chip failed");
        return 1;
    }


    // Settings for the input pin: it is an input,
    // and we want an event on both edges (low->high and high->low)
    struct gpiod_line_settings *inputSettings = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(inputSettings, GPIOD_LINE_DIRECTION_INPUT);
    gpiod_line_settings_set_edge_detection(inputSettings, GPIOD_LINE_EDGE_BOTH);

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


    // A box that holds one event at a time
    struct gpiod_edge_event_buffer *eventBox = gpiod_edge_event_buffer_new(1);


    printf("Copying GPIO%d to GPIO%d on every edge. Ctrl+C to stop.\n", INPUT_PIN, OUTPUT_PIN);

    while (1) {

        // Sleep here until the input pin changes (-1 = wait forever)
        gpiod_line_request_wait_edge_events(pins, -1);

        // Take the event out of the box
        gpiod_line_request_read_edge_events(pins, eventBox, 1);
        struct gpiod_edge_event *event = gpiod_edge_event_buffer_get_event(eventBox, 0);

        // Rising edge = input went high, so set the output high.
        // Falling edge = input went low, so set the output low.
        if (gpiod_edge_event_get_event_type(event) == GPIOD_EDGE_EVENT_RISING_EDGE)
            gpiod_line_request_set_value(pins, OUTPUT_PIN, GPIOD_LINE_VALUE_ACTIVE);
        else
            gpiod_line_request_set_value(pins, OUTPUT_PIN, GPIOD_LINE_VALUE_INACTIVE);
    }

    // We never get here. Ctrl+C ends the program and Linux gives the pins back.
    return 0;
}
