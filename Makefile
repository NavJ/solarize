 # the compiler: gcc for C program, define as g++ for C++
  CC = gcc

  # compiler flags:
  CFLAGS  = -g -Wall -Wextra -lm

  # the build target executable:
  TARGET = solarize

  all: $(TARGET)

  $(TARGET): $(TARGET).c
		$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

  clean:
		$(RM) $(TARGET)
