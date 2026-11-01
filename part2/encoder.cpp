#include "encoder.h"
#include <cstdio>     // perror
#include <fcntl.h>    // open
#include <unistd.h>   // read, close


// Open the device the kernel module made
void Encoder::init()
{
    encoderFile = open("/dev/encoder", O_RDONLY);
    if (encoderFile < 0)
        perror("Open /dev/encoder failed (is the module loaded and the node made?)");
}


// Ask the kernel module for the count. It comes as a binary int.
int Encoder::getCount()
{
    int encoderCount = 0;
    read(encoderFile, &encoderCount, sizeof(encoderCount));
    return encoderCount;
}


void Encoder::close()
{
    ::close(encoderFile);
}
