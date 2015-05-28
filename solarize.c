#include "solarize.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

static void draw_curve(unsigned char *curve) {
  int i, threshold;
  putc('\n', stdout);
  for (threshold = NCOLORS - 1; threshold > 0; threshold /= 2) {
    for (i = 0; i < NCOLORS; i += 5) {
      if (curve[i] > threshold) {
        putc('x', stdout);
      } else {
        putc(' ', stdout);
      }
    }
    putc('\n', stdout);
  }
  for (i = 0; i < NCOLORS; i += 5) {
    putc('-', stdout);
  }
  putc('\n', stdout);
}

void smooth_histogram(const size_t *histogram, int smooth_window, size_t *out) {
  memset(out, 0, NCOLORS * sizeof(size_t));

  // use centered weighted average
  int n = smooth_window;
  int i, j;
  for (i = 0; i < NCOLORS; i++) {
    // use a big int to prevent overflows
    uint64_t val = 0;
    for (j = -n; j <= n; j++) {
      if ((i + j) >= 0 && (i + j) < NCOLORS) {
        // weighted so middle element i has weight (n * i)
        uint64_t to_add = histogram[i + j];
        to_add *= (n - abs(j));
        val += to_add;
      }
    }
    // we must divide our result above by:
    // 2 * (n + (n - 1) + ... + 2 + 1) - n [we only have 1 middle elem] = n^2
    val /= n;
    val /= n;
    out[i] = val;
  }
}

void build_histogram(const unsigned char *data,
                     int n,
                     int nchan,
                     int chan,
                     size_t *histogram) {
  int i;
  memset(histogram, 0, NCOLORS * sizeof(size_t));
  for (i = 0; i < n; i++) {
    unsigned char val = data[(i * nchan) + chan];
    histogram[val]++;
  }
}

void solarize_channel(const size_t *histogram,
                      unsigned char *data,
                      int n,
                      int nchan,
                      int chan,
                      int lin_threshold,
                      bool post_invert) {
  unsigned char curve[NCOLORS];
  size_t histmax;
  int i;

  // find the max in the smoothed histogram
  for (i = 0; i < NCOLORS; i++) {
    if (histogram[i] > histmax) {
      histmax = histogram[i];
    }
  }

  // normalize the histogram to a function
  for (i = 0; i < NCOLORS; i++) {
    assert((NCOLORS * histogram[i]) >= histogram[i]);
    unsigned char val = (NCOLORS * histogram[i]) / histmax;
    if (val < lin_threshold) {
      curve[i] = i;
    } else {
      curve[i] = val;
    }
  }

  // modify the data for the channel
  for (i = 0; i < n; i++) {
    unsigned char val = data[(i * nchan) + chan];
    if (post_invert) {
      data[(i * nchan) + chan] = NCOLORS - curve[val];
    } else {
      data[(i * nchan) + chan] = curve[val];
    }
  }
}

// gamma expands a color in the sRGB color space
// see: http://en.wikipedia.org/wiki/Grayscale
static float gamma_expand(float p) {
  if (p <= 0.04045) {
    return p / 12.92;
  } else {
    return pow((p + 0.055) / 1.055, 2.4);
  }
}

static float gamma_compress(float p) {
  if (p <= 0.0031308) {
    return 12.92 * p;
  } else {
    return (1.055 * pow(p, (1.0 / 2.4))) - 0.055;
  }
}

static unsigned char float_to_b(float p) {
  return (unsigned char) (p * ((float) NCOLORS));
}

static float b_to_float(unsigned char p) {
  return ((float) p) / ((float) NCOLORS);
}

// convert to grayscale preserving luminance
static unsigned char px_to_grayscale(unsigned char r, unsigned char g, unsigned char b) {
  float rlin = gamma_expand(b_to_float(r));
  float glin = gamma_expand(b_to_float(g));
  float blin = gamma_expand(b_to_float(b));
  float ylin = (0.2126 * rlin) + (0.7152 * glin) + (0.0722 * blin);
  // TODO: Try without this step to see whether gamma compression is needed
  //       in the single-channel PNG files produced by stb_image_write.
  float ysrgb = gamma_compress(ylin);
  return float_to_b(ysrgb);
}


// requires exactly 3 channels in input
void to_grayscale(const unsigned char *in, int n, unsigned char *out) {
  int i;
  for (i = 0; i < n; i++) {
    const unsigned char *px = &in[i * 3];
    out[i] = px_to_grayscale(px[0], px[1], px[2]);
  }
}
