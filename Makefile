 # the compiler: gcc for C program, define as g++ for C++
  CC = gcc

  # compiler flags:
  CFLAGS  = -g -Wall -Wextra -pthread
  LINKFLAGS = -lm

  # the build target executable:
  TARGET = solarize

  all: $(TARGET)

  $(TARGET): driver.c solarize.h solarize.c
		$(CC) $(CFLAGS) -o $(TARGET) driver.c solarize.c $(LINKFLAGS)

  clean:
		$(RM) $(TARGET)
