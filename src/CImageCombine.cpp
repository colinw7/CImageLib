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

  CRGBA rgba1, rgba2;

  int w = std::min(getWidth (), image->getWidth ());
  int h = std::min(getHeight(), image->getHeight());

  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
             getRGBAPixel(x, y, rgba1);
      image->getRGBAPixel(x, y, rgba2);

      CRGBA rgba = CRGBA::modeCombine(rgba2, rgba1,
                                      def.src_mode, def.dst_mode,
                                      def.func, def.factor);

      setRGBAPixel(x, y, rgba.clamp());
    }
  }

  dataChanged();

  return true;
}

bool
CImage::
combine(CImagePtr image1, CImagePtr image2)
{
  return image1->combine(image2, 0, 0);
}

bool
CImage::
combine(CImagePtr image)
{
  if (hasColormap() || image->hasColormap()) {
    CImage::errorMsg("Can only combine RGBA images");
    return false;
  }

  CRGBA rgba;

  int w = std::min(getWidth (), image->getWidth ());
  int h = std::min(getHeight(), image->getHeight());

  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      if (image->isTransparent(x, y)) continue;

      image->getRGBAPixel(x, y, rgba);

      setRGBAPixel(x, y, rgba);
    }
  }

  dataChanged();

  return true;
}
