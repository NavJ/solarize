#ifndef _SOLARIZE_H__
#define _SOLARIZE_H__

#include <stdbool.h>

void to_grayscale(const unsigned char *in, unsigned char *out, int n);

void solarize_channel(unsigned char *data,
                      int width,
                      int height,
                      int nchan,
                      int chan,
                      int lin_threshold,
                      int smooth_window);

#endif
