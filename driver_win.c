#include "solarize.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h>

// These values seem to affect stuff the most...
#define DEFAULT_SMOOTH_WINDOW 30
#define DEFAULT_LIN_THRESHOLD 0

#define ERROR_LOAD -1
#define ERROR_SAVE -2
#define ERROR_PROCESS -3
#define ERROR_ARGS -4

static bool use_grayscale = false;
static bool invert = true;
static int lin_threshold = DEFAULT_LIN_THRESHOLD;
static int smooth_window = DEFAULT_SMOOTH_WINDOW;

void *process_image(void *infile_void) {
  char *infile = (char *) infile_void;
  long rc = 0;

  // load the file (not thread safe!)
  int width, height, comp;
  unsigned char *data = stbi_load(infile, &width, &height, &comp, 0);
  if (data == NULL) {
    fprintf(stderr, "Failed to read image %s!\n", infile);
    return NULL;
  }

  // convert to grayscale if required
  if (use_grayscale && comp == 3) {
    printf("%s", "Converting to grayscale... ");
    unsigned char *grayscale_data = (unsigned char *) malloc(width * height);
    to_grayscale(data, width * height, grayscale_data);
    stbi_image_free(data);
    data = grayscale_data;
    comp = 1;
    printf("%s", "Done.\n");
  } else if (use_grayscale && comp != 3) {
    printf("Ignoring grayscale conversion: image contains %d channels, not 3.", comp);
  }

  // modify image
  int i;
  for (i = 0; i < comp; i++) {
    size_t histogram[2 << BIT_DEPTH];
    build_histogram(data, width * height, comp, i, histogram);
    size_t smoothed_histogram[2 << BIT_DEPTH];
    smooth_histogram(histogram, smooth_window, smoothed_histogram);
    solarize_channel(smoothed_histogram, data, width * height, comp, i,
                     lin_threshold, invert);
  }

  // write the output file
  size_t inlen = strlen(infile);
  char *outfile = (char *) malloc(inlen + 9);
  memset(outfile, 0, inlen + 9);
  strncpy(outfile, infile, inlen);
  strcpy(&outfile[inlen], "_sol.png");
  // RETURNS 0 ON FAILURE WTF???
  if (stbi_write_png(outfile, width, height, comp, data, width * comp) == 0) {
    rc = ERROR_SAVE;
  }

  // free memory
  free(outfile);
  stbi_image_free(data);

  if (rc == 0) {
    fprintf(stdout, "Image %s processed successfully!\n", infile);
  } else {
    fprintf(stderr, "Failed to process image %s with error code: %ld.\n", infile, rc);
  }
  return (void *) rc;
}

static void print_usage(const char *name) {
  fprintf(stderr, "Usage: %s [-v] [-g] [-i] [-t THRESHOLD] [-w WINDOW] FILE...\n", name);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    return -1;
  }

  return (int) process_image((void *) argv[1]);
}
