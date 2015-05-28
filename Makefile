all: solarize

solarize: driver_win.c solarize.h solarize.c
    cl driver_win.c solarize.c

clean:
    del *.obj *.exe