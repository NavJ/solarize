#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#define ERROR_LOAD -1
#define ERROR_SAVE -2
#define ERROR_PROCESS -3
#define ERROR_ARGS -4

// These values seem to affect stuff the most...
#define SMOOTH_WINDOW 30
#define LIN_THRESHOLD 0

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

static void smooth_histogram(size_t *h) {
  size_t hn[255];

  // use centered weighted average
  int n = SMOOTH_WINDOW;
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

static void solarize_channel(unsigned char *data,
                            int width,
                            int height,
                            int nchan,
                            int chan) {
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
  smooth_histogram(histogram);

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
    if (val < LIN_THRESHOLD) {
      curve[i] = i;
    } else {
      curve[i] = val;
    }
  }

  draw_curve(curve);

  // modify the data for the channel
  for (i = 0; i < width * height; i++) {
    unsigned char val = data[(i * nchan) + chan];
    data[(i * nchan) + chan] = curve[val];
  }
}

// gamma expands a color in the sRGB color space
// see: http://en.wikipedia.org/wiki/Grayscale
float gamma_expand(float p) {
  if (p <= 0.04045) {
    return p / 12.92;
  } else {
    return pow((p + 0.055) / 1.055, 2.4);
  }
}

float gamma_compress(float p) {
  if (p <= 0.0031308) {
    return 12.92 * p;
  } else {
    return (1.055 * pow(p, (1.0 / 2.4))) - 0.055;
  }
}

unsigned char float_to_b(float p) {
  return (unsigned char) (p * 255.0);
}

float b_to_float(unsigned char p) {
  return ((float) p) / 255.0;
}

// convert to grayscale preserving luminance
unsigned char to_grayscale(unsigned char r, unsigned char g, unsigned char b) {
  float rlin = gamma_expand(b_to_float(r));
  float glin = gamma_expand(b_to_float(g));
  float blin = gamma_expand(b_to_float(b));
  float ylin = (0.2126 * rlin) + (0.7152 * glin) + (0.0722 * blin);
  // TODO: Try without this step to see whether gamma compression is needed
  //       in the single-channel PNG files produced by stb_image_write.
  float ysrgb = gamma_compress(ylin);
  return float_to_b(ysrgb);
}

int process_image(const char *infile, bool use_grayscale) {
  int rc = 0;

  // load the file
  int width, height, comp;
  unsigned char *data = stbi_load(infile, &width, &height, &comp, 0);
  if (data == NULL) {
    return ERROR_LOAD;
  }

  // convert to grayscale if required
  if (use_grayscale && comp > 2) {
    printf("%s", "Converting to grayscale... ");
    unsigned char *grayscale_data = malloc(width * height);
    int i;
    for (i = 0; i < width * height; i++) {
      unsigned char *px = &data[comp * i];
      grayscale_data[i] = to_grayscale(px[0], px[1], px[2]);
    }
    stbi_image_free(data);
    data = grayscale_data;
    comp = 1;
    printf("%s", "Done.\n");
  }

  // modify image
  if (comp < 3) {
    // grayscale
    solarize_channel(data, width, height, comp, 0);
  } else {
    // color
    int rgb;
    for (rgb = 0; rgb < 3; rgb++) {
      solarize_channel(data, width, height, comp, rgb);
    }
  }

  // write the output file
  size_t inlen = strlen(infile);
  char outfile[inlen + 9];
  strncpy(outfile, infile, inlen);
  strcpy(&outfile[inlen], "_sol.png");
  if (stbi_write_png(outfile, width, height, comp, data, width * comp) != 0) {
    rc = ERROR_SAVE;
  }

  // free memory
  stbi_image_free(data);
  return rc;
}

int main(int argc, const char *argv[]) {
  if (argc != 2 && argc != 3) {
    printf("Usage: %s <input> [--grayscale]\n", argv[0]);
    return ERROR_ARGS;
  }
  if (argc == 3 && (strcmp(argv[1], "--grayscale") == 0 ||
                    strcmp(argv[2], "--grayscale") == 0)) {
    return process_image(argv[1], true);
  } else {
    return process_image(argv[1], false);
  }
}
