/// image8bit - A simple image processing module.
///
/// This module is part of a programming project
/// for the course AED, DETI / UA.PT
///
/// You may freely use and modify this code, at your own risk,
/// as long as you give proper credit to the original and subsequent authors.
///
/// João Manuel Rodrigues <jmr@ua.pt>
/// 2013, 2023

// Student authors (fill in below):
// NMec: 112610 Name: David Amorim
// NMec: 112841 Name: Francisca Silva
//
//
// Date:
//

#include "image8bit.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "instrumentation.h"

// The data structure
//
// An image is stored in a structure containing 3 fields:
// Two integers store the image width and height.
// The other field is a pointer to an array that stores the 8-bit gray
// level of each pixel in the image.  The pixel array is one-dimensional
// and corresponds to a "raster scan" of the image from left to right,
// top to bottom.
// For example, in a 100-pixel wide image (img->width == 100),
//   pixel position (x,y) = (33,0) is stored in img->pixel[33];
//   pixel position (x,y) = (22,1) is stored in img->pixel[122].
//
// Clients should use images only through variables of type Image,
// which are pointers to the image structure, and should not access the
// structure fields directly.

// Maximum value you can store in a pixel (maximum maxval accepted)
const uint8 PixMax = 255;

// Internal structure for storing 8-bit graymap images
struct image {
  int width;
  int height;
  int maxval;    // maximum gray value (pixels with maxval are pure WHITE)
  uint8* pixel;  // pixel data (a raster scan)
};

// This module follows "design-by-contract" principles.
// Read `Design-by-Contract.md` for more details.

/// Error handling functions

// In this module, only functions dealing with memory allocation or file
// (I/O) operations use defensive techniques.
//
// When one of these functions fails, it signals this by returning an error
// value such as NULL or 0 (see function documentation), and sets an internal
// variable (errCause) to a string indicating the failure cause.
// The errno global variable thoroughly used in the standard library is
// carefully preserved and propagated, and clients can use it together with
// the ImageErrMsg() function to produce informative error messages.
// The use of the GNU standard library error() function is recommended for
// this purpose.
//
// Additional information:  man 3 errno;  man 3 error;

// Variable to preserve errno temporarily
static int errsave = 0;

// Error cause
static char* errCause;

/// Error cause.
/// After some other module function fails (and returns an error code),
/// calling this function retrieves an appropriate message describing the
/// failure cause.  This may be used together with global variable errno
/// to produce informative error messages (using error(), for instance).
///
/// After a successful operation, the result is not garanteed (it might be
/// the previous error cause).  It is not meant to be used in that situation!
char* ImageErrMsg() {  ///
  return errCause;
}

// Defensive programming aids
//
// Proper defensive programming in C, which lacks an exception mechanism,
// generally leads to possibly long chains of function calls, error checking,
// cleanup code, and return statements:
//   if ( funA(x) == errorA ) { return errorX; }
//   if ( funB(x) == errorB ) { cleanupForA(); return errorY; }
//   if ( funC(x) == errorC ) { cleanupForB(); cleanupForA(); return errorZ; }
//
// Understanding such chains is difficult, and writing them is boring, messy
// and error-prone.  Programmers tend to overlook the intricate details,
// and end up producing unsafe and sometimes incorrect programs.
//
// In this module, we try to deal with these chains using a somewhat
// unorthodox technique.  It resorts to a very simple internal function
// (check) that is used to wrap the function calls and error tests, and chain
// them into a long Boolean expression that reflects the success of the entire
// operation:
//   success =
//   check( funA(x) != error , "MsgFailA" ) &&
//   check( funB(x) != error , "MsgFailB" ) &&
//   check( funC(x) != error , "MsgFailC" ) ;
//   if (!success) {
//     conditionalCleanupCode();
//   }
//   return success;
//
// When a function fails, the chain is interrupted, thanks to the
// short-circuit && operator, and execution jumps to the cleanup code.
// Meanwhile, check() set errCause to an appropriate message.
//
// This technique has some legibility issues and is not always applicable,
// but it is quite concise, and concentrates cleanup code in a single place.
//
// See example utilization in ImageLoad and ImageSave.
//
// (You are not required to use this in your code!)

// Check a condition and set errCause to failmsg in case of failure.
// This may be used to chain a sequence of operations and verify its success.
// Propagates the condition.
// Preserves global errno!
static int check(int condition, const char* failmsg) {
  errCause = (char*)(condition ? "" : failmsg);
  return condition;
}

/**
 * Como não sabemos se podemos usar funções da libm, devido ao Makefile dado não linkar com
 * essa biblioteca, implementámos as funções "min", "max" e "round" da libm
 */

/// Returns the minimum between the integers a and b
static inline int min(int a, int b) {
  if (a < b) {
    return a;
  }

  return b;
}

/// Returns the maximum between the integers a and b
static inline int max(int a, int b) {
  if (a > b) {
    return a;
  }

  return b;
}

/// Rounds the double n to the nearest integer
static inline int round(double n) {
  if (n > 0) {
    return (int)(n + 0.5);
  }

  return (int)(n - 0.5);
}

/// Init Image library.  (Call once!)
/// Currently, simply calibrate instrumentation and set names of counters.
void ImageInit(void) {  ///
  InstrCalibrate();
  InstrName[0] = "pixmem";  // InstrCount[0] will count pixel array acesses
  // Name other counters here...
  InstrName[1] = "pixcmp";
  InstrName[2] = "pixadd";
}

// Macros to simplify accessing instrumentation counters:
#define PIXMEM InstrCount[0]
// Add more macros here...
#define PIXCMP InstrCount[1]
#define PIXADD InstrCount[2]

// TIP: Search for PIXMEM or InstrCount to see where it is incremented!

/// Image management functions

/// Create a new black image.
///   width, height : the dimensions of the new image.
///   maxval: the maximum gray level (corresponding to white).
/// Requires: width and height must be non-negative, maxval > 0.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageCreate(int width, int height, uint8 maxval) {  ///
  assert(width >= 0);
  assert(height >= 0);
  assert(0 < maxval && maxval <= PixMax);

  Image img = (Image)malloc(sizeof(struct image));

  if (img == NULL) {
    errCause = "Memory allocation for Image structure failed";
    return NULL;
  }

  img->width = width;
  img->height = height;
  img->maxval = maxval;
  img->pixel = malloc(img->width * img->height);

  if (img->pixel == NULL) {
    errCause = "Memory allocation for pixel array failed";
    free(img);
    return NULL;
  }

  return img;
}

/// Destroy the image pointed to by (*imgp).
///   imgp : address of an Image variable.
/// If (*imgp)==NULL, no operation is performed.
/// Ensures: (*imgp)==NULL.
/// Should never fail, and should preserve global errno/errCause.
void ImageDestroy(Image* imgp) {  ///
  assert(imgp != NULL);

  Image img = *imgp;

  free(img->pixel);
  free(img);

  *imgp = NULL;

  assert(*imgp == NULL);
}

/// PGM file operations

// See also:
// PGM format specification: http://netpbm.sourceforge.net/doc/pgm.html

// Match and skip 0 or more comment lines in file f.
// Comments start with a # and continue until the end-of-line, inclusive.
// Returns the number of comments skipped.
static int skipComments(FILE* f) {
  char c;
  int i = 0;
  while (fscanf(f, "#%*[^\n]%c", &c) == 1 && c == '\n') {
    i++;
  }
  return i;
}

/// Load a raw PGM file.
/// Only 8 bit PGM files are accepted.
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageLoad(const char* filename) {  ///
  int w, h;
  int maxval;
  char c;
  FILE* f = NULL;
  Image img = NULL;

  int success =
      check((f = fopen(filename, "rb")) != NULL, "Open failed") &&
      // Parse PGM header
      check(fscanf(f, "P%c ", &c) == 1 && c == '5', "Invalid file format") &&
      skipComments(f) >= 0 &&
      check(fscanf(f, "%d ", &w) == 1 && w >= 0, "Invalid width") &&
      skipComments(f) >= 0 &&
      check(fscanf(f, "%d ", &h) == 1 && h >= 0, "Invalid height") &&
      skipComments(f) >= 0 &&
      check(fscanf(f, "%d", &maxval) == 1 && 0 < maxval && maxval <= (int)PixMax, "Invalid maxval") &&
      check(fscanf(f, "%c", &c) == 1 && isspace(c), "Whitespace expected") &&
      // Allocate image
      (img = ImageCreate(w, h, (uint8)maxval)) != NULL &&
      // Read pixels
      check(fread(img->pixel, sizeof(uint8), w * h, f) == w * h, "Reading pixels");
  PIXMEM += (unsigned long)(w * h);  // count pixel memory accesses

  // Cleanup
  if (!success) {
    errsave = errno;
    ImageDestroy(&img);
    errno = errsave;
  }
  if (f != NULL) fclose(f);
  return img;
}

/// Save image to PGM file.
/// On success, returns nonzero.
/// On failure, returns 0, errno/errCause are set appropriately, and
/// a partial and invalid file may be left in the system.
int ImageSave(Image img, const char* filename) {  ///
  assert(img != NULL);
  int w = img->width;
  int h = img->height;
  uint8 maxval = img->maxval;
  FILE* f = NULL;

  int success =
      check((f = fopen(filename, "wb")) != NULL, "Open failed") &&
      check(fprintf(f, "P5\n%d %d\n%u\n", w, h, maxval) > 0, "Writing header failed") &&
      check(fwrite(img->pixel, sizeof(uint8), w * h, f) == w * h, "Writing pixels failed");
  PIXMEM += (unsigned long)(w * h);  // count pixel memory accesses

  // Cleanup
  if (f != NULL) fclose(f);
  return success;
}

/// Information queries

/// These functions do not modify the image and never fail.

/// Get image width
int ImageWidth(Image img) {  ///
  assert(img != NULL);
  return img->width;
}

/// Get image height
int ImageHeight(Image img) {  ///
  assert(img != NULL);
  return img->height;
}

/// Get image maximum gray level
int ImageMaxval(Image img) {  ///
  assert(img != NULL);
  return img->maxval;
}

/// Pixel stats
/// Find the minimum and maximum gray levels in image.
/// On return,
/// *min is set to the minimum gray level in the image,
/// *max is set to the maximum.
void ImageStats(Image img, uint8* min, uint8* max) {  ///
  assert(img != NULL);

  uint8 first_pixel = ImageGetPixel(img, 0, 0);
  *min = first_pixel;
  *max = first_pixel;

  for (int i = 1; i < img->width * img->height; ++i) {
    PIXMEM += 1;

    uint8 pixel = img->pixel[i];

    if (pixel < *min) {
      *min = pixel;
    } else if (pixel > *max) {
      *max = pixel;
    }
  }
}

/// Check if pixel position (x,y) is inside img.
int ImageValidPos(Image img, int x, int y) {  ///
  assert(img != NULL);
  return (0 <= x && x < img->width) && (0 <= y && y < img->height);
}

/// Check if rectangular area (x,y,w,h) is completely inside img.
int ImageValidRect(Image img, int x, int y, int w, int h) {  ///
  assert(img != NULL);
  assert(w >= 0);
  assert(h >= 0);

  return ImageValidPos(img, x, y) && ImageValidPos(img, x + w, y + h);
}

/// Pixel get & set operations

/// These are the primitive operations to access and modify a single pixel
/// in the image.
/// These are very simple, but fundamental operations, which may be used to
/// implement more complex operations.

// Transform (x, y) coords into linear pixel index.
// This internal function is used in ImageGetPixel / ImageSetPixel.
// The returned index must satisfy (0 <= index < img->width*img->height)
static inline int G(Image img, int x, int y) {
  int index = (y * img->width) + x;
  assert(0 <= index && index < img->width * img->height);
  return index;
}

/// Get the pixel (level) at position (x,y).
uint8 ImageGetPixel(Image img, int x, int y) {  ///
  assert(img != NULL);
  assert(ImageValidPos(img, x, y));
  PIXMEM += 1;  // count one pixel access (read)
  return img->pixel[G(img, x, y)];
}

/// Set the pixel at position (x,y) to new level.
void ImageSetPixel(Image img, int x, int y, uint8 level) {  ///
  assert(img != NULL);
  assert(ImageValidPos(img, x, y));
  PIXMEM += 1;  // count one pixel access (store)
  img->pixel[G(img, x, y)] = level;
}

/// Pixel transformations

/// These functions modify the pixel levels in an image, but do not change
/// pixel positions or image geometry in any way.
/// All of these functions modify the image in-place: no allocation involved.
/// They never fail.

/// Transform image to negative image.
/// This transforms dark pixels to light pixels and vice-versa,
/// resulting in a "photographic negative" effect.
void ImageNegative(Image img) {  ///
  assert(img != NULL);

  for (int i = 0; i < img->width * img->height; ++i) {
    PIXMEM += 1;
    img->pixel[i] = img->maxval - img->pixel[i];
  }
}

/// Apply threshold to image.
/// Transform all pixels with level<thr to black (0) and
/// all pixels with level>=thr to white (maxval).
void ImageThreshold(Image img, uint8 thr) {  ///
  assert(img != NULL);

  for (int i = 0; i < img->width * img->height; ++i) {
    PIXMEM += 1;
    uint8 pixel = img->pixel[i];

    if (pixel >= thr) {
      img->pixel[i] = img->maxval;
    } else {
      img->pixel[i] = 0;
    }
  }
}

/// Brighten image by a factor.
/// Multiply each pixel level by a factor, but saturate at maxval.
/// This will brighten the image if factor>1.0 and
/// darken the image if factor<1.0.
void ImageBrighten(Image img, double factor) {  ///
  assert(img != NULL);
  assert(factor >= 0.0);

  for (int i = 0; i < img->width * img->height; ++i) {
    PIXMEM += 1;
    uint8 new_color = round(img->pixel[i] * factor);

    if (new_color > img->maxval) {
      new_color = img->maxval;
    }

    img->pixel[i] = new_color;
  }
}

/// Geometric transformations

/// These functions apply geometric transformations to an image,
/// returning a new image as a result.
///
/// Success and failure are treated as in ImageCreate:
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.

// Implementation hint:
// Call ImageCreate whenever you need a new image!

/// Rotate an image.
/// Returns a rotated version of the image.
/// The rotation is 90 degrees anti-clockwise.
/// Ensures: The original img is not modified.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageRotate(Image img) {  ///
  assert(img != NULL);

  Image new_img = ImageCreate(img->height, img->width, img->maxval);

  if (new_img == NULL) {
    return NULL;
  }

  for (int y = 0; y < img->height; ++y) {
    for (int x = 0; x < img->width; ++x) {
      uint8 pixel = ImageGetPixel(img, x, y);
      ImageSetPixel(new_img, y, img->height - x - 1, pixel);
    }
  }

  return new_img;
}

/// Mirror an image = flip left-right.
/// Returns a mirrored version of the image.
/// Ensures: The original img is not modified.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageMirror(Image img) {  ///
  assert(img != NULL);

  Image new_img = ImageCreate(img->width, img->height, img->maxval);

  if (new_img == NULL) {
    return NULL;
  }

  for (int y = 0; y < img->height; ++y) {
    for (int x = 0; x < img->height; ++x) {
      uint8 pixel = ImageGetPixel(img, x, y);
      ImageSetPixel(new_img, new_img->width - x - 1, y, pixel);
    }
  }

  return new_img;
}

/// Crop a rectangular subimage from img.
/// The rectangle is specified by the top left corner coords (x, y) and
/// width w and height h.
/// Requires:
///   The rectangle must be inside the original image.
/// Ensures:
///   The original img is not modified.
///   The returned image has width w and height h.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageCrop(Image img, int x, int y, int w, int h) {  ///
  assert(img != NULL);
  assert(ImageValidRect(img, x, y, w, h));

  Image new_img = ImageCreate(w, h, img->maxval);

  if (new_img == NULL) {
    return NULL;
  }

  for (int i = x; i < x + w; ++i) {
    for (int j = y; j < y + h; ++j) {
      uint8 pixel = ImageGetPixel(img, i, j);
      ImageSetPixel(new_img, i - x, j - x, pixel);
    }
  }

  assert(new_img->width == w && new_img->height == h);

  return new_img;
}

/// Operations on two images

/// Paste an image into a larger image.
/// Paste img2 into position (x, y) of img1.
/// This modifies img1 in-place: no allocation involved.
/// Requires: img2 must fit inside img1 at position (x, y).
void ImagePaste(Image img1, int x, int y, Image img2) {  ///
  assert(img1 != NULL);
  assert(img2 != NULL);
  assert(ImageValidRect(img1, x, y, img2->width, img2->height));

  for (int y0 = 0; y0 < img2->height; ++y0) {
    for (int x0 = 0; x0 < img2->width; ++x0) {
      uint8 pixel = ImageGetPixel(img2, x0, y0);
      ImageSetPixel(img1, x + x0, y + y0, pixel);
    }
  }
}

/// Blend an image into a larger image.
/// Blend img2 into position (x, y) of img1.
/// This modifies img1 in-place: no allocation involved.
/// Requires: img2 must fit inside img1 at position (x, y).
/// alpha usually is in [0.0, 1.0], but values outside that interval
/// may provide interesting effects.  Over/underflows should saturate.
void ImageBlend(Image img1, int x, int y, Image img2, double alpha) {  ///
  assert(img1 != NULL);
  assert(img2 != NULL);
  assert(ImageValidRect(img1, x, y, img2->width, img2->height));

  for (int y0 = 0; y0 < img2->height; ++y0) {
    for (int x0 = 0; x0 < img2->width; ++x0) {
      uint8 pixel1 = ImageGetPixel(img1, x + x0, y + y0);
      uint8 pixel2 = ImageGetPixel(img2, x0, y0);

      int blended_pixel = round(pixel1 * (1 - alpha) + pixel2 * alpha);

      if (blended_pixel > img1->maxval) {
        blended_pixel = img1->maxval;
      } else if (blended_pixel < 0) {
        blended_pixel = 0;
      }

      ImageSetPixel(img1, x + x0, y + y0, blended_pixel);
    }
  }
}

/// Compare an image to a subimage of a larger image.
/// Returns 1 (true) if img2 matches subimage of img1 at pos (x, y).
/// Returns 0, otherwise.
int ImageMatchSubImage(Image img1, int x, int y, Image img2) {  ///
  assert(img1 != NULL);
  assert(img2 != NULL);
  assert(ImageValidPos(img1, x, y));
  int match = 1;

  for (int y0 = 0; y0 < img2->height && match; ++y0) {
    for (int x0 = 0; x0 < img2->width && match; ++x0) {
      uint8 pixel1 = ImageGetPixel(img1, x + x0, y + y0);
      uint8 pixel2 = ImageGetPixel(img2, x0, y0);
      PIXCMP += 1;
      match = pixel1 == pixel2;
    }
  }

  return match;
}

/// Locate a subimage inside another image.
/// Searches for img2 inside img1.
/// If a match is found, returns 1 and matching position is set in vars (*px, *py).
/// If no match is found, returns 0 and (*px, *py) are left untouched.
int ImageLocateSubImage(Image img1, int* px, int* py, Image img2) {  ///
  assert(img1 != NULL);
  assert(img2 != NULL);

  for (int x = 0; x <= img1->width - img2->width; ++x) {
    for (int y = 0; y <= img1->height - img2->height; ++y) {
      if (ImageMatchSubImage(img1, x, y, img2)) {
        *px = x;
        *py = y;
        return 1;
      }
    }
  }
  return 0;
}

/// Filtering

/// Returns the average color of the pixels inside the rectangle.
static uint8 RectAvgColor(const Image img, int x, int y, int w, int h) {
  assert(img != NULL);
  assert(ImageValidRect(img, x, y, w, h));

  int sum = 0;

  for (int row = y; row < y + h; ++row) {
    for (int col = x; col < x + w; ++col) {
      // printf("%d ", ImageGetPixel(img, c, r));
      PIXADD += 1;
      sum += ImageGetPixel(img, col, row);
    }
    // printf("\n");
  }

  return round((double)sum / (w * h));
}

/// Blur an image by a applying a (2dx+1)x(2dy+1) mean filter.
/// Each pixel is substituted by the mean of the pixels in the rectangle
/// [x-dx, x+dx]x[y-dy, y+dy].
/// The image is changed in-place.
/// This algorithm takes each pixel and calculates the average color in the
/// rectangle [x-dx, x+dx]x[y-dy, y+dy]
void ImageBlur3(Image img, int dx, int dy) {  ///
  assert(img != NULL);

  Image img_copy = ImageCreate(img->width, img->height, img->maxval);

  if (img_copy == NULL) {
    fprintf(stderr, "ERROR: Memory allocation failed: %s\n", strerror(errno));
    exit(1);
  }

  memcpy(img_copy->pixel, img->pixel, img->height * img->width);

  int x0, y0, w, h;

  for (int y = 0; y < img->height; ++y) {
    for (int x = 0; x < img->width; ++x) {
      x0 = max(0, x - dx);
      y0 = max(0, y - dy);

      w = min(dx + min(dx + 1, x), img->width - x);
      h = min(dy + min(dy + 1, y), img->height - y);
      ImageSetPixel(img, x, y, RectAvgColor(img_copy, x0, y0, w, h));
    }
  }

  ImageDestroy(&img_copy);
}

/// A little better blur algorithm
/// This algorithm uses an cumulative sum array to re-use the rows sum
/// in the calculation of the average pixel color inside the rectangle
void ImageBlur2(Image img, int dx, int dy) {
  assert(img != NULL);

  // An array of the cumulative sum of the pixels row-wise
  uint32_t* pixels_sum = malloc(img->width * img->height * sizeof(uint32_t));

  if (pixels_sum == NULL) {
    fprintf(stderr, "ERROR: Failed to allocate memory for pixels sum array: %s\n", strerror(errno));
    exit(1);
  }

  for (int y = 0; y < img->height; ++y) {
    for (int x = 0; x < img->width; ++x) {
      pixels_sum[y * img->width + x] = ImageGetPixel(img, x, y);

      if (x != 0) {
        PIXADD += 1;
        pixels_sum[y * img->width + x] += pixels_sum[y * img->width + x - 1];
      }
    }
  }

  int x0, y0, x1, y1, w, h;

  for (int y = 0; y < img->height; ++y) {
    for (int x = 0; x < img->width; ++x) {
      x0 = max(0, x - dx);
      y0 = max(0, y - dy);

      x1 = min(x + dx, img->width - 1);
      y1 = min(y + dy, img->height - 1);

      w = x1 - x0 + 1;
      h = y1 - y0 + 1;

      int sum = 0;

      // Calculate the sum of each row using the pixels_sum cumulative sum array defined above
      for (int row = y0; row <= y1; ++row) {
        sum += pixels_sum[row * img->width + x1];
        PIXADD += 1;

        // If the left border doesn't touch the image edge
        if (x0 != 0) {
          sum -= pixels_sum[row * img->width + x0 - 1];
          PIXADD += 1;
        }

        uint8 blurred_pixel = round((double)sum / (w * h));
        ImageSetPixel(img, x, y, blurred_pixel);
      }
    }
  }
  free(pixels_sum);
}

/// A better blur algorithm
/// This algorithm uses an cumulative sum array to re-use all the sums
void ImageBlur(Image img, int dx, int dy) {
  assert(img != NULL);

  // Each array entry is the cumulative sum of the pixel colors in the rectangle
  // defined with the left top corner (0, 0) and the right bottom corner (i, j)
  // Obs: uint32 should be enough for ~ 4100x4100 *white* image
  uint32_t* pixels_sum = malloc(img->width * img->height * sizeof(uint32_t));

  if (pixels_sum == NULL) {
    fprintf(stderr, "ERROR: Failed to allocate memory for pixels sum array: %s\n", strerror(errno));
    exit(1);
  }

  int x, y;

  // Calculate the first row of the matrix
  pixels_sum[0] = ImageGetPixel(img, 0, 0);
  for (x = 1; x < img->width; ++x) {
    PIXADD += 1;
    pixels_sum[x] = ImageGetPixel(img, x, 0) + pixels_sum[x - 1];
  }

  // Calculate the remaining rows of the cumulative sum matrix
  for (y = 1; y < img->height; ++y) {
    for (x = 0; x < img->width; ++x) {
      PIXADD += 1;
      pixels_sum[y * img->width + x] = ImageGetPixel(img, x, y) + pixels_sum[(y - 1) * img->width + x];

      if (x != 0) {
        PIXADD += 2;
        pixels_sum[y * img->width + x] += pixels_sum[y * img->width + x - 1] - pixels_sum[(y - 1) * img->width + x - 1];
      }
    }
  }

  int x0, y0, x1, y1, x2, y2, x3, y3, w, h;

  // For each image pixel, calculate the average pixel color inside the blur rectangle
  for (y = 0; y < img->height; ++y) {
    for (x = 0; x < img->width; ++x) {
      // Top left corner
      x0 = max(0, x - dx);
      y0 = max(0, y - dy);

      // Bottom right
      x1 = min(x + dx, img->width - 1);
      y1 = min(y + dy, img->height - 1);

      // Top right
      x2 = x1;
      y2 = y0;

      // Bottom left
      x3 = x0;
      y3 = y1;

      // Needed to calculate the number of pixels in the rectangle to calc the average color of the blurred pixel
      w = x1 - x0 + 1;
      h = y1 - y0 + 1;

      // Considering the cumulative sum matrix above, the sum of the pixel colors inside the rectangle is equal to
      // bottom right corner - bottom left corner - top right corner + top left corner
      // we add the top left corner, because that sum is subtracted twice (bottom left corner and top right corner)
      uint32_t sum = pixels_sum[y1 * img->width + x1];

      // Subtract top right corner
      if (y2 != 0) {
        PIXADD += 1;
        sum -= pixels_sum[(y2 - 1) * img->width + x2];
      }
      // Subtract bottom left corner
      if (x3 != 0) {
        PIXADD += 1;
        sum -= pixels_sum[y3 * img->width + x3 - 1];
      }
      // Re-add top left corner
      if (x0 != 0 && y0 != 0) {
        PIXADD += 1;
        sum += pixels_sum[(y0 - 1) * img->width + x0 - 1];
      }
      uint8 blurred_pixel = round((double)sum / (w * h));
      ImageSetPixel(img, x, y, blurred_pixel);
    }
  }

  free(pixels_sum);
}