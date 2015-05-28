#ifndef _SOLARIZE_H__
#define _SOLARIZE_H__

#include <stdbool.h>
#include <stddef.h>

#define BIT_DEPTH 8
#define NCOLORS 256

void to_grayscale(const unsigned char *in, int n, unsigned char *out);

// out (and all histograms) must be of length 2^BIT_DEPTH
void build_histogram(const unsigned char *data,
                     int n,
                     int nchan,
                     int chan,
                     size_t histogram[NCOLORS]);

void smooth_histogram(const size_t histogram[NCOLORS],
                      int smooth_window,
                      size_t out[NCOLORS]);

void solarize_channel(const size_t histogram[NCOLORS],
                      unsigned char *data,
                      int n,
                      int nchan,
                      int chan,
                      int lin_threshold,
                      bool post_invert);

#endif
