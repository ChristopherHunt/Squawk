# Make file for Squawk driver executable

CC = g++
CFLAGS = -Wall
SQUAWK_OBJS = Squawk.o LSM303.o LCD.o GPIO.o 

Squawk: $(SQUAWK_OBJS)
	$(CC) $(CFLAGS) $(SQUAWK_OBJS) -o Squawk 

LSM303.o: Libraries/LSM303/LSM303.cpp
	$(CC) $(CFLAGS) Libraries/LSM303/LSM303.cpp -c

LCD.o: Libraries/LCD/LCD.cpp
	$(CC) $(CFLAGS) Libraries/LCD/LCD.cpp -c

GPIO.o: Libraries/GPIO/GPIO.cpp
	$(CC) $(CFLAGS) Libraries/GPIO/GPIO.cpp -c

%.c: %.h
	touch $@

clean:
	rm *.o Squawk
