// imageGen - A program that performs some image processing.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "image8bit.h"

void generate_dot(char* file_path) {
  Image img = ImageCreate(1, 1, 255);

  ImageSetPixel(img, 0, 0, 0);

  ImageSave(img, file_path);
  ImageDestroy(&img);
}

void generate_square(char* file_path) {
  int w = 200, h = 200;
  Image img = ImageCreate(w, h, 255);

  for (int x = 0; x < w; ++x) {
    for (int y = 0; y < h; ++y) {
      if (x == y) {
        ImageSetPixel(img, x, y, 0);
      } else {
        ImageSetPixel(img, x, y, ImageMaxval(img));
      }
    }
  }

  ImageSave(img, file_path);
  ImageDestroy(&img);
}

void generate_white(char* file_path) {
  int w = 200, h = 200;
  Image img = ImageCreate(w, h, 255);

  for (int x = 0; x < w; ++x) {
    for (int y = 0; y < h; ++y) {
      ImageSetPixel(img, x, y, ImageMaxval(img));
    }
  }

  ImageSave(img, file_path);
  ImageDestroy(&img);
}

void generate_subimg(char* file_path) {
  int w = 100, h = 100;
  Image img = ImageCreate(w, h, 255);

  for (int x = 0; x < w; ++x) {
    for (int y = 0; y < h; ++y) {
      ImageSetPixel(img, x, y, ImageMaxval(img));
    }
  }

  ImageSetPixel(img, 99, 99, 0);

  ImageSave(img, file_path);
  ImageDestroy(&img);
}

void generate_white_sm(char* file_path) {
  int w = 100, h = 100;
  Image img = ImageCreate(w, h, 255);

  for (int x = 0; x < w; ++x) {
    for (int y = 0; y < h; ++y) {
      ImageSetPixel(img, x, y, ImageMaxval(img));
    }
  }

  ImageSave(img, file_path);
  ImageDestroy(&img);
}

void generate_subimg_sm(char* file_path) {
  int w = 50, h = 50;
  Image img = ImageCreate(w, h, 255);

  for (int x = 0; x < w; ++x) {
    for (int y = 0; y < h; ++y) {
      ImageSetPixel(img, x, y, ImageMaxval(img));
    }
  }

  ImageSetPixel(img, 49, 49, 0);

  ImageSave(img, file_path);
  ImageDestroy(&img);
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "ERROR: Missing arguments!\n");
    fprintf(stderr, "Usage: %s [dot/square/white/subimg/all]\n", argv[0]);
    exit(1);
  }

  if (strcmp(argv[1], "dot") == 0) {
    generate_dot("./test/dot.pgm");
  } else if (strcmp(argv[1], "square") == 0) {
    generate_square("./test/square.pgm");
  } else if (strcmp(argv[1], "white") == 0) {
    generate_white("./test/white.pgm");
    generate_white_sm("./test/white_sm.pgm");
  } else if (strcmp(argv[1], "subimg") == 0) {
    generate_subimg("./test/subimg.pgm");
    generate_subimg_sm("./test/subimg_sm.pgm");
  } else if (strcmp(argv[1], "all") == 0) {
    generate_dot("./test/dot.pgm");
    generate_square("./test/square.pgm");
    generate_white("./test/white.pgm");
    generate_white_sm("./test/white_sm.pgm");
    generate_subimg("./test/subimg.pgm");
    generate_subimg_sm("./test/subimg_sm.pgm");
  } else {
    fprintf(stderr, "ERROR: Invalid argument!\n");
    fprintf(stderr, "Usage: %s [dot/square/white/subimg/all]\n", argv[0]);
    exit(1);
  }

  printf("Image(s) generated successfully!\n");

  return 0;
}