#include "solarize.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

// These values seem to affect stuff the most...
#define DEFAULT_SMOOTH_WINDOW 30
#define DEFAULT_LIN_THRESHOLD 0

#define ERROR_LOAD -1
#define ERROR_SAVE -2
#define ERROR_PROCESS -3
#define ERROR_ARGS -4

static pthread_mutex_t img_read_lock = PTHREAD_MUTEX_INITIALIZER;
static bool use_grayscale = false;
static bool invert = true;
static int lin_threshold = DEFAULT_LIN_THRESHOLD;
static int smooth_window = DEFAULT_SMOOTH_WINDOW;

void *process_image(void *infile_void) {
  char *infile = (char *) infile_void;
  long rc = 0;

  // load the file (not thread safe!)
  int width, height, comp;
  pthread_mutex_lock(&img_read_lock);
  unsigned char *data = stbi_load(infile, &width, &height, &comp, 0);
  pthread_mutex_unlock(&img_read_lock);
  if (data == NULL) {
    fprintf(stderr, "Failed to read image %s!\n", infile);
    return NULL;
  }

  // convert to grayscale if required
  if (use_grayscale && comp == 3) {
    printf("%s", "Converting to grayscale... ");
    unsigned char *grayscale_data = malloc(width * height);
    to_grayscale(data, grayscale_data, width * height);
    stbi_image_free(data);
    data = grayscale_data;
    comp = 1;
    printf("%s", "Done.\n");
  } else if (use_grayscale && comp != 3) {
    printf("Ignoring grayscale conversion: image contains %d channels, not 3.", comp);
  }

  // modify image
  if (comp < 3) {
    // grayscale
    solarize_channel(data, width, height, comp, 0,
                     lin_threshold, smooth_window, invert);
  } else {
    // color
    int rgb;
    for (rgb = 0; rgb < 3; rgb++) {
      solarize_channel(data, width, height, comp, rgb,
                       lin_threshold, smooth_window, invert);
    }
  }

  // write the output file
  size_t inlen = strlen(infile);
  char outfile[inlen + 9];
  memset(outfile, 0, inlen + 9);
  strncpy(outfile, infile, inlen);
  strcpy(&outfile[inlen], "_sol.png");
  // RETURNS 0 ON FAILURE WTF???
  // also is this thread safe??? Maybe not?
  //pthread_mutex_lock(&img_read_lock);
  if (stbi_write_png(outfile, width, height, comp, data, width * comp) == 0) {
    rc = ERROR_SAVE;
  }
  //pthread_mutex_unlock(&img_read_lock);

  // free memory
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
  int opt;

  while ((opt = getopt(argc, argv, "ivgt:w:")) != -1) {
    switch (opt) {
    case 'g':
      use_grayscale = true;
      break;
    case 't':
      lin_threshold = atoi(optarg);
      break;
    case 'w':
      smooth_window = atoi(optarg);
      break;
    case 'v':
      break;
    case 'i':
      invert = false;
      break;
    default:
      print_usage(argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  if (optind >= argc) {
    print_usage(argv[0]);
    exit(EXIT_FAILURE);
  }

  // simple multithreading.
  // TODO: pool threads rather than one per file
  int nfiles = argc - optind;
  pthread_t threads[nfiles];
  int rc;
  int i;
  for (i = 0; i < nfiles; i++) {
    rc = pthread_create(&threads[i], NULL, process_image, (void *) argv[i + optind]);
    if (rc != 0) {
      fprintf(stderr, "Failed to create thread for image %s!\n", argv[i + optind]);
    }
  }
  pthread_exit(NULL);
  return 0;
}
