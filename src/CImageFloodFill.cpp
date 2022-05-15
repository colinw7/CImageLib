#include <CImageFloodFill.h>

struct CFillSegment {
  int y, xl, xr, dy;
};

#define CIMAGE_FLOOD_FILL_MAX_DEPTH 16384

#define CIMAGE_FLOOD_FILL_PUSH(Y, XL, XR, DY) \
  if (sp < stack + CIMAGE_FLOOD_FILL_MAX_DEPTH && \
      Y + (DY) >= bottom && Y + (DY) <= top) { \
    sp->y = Y; sp->xl = XL; sp->xr = XR; sp->dy = DY; sp++; \
  }

#define CIMAGE_FLOOD_FILL_POP(Y, XL, XR, DY) \
  { \
    sp--; Y = sp->y; XL = sp->xl; XR = sp->xr; DY = sp->dy; \
  }

void
CImage::
floodFill(int x, int y, const CRGBA &rgba)
{
  if (! CASSERT(! hasColormap(), "RGB image required")) return;

  int left, bottom, right, top;

  getWindow(&left, &bottom, &right, &top);

  if (x < left || x > right || y < bottom || y > top)
    return;

  CRGBA rgba1;

  getRGBAPixel(x, y, rgba1);

  if (rgba1 == rgba)
    return;

  //-------

  CFillSegment stack[CIMAGE_FLOOD_FILL_MAX_DEPTH];

  CFillSegment *sp = stack;

  CIMAGE_FLOOD_FILL_PUSH(y    , x, x,  1); // needed in some cases
  CIMAGE_FLOOD_FILL_PUSH(y + 1, x, x, -1); // seed segment (popped 1st)

  CRGBA rgba2;
  int   x1, x2, dy, l;

  while (sp > stack) {
    // pop segment off stack and fill a neighboring scan line */

    CIMAGE_FLOOD_FILL_POP(y, x1, x2, dy);

    y += dy;

    // segment of scan line y - dy for x1 <= x <= x2 was previously filled,
    // now explore adjacent pixels in scan line y

    for (x = x1; x >= left; x--) {
      getRGBAPixel(x, y, rgba2);

      if (rgba2 != rgba1)
        break;

      setRGBAPixel(x, y, rgba);
    }

    if (x >= x1)
      goto skip;

    l = x + 1;

    if (l < x1)
      CIMAGE_FLOOD_FILL_PUSH(y, l, x1 - 1, -dy); // leak on left?

    x = x1 + 1;

    do {
      for (; x <= right; x++) {
        getRGBAPixel(x, y, rgba2);

        if (rgba2 != rgba1)
          break;

        setRGBAPixel(x, y, rgba);
      }

      CIMAGE_FLOOD_FILL_PUSH(y, l, x - 1, dy);

      if (x > x2 + 1)
        CIMAGE_FLOOD_FILL_PUSH(y, x2 + 1, x - 1, -dy); // leak on right?

 skip:
      for (x++; x <= x2; x++) {
        getRGBAPixel(x, y, rgba2);

        if (rgba2 == rgba1)
          break;
      }

      l = x;
    }
    while (x <= x2);
  }
}

void
CImage::
floodFill(int x, int y, int pixel)
{
  if (! CASSERT(hasColormap(), "Colormap image required")) return;

  int left, bottom, right, top;

  getWindow(&left, &bottom, &right, &top);

  if (x < left || x > right || y < bottom || y > top)
    return;

  int pixel1 = getColorIndexPixel(x, y);

  if (pixel1 == pixel)
    return;

  //-------

  CFillSegment stack[CIMAGE_FLOOD_FILL_MAX_DEPTH];

  CFillSegment *sp = stack;

  CIMAGE_FLOOD_FILL_PUSH(y    , x, x,  1); // needed in some cases
  CIMAGE_FLOOD_FILL_PUSH(y + 1, x, x, -1); // seed segment (popped 1st)

  int pixel2;
  int x1, x2, dy, l;

  while (sp > stack) {
    // pop segment off stack and fill a neighboring scan line */

    CIMAGE_FLOOD_FILL_POP(y, x1, x2, dy);

    y += dy;

    // segment of scan line y - dy for x1 <= x <= x2 was previously filled,
    // now explore adjacent pixels in scan line y

    for (x = x1; x >= left; x--) {
      pixel2 = getColorIndexPixel(x, y);

      if (pixel2 != pixel1)
        break;

      setColorIndexPixel(x, y, uint(pixel));
    }

    if (x >= x1)
      goto skip;

    l = x + 1;

    if (l < x1)
      CIMAGE_FLOOD_FILL_PUSH(y, l, x1 - 1, -dy); // leak on left?

    x = x1 + 1;

    do {
      for (; x <= right; x++) {
        pixel2 = getColorIndexPixel(x, y);

        if (pixel2 != pixel1)
          break;

        setColorIndexPixel(x, y, uint(pixel));
      }

      CIMAGE_FLOOD_FILL_PUSH(y, l, x - 1, dy);

      if (x > x2 + 1)
        CIMAGE_FLOOD_FILL_PUSH(y, x2 + 1, x - 1, -dy); // leak on right?

 skip:
      for (x++; x <= x2; x++) {
        pixel2 = getColorIndexPixel(x, y);

        if (pixel2 == pixel1)
          break;
      }

      l = x;
    }
    while (x <= x2);
  }
}

class CMapImageAdapter {
 public:
  CMapImageAdapter(CImage *image) :
   image_(image) {
  }

  int getValue(int x, int y) const {
    return image_->getColorIndexPixel(x, y);
  }

 private:
  CImage *image_;
};

class RGBImageAdapter {
 public:
  RGBImageAdapter(CImage *image) :
   image_(image) {
  }

  CRGBA getValue(int x, int y) const {
    CRGBA rgba;

    image_->getRGBAPixel(x, y, rgba);

    return rgba;
  }

 private:
  CImage *image_;
};

#ifdef LARGEST_RECT

#include <CLargestRect.h>

void
CImage::
fillLargestRect(int x, int y, int pixel)
{
  if (! hasColormap()) return;

  int fg = getColorIndexPixel(x, y);

  CMapImageAdapter adapter(this);

  typedef CLargestRect<CMapImageAdapter,int> LargestRect;

  LargestRect rect(adapter, int(getWidth()), int(getHeight()));

  auto r = rect.largestRect(fg);

  for (int yy = 0; yy < r.height; ++yy)
    for (int xx = 0; xx < r.width; ++xx)
      setColorIndexPixel(r.left + xx, r.top + yy, uint(pixel));
}

void
CImage::
fillLargestRect(int x, int y, const CRGBA &rgba)
{
  if (hasColormap()) return;

  CRGBA fg;

  getRGBAPixel(x, y, fg);

  RGBImageAdapter adapter(this);

  typedef CLargestRect<RGBImageAdapter,CRGBA> LargestRect;

  LargestRect rect(adapter, int(getWidth()), int(getHeight()));

  auto r = rect.largestRect(fg);

  for (int yy = 0; yy < r.height; ++yy)
    for (int xx = 0; xx < r.width; ++xx)
      setRGBAPixel(r.left + xx, r.top + yy, rgba);
}

#endif
