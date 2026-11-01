// Prints the encoder count from the kernel module 10 times a second.

#include <stdio.h>    // printf, perror
#include <fcntl.h>    // open
#include <unistd.h>   // read, usleep

int main(void)
{
    // Open the device the kernel module made
    int encoderFile = open("/dev/encoder", O_RDONLY);
    if (encoderFile < 0) {
        perror("Open /dev/encoder failed (is the module loaded and the node made?)");
        return 1;
    }

    while (1) {

        // Read the count. The module gives it as a binary int.
        int encoderCount = 0;
        read(encoderFile, &encoderCount, sizeof(encoderCount));

        printf("count: %d\n", encoderCount);

        usleep(100000);   // 0.1 s
    }

    return 0;
}
