# the compiler: gcc for C program, define as g++ for C++
CC = gcc

# compiler flags:
CFLAGS  = -g -Wall -Wextra -pthread -Wno-unused-value -Wno-unused-parameter -Wno-unused-variable
LINKFLAGS = -lm -lcomdlg32 -luser32 -lcomctl32

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
