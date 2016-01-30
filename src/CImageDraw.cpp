#include <CImageDraw.h>

void
CImage::
fillRGBARectangle(int x1, int y1, int x2, int y2, const CRGBA &rgba)
{
  if ((x1 < 0 || x1 >= size_.width || y1 < 0 || y1 >= size_.height) ||
      (x2 < 0 || x2 >= size_.width || y2 < 0 || y2 >= size_.height))
    return;

  uint pixel = rgbaToPixel(rgba);

  uint *p1 = data_ + y1*size_.width;

  for (int y = y1; y <= y2; ++y) {
    uint *p2 = p1 + x1;

    for (int x = x1; x <= x2; ++x, ++p2)
      *p2 = pixel;

    p1 += size_.width;
  }

  dataChanged();
}

void
CImage::
fillColorIndexRectangle(int x1, int y1, int x2, int y2, int ind)
{
  uint *p1 = data_ + y1*size_.width;

  for (int y = y1; y <= y2; ++y) {
    uint *p2 = p1 + x1;

    for (int x = x1; x <= x2; ++x, ++p2)
      *p2 = ind;

    p1 += size_.width;
  }

  dataChanged();
}

void
CImage::
drawColorIndexPoint(int x, int y, int color_ind)
{
  setColorIndexPixel(x, y, color_ind);
}

void
CImage::
drawColorIndexPoint(int i, int color_ind)
{
  setColorIndexPixel(i, color_ind);
}

void
CImage::
drawRGBAPoint(int x, int y, const CRGBA &rgba)
{
  if (x < 0 || x >= size_.width || y < 0 || y >= size_.height)
    return;

  if (combine_enabled_) {
    CRGBA rgba1;

    getRGBAPixel(x, y, rgba1);

    CRGBA rgba2 =
      CRGBA::modeCombine(rgba, rgba1,
                         combine_def_.src_mode,
                         combine_def_.dst_mode,
                         combine_def_.func,
                         combine_def_.factor);

    setRGBAPixel(x, y, rgba2.clamp());
  }
  else
    setRGBAPixel(x, y, rgba);
}

void
CImage::
drawRGBAPoint(int i, const CRGBA &rgba)
{
  if (combine_enabled_) {
    CRGBA rgba1;

    getRGBAPixel(i, rgba1);

    CRGBA rgba2 =
      CRGBA::modeCombine(rgba, rgba1,
                         combine_def_.src_mode,
                         combine_def_.dst_mode,
                         combine_def_.func,
                         combine_def_.factor);

    setRGBAPixel(i, rgba2.clamp());
  }
  else
    setRGBAPixel(i, rgba);
}
