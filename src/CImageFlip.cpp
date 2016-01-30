#include <CImageFlip.h>

CImagePtr
CImage::
flippedH() const
{
  CImagePtr image = dup();

  image->flipH();

  return image;
}

CImagePtr
CImage::
flippedV() const
{
  CImagePtr image = dup();

  image->flipV();

  return image;
}

CImagePtr
CImage::
flippedHV() const
{
  CImagePtr image = dup();

  image->flipHV();

  return image;
}

void
CImage::
flipH()
{
  uint *p1, *p2;

  int left, bottom, right, top;

  getWindow(&left, &bottom, &right, &top);

  for (int x = left; x <= right; ++x) {
    p1 = data_ + x;

    p2  = p1 + top*size_.width;
    p1 += bottom*size_.width;

    for (int y = bottom; p1 < p2; ++y, p1 += size_.width, p2 -= size_.width)
      std::swap(*p1, *p2);
  }
}

void
CImage::
flipV()
{
  uint *p1, *p2;

  int left, bottom, right, top;

  getWindow(&left, &bottom, &right, &top);

  for (int y = bottom; y <= top; ++y) {
    p1 = data_ + y*size_.width;

    p2  = p1 + right;
    p1 += left;

    for (int x = left; p1 < p2; ++x, ++p1, --p2)
      std::swap(*p1, *p2);
  }
}

void
CImage::
flipHV()
{
  uint *p1, *p2;

  int left, bottom, right, top;

  getWindow(&left, &bottom, &right, &top);

  int w = right - left + 1;
  int h = top - bottom + 1;

  int s = std::min(w, h);

  right = left + s;
  top   = bottom + s;

  for (int x = 0; x < s; ++x) {
    for (int y = 0; y < s; ++y) {
      if (x > y) {
        p1 = data_ + (y + bottom)*size_.width + x + left;
        p2 = data_ + (x + bottom)*size_.width + y + left;

        std::swap(*p1, *p2);
      }
    }
  }
}
