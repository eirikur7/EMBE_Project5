#!/bin/bash
# Part 1, test 1: copy the input pin to the output pin from a shell script.
INPUT_PIN=25    # GPIO25, the encoder signal comes in here
OUTPUT_PIN=17   # GPIO17, the copy goes out here

while true; do

    # Read the input pin. Gives 0 (low) or 1 (high).
    INPUT_VALUE=$(gpioget --numeric -c gpiochip0 $INPUT_PIN)

    # Set the output pin to the same value
    gpioset -t0 -c gpiochip0 $OUTPUT_PIN=$INPUT_VALUE

done
