#include "solarize.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h>
#include <assert.h>

#define ERROR_LOAD -1
#define ERROR_SAVE -2
#define ERROR_PROCESS -3
#define ERROR_ARGS -4

static void draw_curve(unsigned char *curve) {
  int i, threshold;
  for (i = 0; i < 255; i++) {
    for (threshold = 1; threshold <= 255; threshold *= 2) {
      if (curve[i] > threshold) {
        putc('x', stdout);
      } else {
        putc(' ', stdout);
      }
    }
    putc('\n', stdout);
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
    if (histogram[val] > histmax) {
      histmax = histogram[val];
    }
  }

  // normalize the histogram to a function
  for (i = 0; i < 255; i++) {
    assert((255 * histogram[i]) > histogram[i]);
    curve[i] = (255 * histogram[i]) / histmax;
  }

  // modify the data for the channel
  for (i = 0; i < width * height; i++) {
    unsigned char val = data[(i * nchan) + chan];
    data[(i * nchan) + chan] = curve[val];
  }
}

int process_image(const char *infile) {
  int rc = 0;

  // load the file
  int width, height, comp;
  unsigned char *data = stbi_load(infile, &width, &height, &comp, 0);
  if (data == NULL) {
    return ERROR_LOAD;
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
  if (argc != 2) {
    printf("Usage: %s [input]\n", argv[0]);
    return ERROR_ARGS;
  }
  return process_image(argv[1]);
}
