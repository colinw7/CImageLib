#include <CImageCombine.h>

bool
CImage::
combineDef(CImagePtr image1, CImagePtr image2)
{
  return combine(image1, image2, combine_def_);
}

bool
CImage::
combineDef(CImagePtr image)
{
  return combine(image, combine_def_);
}

bool
CImage::
combine(CImagePtr image1, CImagePtr image2, const CRGBACombineDef &def)
{
  return image1->combine(image2, def);
}

bool
CImage::
combine(CImagePtr image, const CRGBACombineDef &def)
{
  if (hasColormap() || image->hasColormap()) {
    CImage::errorMsg("Can only combine RGBA images");
    return false;
  }

  auto w = std::min(getWidth (), image->getWidth ());
  auto h = std::min(getHeight(), image->getHeight());

  for (size_t y = 0; y < h; ++y) {
    for (size_t x = 0; x < w; ++x) {
      CRGBA rgba1, rgba2;

             getRGBAPixel(int(x), int(y), rgba1);
      image->getRGBAPixel(int(x), int(y), rgba2);

      CRGBA rgba = def.combine(rgba1, rgba2);

      setRGBAPixel(int(x), int(y), rgba.clamp());
    }
  }

  dataChanged();

  return true;
}

bool
CImage::
combine(CImagePtr image, CRGBABlendMode mode)
{
  if (hasColormap() || image->hasColormap()) {
    CImage::errorMsg("Can only combine RGBA images");
    return false;
  }

  CRGBA rgba1, rgba2;

  auto w = std::min(getWidth (), image->getWidth ());
  auto h = std::min(getHeight(), image->getHeight());

  for (size_t y = 0; y < h; ++y) {
    for (size_t x = 0; x < w; ++x) {
             getRGBAPixel(int(x), int(y), rgba1);
      image->getRGBAPixel(int(x), int(y), rgba2);

      CRGBA rgba = CRGBA::blendCombine(rgba1, rgba2, mode);

      setRGBAPixel(int(x), int(y), rgba.clamp());
    }
  }

  dataChanged();

  return true;
}

bool
CImage::
combine(CImagePtr image1, CImagePtr image2)
{
  return image1->combine(0, 0, image2);
}

bool
CImage::
combine(CImagePtr image)
{
  return combine(0, 0, image);
}

bool
CImage::
combine(int x, int y, CImagePtr image)
{
  if (hasColormap() || image->hasColormap()) {
    CImage::errorMsg("Can only combine RGBA images");
    return false;
  }

  auto w = std::min(getWidth (), image->getWidth ());
  auto h = std::min(getHeight(), image->getHeight());

  for (size_t y1 = 0; y1 < h; ++y1) {
    for (size_t x1 = 0; x1 < w; ++x1) {
      if (! validPixel(int(x1) + x, int(y1) + y))
        continue;

      CRGBA rgba1;

      image->getRGBAPixel(int(x1), int(y1), rgba1);

      if (! rgba1.getAlphaI())
        continue;

      CRGBA rgba2;

      getRGBAPixel(int(x1) + x, int(y1) + y, rgba2);

      CRGBA rgba = rgba2.combined(rgba1);

      setRGBAPixel(int(x1) + x, int(y1) + y, rgba);
    }
  }

  dataChanged();

  return true;
}
