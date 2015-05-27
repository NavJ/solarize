#include "solarize.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

static void draw_curve(unsigned char *curve) {
  int i, threshold;
  putc('\n', stdout);
  for (threshold = 255; threshold > 0; threshold /= 2) {
    for (i = 0; i < 255; i += 5) {
      if (curve[i] > threshold) {
        putc('x', stdout);
      } else {
        putc(' ', stdout);
      }
    }
    putc('\n', stdout);
  }
  for (i = 0; i < 255; i += 5) {
    putc('-', stdout);
  }
  putc('\n', stdout);
}

static void smooth_histogram(size_t *h, int smooth_window) {
  size_t hn[255];

  // use centered weighted average
  int n = smooth_window;
  int i, j;
  for (i = 0; i < 255; i++) {
    hn[i] = 0;
    for (j = -n; j < n; j++) {
      if ((i + j) >= 0 && (i + j) <= 255) {
        // weighted so middle element i has weight (n * i)
        hn[i] += h[i + j] * (n - abs(j));
      }
    }
    // we must divide our result above by:
    // 2 * (n + (n - 1) + ... + 2 + 1) - n [we only have 1 middle elem] = n^2
    hn[i] /= n * n;
  }

  // copy back to original histogram
  for (i = 0; i < 255; i++) {
    h[i] = hn[i];
  }
}

void solarize_channel(unsigned char *data,
                      int width,
                      int height,
                      int nchan,
                      int chan,
                      int lin_threshold,
                      int smooth_window) {
  assert(lin_threshold >= 0 && smooth_window > 0);

  // build histogram
  unsigned char curve[255];
  size_t histogram[255];
  size_t histmax = 0;
  int i;
  memset(histogram, 0, 255);
  for (i = 0; i < width * height; i++) {
    unsigned char val = data[(i * nchan) + chan];
    histogram[val]++;
  }

  // smooth the histogram
  smooth_histogram(histogram, smooth_window);

  // find the max in the smoothed histogram
  for (i = 0; i < 255; i++) {
    if (histogram[i] > histmax) {
      histmax = histogram[i];
    }
  }

  // normalize the histogram to a function
  for (i = 0; i < 255; i++) {
    assert((255 * histogram[i]) > histogram[i]);
    unsigned char val = (255 * histogram[i]) / histmax;
    if (val < lin_threshold) {
      curve[i] = i;
    } else {
      curve[i] = val;
    }
  }

  // modify the data for the channel
  for (i = 0; i < width * height; i++) {
    unsigned char val = data[(i * nchan) + chan];
    data[(i * nchan) + chan] = curve[val];
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
  return (unsigned char) (p * 255.0);
}

static float b_to_float(unsigned char p) {
  return ((float) p) / 255.0;
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
void to_grayscale(const unsigned char *in, unsigned char *out, int n) {
  int i;
  for (i = 0; i < n; i++) {
    const unsigned char *px = &in[i * 3];
    out[i] = px_to_grayscale(px[0], px[1], px[2]);
  }
}
