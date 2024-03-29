#include <CImageTile.h>

CImagePtr
CImage::
tile(int width, int height, const CImageTileData &tile)
{
  CImagePtr image = CImageMgrInst->createImage();

  image->setType(getType());

  image->setDataSize(width, height);

  //------

  if (hasColormap()) {
    int num_colors = getNumColors();

    for (int i = 0; i < num_colors; ++i)
      image->addColor(getColor(uint(i)));

    //------

    if (isTransparent())
      image->setTransparentColor(uint(getTransparentColor()));
  }

  //------

  int x_offset = 0;

  if      (tile.halign == CHALIGN_TYPE_CENTER)
    x_offset = (width - int(getWidth()))/2;
  else if (tile.halign == CHALIGN_TYPE_RIGHT)
    x_offset = width - int(getWidth());

  while (x_offset > 0)
    x_offset -= getWidth();

  x_offset = -x_offset;

  //------

  int y_offset = 0;

  if      (tile.valign == CVALIGN_TYPE_CENTER)
    y_offset = (height - int(getHeight()))/2;
  else if (tile.valign == CVALIGN_TYPE_BOTTOM)
    y_offset = height - int(getHeight());

  while (y_offset > 0)
    y_offset -= getHeight();

  y_offset = -y_offset;

  //------

  if (! hasColormap()) {
    for (int y = 0; y < height; ++y) {
      auto y1 = uint(y + y_offset) % getHeight();

      for (int x = 0; x < width; ++x) {
        auto x1 = uint(x + x_offset) % getWidth();

        double r, g, b, a;

        getRGBAPixel(int(x1), int(y1), &r, &g, &b, &a);

        image->setRGBAPixel(x, y, r, g, b, a);
      }
    }
  }
  else {
    for (int y = 0; y < height; ++y) {
      auto y1 = uint(y + y_offset) % getHeight();

      for (int x = 0; x < width; ++x) {
        auto x1 = uint(x + x_offset) % getWidth();

        int pixel = getColorIndexPixel(int(x1), int(y1));

        image->setColorIndexPixel(x, y, uint(pixel));
      }
    }
  }

  //------

  return image;
}
