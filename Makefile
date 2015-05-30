# the compiler: gcc for C program, define as g++ for C++
CC = gcc

# compiler flags:
CFLAGS  = -g -Wall -Wextra -pthread
LINKFLAGS = -lm

# the build target executable:
TARGET = solarize
RESOURCES = resource

all: resources $(TARGET)

resources: $(RESOURCES).rc
	windres $(RESOURCES).rc $(RESOURCES).o

$(TARGET): driver_win_gui.c solarize.h solarize.c resource.h
	$(CC) $(CFLAGS) -o $(TARGET) driver_win_gui.c solarize.c $(RESOURCES).o $(LINKFLAGS)

clean:
	del $(TARGET).exe
