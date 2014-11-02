#include <CImageLibI.h>

using std::min;
using std::max;
using std::cerr;
using std::endl;

void
CImage::
combineDef(CImagePtr image1, CImagePtr image2)
{
  combine(image1, image2, combine_def_);
}

void
CImage::
combineDef(CImagePtr image)
{
  combine(image, combine_def_);
}

void
CImage::
combine(CImagePtr image1, CImagePtr image2, const CRGBACombineDef &def)
{
  image1->combine(image2, def);
}

void
CImage::
combine(CImagePtr image, const CRGBACombineDef &def)
{
  if (hasColormap() || image->hasColormap()) {
    cerr << "Can only combine RGBA images" << endl;
    return;
  }

  CRGBA rgba1, rgba2;

  int w = min(getWidth (), image->getWidth ());
  int h = min(getHeight(), image->getHeight());

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
}

void
CImage::
combine(CImagePtr image1, CImagePtr image2)
{
  image1->combine(image2);
}

void
CImage::
combine(CImagePtr image)
{
  if (hasColormap() || image->hasColormap()) {
    cerr << "Can only combine RGBA images" << endl;
    return;
  }

  CRGBA rgba;

  int w = min(getWidth (), image->getWidth ());
  int h = min(getHeight(), image->getHeight());

  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      if (image->isTransparent(x, y)) continue;

      image->getRGBAPixel(x, y, rgba);

      setRGBAPixel(x, y, rgba);
    }
  }

  dataChanged();
}
