#include "solarize.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define ERROR_LOAD -1
#define ERROR_SAVE -2
#define ERROR_PROCESS -3
#define ERROR_ARGS -4

int process_image(const char *infile) {
  int rc = 0;

  // load the file
  int width, height, comp;
  unsigned char *data = stbi_load(infile, &width, &height, &comp, 0);
  if (data == NULL) {
    return ERROR_LOAD;
  }

  // TODO: Do things with image

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
