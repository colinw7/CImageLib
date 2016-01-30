#include <CImageScroll.h>

void
CImage::
scroll(int dx, int dy)
{
  scrollX(dx);
  scrollY(dy);
}

void
CImage::
scrollX(int offset)
{
  uint *p1;

  int left, bottom, right, top;

  getWindow(&left, &bottom, &right, &top);

  int width = right - left + 1;

  std::vector<uint> buffer;

  buffer.resize(width);

  while (offset < 0)
    offset += width;

  while (offset > width)
    offset -= width;

  for (int y = bottom; y <= top; ++y) {
    p1 = data_ + y*size_.width + left;

    for (int x = left, i = offset; x <= right; ++x, ++i, p1++)
      buffer[i % width] = *p1;

    p1 = data_ + y*size_.width + left;

    for (int x = left, i = 0; x <= right; ++x, ++i, p1++)
      *p1 = buffer[i];
  }
}

void
CImage::
scrollY(int offset)
{
  uint *p1;

  int left, bottom, right, top;

  getWindow(&left, &bottom, &right, &top);

  int height = top - bottom + 1;

  std::vector<uint> buffer;

  buffer.resize(height);

  while (offset < 0)
    offset += height;

  while (offset > height)
    offset -= height;

  for (int x = left; x <= right; ++x) {
    p1 = data_ + bottom*size_.width + x;

    for (int y = bottom, i = offset; y <= top; ++y, ++i, p1 += size_.width)
      buffer[i % height] = *p1;

    p1 = data_ + bottom*size_.width + x;

    for (int y = bottom, i = 0; y <= top; ++y, ++i, p1 += size_.width)
      *p1 = buffer[i];
  }
}
