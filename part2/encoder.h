// Encoder class: reads the encoder count from the kernel module (/dev/encoder).
#ifndef ENCODER_H
#define ENCODER_H

class Encoder
{
public:
    // Counts per turn of the output shaft (both edges of C1, 1:100 gearbox), same as Project 4
    static const int COUNTS_PER_REVOLUTION = 1400;

    void init();          // open /dev/encoder
    int  getCount();      // the count right now
    void close();         // close /dev/encoder

private:
    int encoderFile = -1;
};

#endif
