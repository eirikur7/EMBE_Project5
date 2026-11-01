all: motor_control read_encoder

motor_control: main.cpp encoder.cpp motor.cpp controller.cpp PI_controller.cpp
	g++ -Wall -o motor_control main.cpp encoder.cpp motor.cpp controller.cpp PI_controller.cpp

read_encoder: read_encoder.c
	gcc -Wall -o read_encoder read_encoder.c

clean:
	rm -f motor_control read_encoder
