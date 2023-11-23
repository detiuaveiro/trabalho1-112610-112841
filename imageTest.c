// imageTest - A program that performs some image processing.
//
// This program is an example use of the image8bit module,
// a programming project for the course AED, DETI / UA.PT
//
// You may freely use and modify this code, NO WARRANTY, blah blah,
// as long as you give proper credit to the original and subsequent authors.
//
// João Manuel Rodrigues <jmr@ua.pt>
// 2023

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "image8bit.h"
#include "instrumentation.h"

const char* IMAGES[] = {
    "./test/dot.pgm",
    "./test/square.pgm",
    "./test/white.pgm",
    "./test/white_sm.pgm",
    "./test/subimg.pgm",
    "./test/subimg_sm.pgm",
};

void test_locate_subimage() {
  printf("# LOAD image\n");
  InstrReset();  // to reset instrumentation
  Image dot = ImageLoad(IMAGES[0]);
  Image square = ImageLoad(IMAGES[1]);
  Image white = ImageLoad(IMAGES[2]);
  Image white_sm = ImageLoad(IMAGES[3]);
  Image subimg = ImageLoad(IMAGES[4]);
  Image subimg_sm = ImageLoad(IMAGES[5]);

  if (dot == NULL) {
    error(2, errno, "Loading %s: %s", IMAGES[0], ImageErrMsg());
  }
  if (square == NULL) {
    error(2, errno, "Loading %s: %s", IMAGES[1], ImageErrMsg());
  }
  if (white == NULL) {
    error(2, errno, "Loading %s: %s", IMAGES[2], ImageErrMsg());
  }
  if (white_sm == NULL) {
    error(2, errno, "Loading %s: %s", IMAGES[2], ImageErrMsg());
  }
  if (subimg == NULL) {
    error(2, errno, "Loading %s: %s", IMAGES[3], ImageErrMsg());
  }
  if (subimg_sm == NULL) {
    error(2, errno, "Loading %s: %s", IMAGES[3], ImageErrMsg());
  }
  InstrPrint();  // to print instrumentation

  int px, py;

  // PIXCMP should be 2
  printf("# LOCATE image best case #1 (200x200) (1x1)\n");
  InstrReset();  // to reset instrumentation
  ImageLocateSubImage(square, &px, &py, dot);
  InstrPrint();  // to print instrumentation

  // PIXCMP should be 2
  printf("# LOCATE image best case #2 (200x200) (200x200)\n");
  InstrReset();  // to reset instrumentation
  ImageLocateSubImage(white, &px, &py, square);
  InstrPrint();  // to print instrumentation

  // PIXCMP should be ... bad
  printf("# LOCATE image worst case #1 (100x100) (50x50)\n");
  InstrReset();  // to reset instrumentation
  ImageLocateSubImage(white_sm, &px, &py, subimg_sm);
  InstrPrint();  // to print instrumentation

  // PIXCMP should be ... bad
  printf("# LOCATE image worst case #1 (200x200) (100x100)\n");
  InstrReset();  // to reset instrumentation
  ImageLocateSubImage(white, &px, &py, subimg);
  InstrPrint();  // to print instrumentation

  // PIXCMP should be ... not too bad but still bad
  printf("# LOCATE image worst case #2 (200x200) (200x200)\n");
  InstrReset();  // to reset instrumentation
  ImageLocateSubImage(white, &px, &py, white);
  InstrPrint();  // to print instrumentation

  // PIXCMP should be ... not too bad but still bad
  printf("# LOCATE image worst case #2 (100x100) (100x100)\n");
  InstrReset();  // to reset instrumentation
  ImageLocateSubImage(white_sm, &px, &py, white_sm);
  InstrPrint();  // to print instrumentation

  // Cleanup
  ImageDestroy(&dot);
  ImageDestroy(&square);
  ImageDestroy(&white);
  ImageDestroy(&white_sm);
  ImageDestroy(&subimg);
  ImageDestroy(&subimg_sm);
}

int main(int argc, char* argv[]) {
  program_name = argv[0];

  ImageInit();

  test_locate_subimage();

  return 0;
}
