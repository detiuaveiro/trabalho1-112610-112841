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

const char* IMAGES2[] = {
    "./pgm/large/ireland_03_1600x1200.pgm",
    "./pgm/medium/airfield-05_640x480.pgm",
    "./pgm/small/bird_256x256.pgm",
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

void test_blur() {
  printf("# LOAD image\n");
  InstrReset();  // to reset instrumentation
  Image large = ImageLoad(IMAGES2[0]);
  Image medium = ImageLoad(IMAGES2[1]);
  Image small = ImageLoad(IMAGES2[2]);

  if (large == NULL) {
    error(2, errno, "Loading %s: %s", IMAGES2[0], ImageErrMsg());
  }
  if (medium == NULL) {
    error(2, errno, "Loading %s: %s", IMAGES2[1], ImageErrMsg());
  }
  if (small == NULL) {
    error(2, errno, "Loading %s: %s", IMAGES2[2], ImageErrMsg());
  }
  InstrPrint();  // to print instrumentation

  int dx = 40, dy = 20;
  printf("# ImageBlur (Most optimized) (1600x1200) with blur window (40x20)\n");
  InstrReset();  // to reset instrumentation
  ImageBlur(large, dx, dy);
  InstrPrint();  // to print instrumentation
  printf("# ImageBlur2 (Half optimized) (1600x1200) with blur window (40x20)\n");
  InstrReset();  // to reset instrumentation
  ImageBlur2(large, dx, dy);
  InstrPrint();  // to print instrumentation
  printf("# ImageBlur3 (Not optimized) (1600x1200) with blur window (40x20)\n");
  InstrReset();  // to reset instrumentation
  ImageBlur3(large, dx, dy);
  InstrPrint();  // to print instrumentation

  dx = 40, dy = 40;

  printf("# ImageBlur (Most optimized) (640x480) with blur window (40x40)\n");
  InstrReset();  // to reset instrumentation
  ImageBlur(medium, dx, dy);
  InstrPrint();  // to print instrumentation
  printf("# ImageBlur2 (Half optimized) (640x480) with blur window (40x40)\n");
  InstrReset();  // to reset instrumentation
  ImageBlur2(medium, dx, dy);
  InstrPrint();  // to print instrumentation
  printf("# ImageBlur3 (Not optimized) (640x480) with blur window (40x40)\n");
  InstrReset();  // to reset instrumentation
  ImageBlur3(medium, dx, dy);
  InstrPrint();  // to print instrumentation

  dx = 20, dy = 20;

  printf("# ImageBlur (Most optimized) (640x480) with blur window (20x20)\n");
  InstrReset();  // to reset instrumentation
  ImageBlur(medium, dx, dy);
  InstrPrint();  // to print instrumentation
  printf("# ImageBlur2 (Half optimized) (640x480) with blur window (20x20)\n");
  InstrReset();  // to reset instrumentation
  ImageBlur2(medium, dx, dy);
  InstrPrint();  // to print instrumentation
  printf("# ImageBlur3 (Not optimized) (640x480) with blur window (20x20)\n");
  InstrReset();  // to reset instrumentation
  ImageBlur3(medium, dx, dy);
  InstrPrint();  // to print instrumentation

  dx = 4, dy = 2;

  printf("# ImageBlur (Most optimized) (256x256) with blur window (4x2)\n");
  InstrReset();  // to reset instrumentation
  ImageBlur(small, dx, dy);
  InstrPrint();  // to print instrumentation
  printf("# ImageBlur2 (Half optimized) (256x256) with blur window (4x2)\n");
  InstrReset();  // to reset instrumentation
  ImageBlur2(small, dx, dy);
  InstrPrint();  // to print instrumentation
  printf("# ImageBlur3 (Not optimized) (256x256) with blur window (4x2)\n");
  InstrReset();  // to reset instrumentation
  ImageBlur3(small, dx, dy);
  InstrPrint();  // to print instrumentation

  ImageDestroy(&large);
  ImageDestroy(&medium);
  ImageDestroy(&small);
}

int main(int argc, char* argv[]) {
  program_name = argv[0];

  ImageInit();

  // test_locate_subimage();
  test_blur();

  return 0;
}
