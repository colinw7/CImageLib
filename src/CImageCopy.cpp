#include <CImageCopy.h>

#include <cstring>

CImageCopyType CImage::copy_type = CIMAGE_COPY_ALL;

CImageCopyType
CImage::
setCopyType(CImageCopyType type)
{
  std::swap(copy_type, type);

  return type;
}

CImagePtr
CImage::
subImage(int x, int y, int width, int height) const
{
  if (width < 0)
    width = size_.width - x;

  if (height < 0)
    height = size_.height - y;

  CImagePtr image = CImageMgrInst->createImage();

  image->setDataSize(width, height);

  subCopyTo(image, x, y, width, height, 0, 0);

  image->dataChanged();

  return image;
}

void
CImage::
copyFrom(CImagePtr src)
{
  uint num_data1 =      getDataSize();
  uint num_data2 = src->getDataSize();
  bool colormap1 =      hasColormap();
  bool colormap2 = src->hasColormap();

  if ((num_data1 == num_data2) && (colormap1 == colormap2)) {
    if (! colormap1) {
      memcpy(data_, src->data_, num_data1*sizeof(uint));

      dataChanged();

      return;
    }

    uint num_colors1 =      getNumColors();
    uint num_colors2 = src->getNumColors();

    if (num_colors1 == num_colors2) {
      memcpy(data_, src->data_, num_data1*sizeof(uint));

      for (uint i = 0; i < num_colors1; ++i)
        colors_[i] = src->colors_[i];

      dataChanged();

      return;
    }
  }

  subCopyFrom(src, 0, 0, -1, -1, 0, 0);
}

void
CImage::
copyFrom(CImagePtr src, int dst_x, int dst_y)
{
  subCopyFrom(src, 0, 0, -1, -1, dst_x, dst_y);
}

void
CImage::
copyTo(CImagePtr &dst, int dst_x, int dst_y) const
{
  subCopyTo(dst, 0, 0, -1, -1, dst_x, dst_y);
}

void
CImage::
subCopyFrom(CImagePtr src, int src_x, int src_y, int width, int height, int dst_x, int dst_y)
{
  int src_width  = src->getWidth ();
  int src_height = src->getHeight();

  int dst_width  = getWidth ();
  int dst_height = getHeight();

  if (width  < 0) width  = src_width;
  if (height < 0) height = src_height;

  int width1  = width;
  int height1 = height;

  if (src_x +  width1 > src_width )  width1 = src_width  - src_x;
  if (src_y + height1 > src_height) height1 = src_height - src_y;
  if (dst_x +  width1 > dst_width )  width1 = dst_width  - dst_x;
  if (dst_y + height1 > dst_height) height1 = dst_height - dst_y;

  if (src->hasColormap()) {
    convertToColorIndex();

    std::map<int,int> colorMap;

    int num_colors = src->getNumColors();

    for (int i = 0; i < num_colors; ++i) {
      if (copy_type == CIMAGE_COPY_SKIP_TRANSPARENT &&
          src->isTransparentColor(i))
        continue;

      int ind = findColor(src->getColor(i));

      if (ind < 0)
        ind = addColor(src->getColor(i));

      colorMap[i] = ind;
    }

    int ys = src_y, yd = dst_y;

    for (int y = 0; y < height1; ++y, ++ys, ++yd) {
      int xs = src_x, xd = dst_x;

      for (int x = 0; x < width1; ++x, ++xs, ++xd) {
        int pixel;

        if (src->validPixel(xs, ys))
          pixel = src->getColorIndexPixel(xs, ys);
        else
          pixel = 0;

        if (copy_type == CIMAGE_COPY_SKIP_TRANSPARENT && src->isTransparentColor(pixel))
          continue;

        if (validPixel(xd, yd))
          setColorIndexPixel(xd, yd, colorMap[pixel]);
      }
    }
  }
  else {
    int ys = src_y, yd = dst_y;

    for (int y = 0; y < height1; ++y, ++ys, ++yd) {
      int xs = src_x, xd = dst_x;

      for (int x = 0; x < width1; ++x, ++xs, ++xd) {
        if (src->validPixel(xs, ys) && ! src->isTransparent(xs, ys)) {
          CRGBA rgba;

          src->getRGBAPixel(xs, ys, rgba);

          if (validPixel(xd, yd))
            setRGBAPixel(xd, yd, rgba);
        }
      }
    }
  }
}

void
CImage::
subCopyTo(CImagePtr &dst, int src_x, int src_y, int width, int height, int dst_x, int dst_y) const
{
  int src_width  = getWidth ();
  int src_height = getHeight();

  int dst_width  = dst->getWidth ();
  int dst_height = dst->getHeight();

  if (width  < 0) width  = src_width;
  if (height < 0) height = src_height;

  int width1  = width;
  int height1 = height;

  if (src_x +  width1 > src_width )  width1 = src_width  - src_x;
  if (src_y + height1 > src_height) height1 = src_height - src_y;
  if (dst_x +  width1 > dst_width )  width1 = dst_width  - dst_x;
  if (dst_y + height1 > dst_height) height1 = dst_height - dst_y;

  if (hasColormap()) {
    dst->convertToColorIndex();

    std::map<int,int> colorMap;

    int num_colors = getNumColors();

    for (int i = 0; i < num_colors; ++i) {
      if (copy_type == CIMAGE_COPY_SKIP_TRANSPARENT && isTransparentColor(i))
        continue;

      int ind = dst->findColor(getColor(i));

      if (ind < 0)
        ind = dst->addColor(getColor(i));

      colorMap[i] = ind;
    }

    int ys = src_y;
    int yd = dst_y;

    for (int y = 0; y < height1; ++y, ++ys, ++yd) {
      int xs = src_x;
      int xd = dst_x;

      for (int x = 0; x < width1; ++x, ++xs, ++xd) {
        int pixel;

        if (validPixel(xs, ys))
          pixel = getColorIndexPixel(xs, ys);
        else
          pixel = 0;

        if (copy_type == CIMAGE_COPY_SKIP_TRANSPARENT && isTransparentColor(pixel))
          continue;

        if (dst->validPixel(xd, yd))
          dst->setColorIndexPixel(xd, yd, colorMap[pixel]);
      }
    }
  }
  else {
    int ys = src_y;
    int yd = dst_y;

    for (int y = 0; y < height1; ++y, ++ys, ++yd) {
      int xs = src_x;
      int xd = dst_x;

      for (int x = 0; x < width1; ++x, ++xs, ++xd) {
        CRGBA rgba;

        if (validPixel(xs, ys))
          getRGBAPixel(xs, ys, rgba);
        else
          rgba = CRGBA(0,0,0,0);

        if (dst->validPixel(xd, yd))
          dst->setRGBAPixel(xd, yd, rgba);
      }
    }
  }
}

bool
CImage::
copy(CImagePtr image, int x, int y)
{
  int iwidth  = getWidth ();
  int iheight = getHeight();

  int iwidth1  = image->getWidth ();
  int iheight1 = image->getHeight();

  int x1 = x;
  int x2 = std::min(x + iwidth1  - 1, iwidth  - 1);
  int y1 = y;
  int y2 = std::min(y + iheight1 - 1, iheight - 1);

  if (hasColormap() && image->hasColormap()) {
    int color_ind;

    for (int y = y1; y <= y2; ++y) {
      for (int x = x1; x <= x2; ++x) {
        color_ind = image->getColorIndexPixel(x, y);

        drawColorIndexPoint(x, y, color_ind);
      }
    }
  }
  else {
    CRGBA rgba;

    if (hasColormap())
      convertToRGB();

    for (int y = y1; y <= y2; ++y) {
      for (int x = x1; x <= x2; ++x) {
        image->getRGBAPixel(x, y, rgba);

        drawRGBAPoint(x, y, rgba);
      }
    }
  }

  return true;
}

CImagePtr
CImage::
copy(CImagePtr image1, CImagePtr image2, int x, int y)
{
  CImagePtr image = image1->dup();

  (void) image->copy(image2, x, y);

  return image;
}

bool
CImage::
copyAlpha(CImagePtr image, int x, int y)
{
  int iwidth  = getWidth ();
  int iheight = getHeight();

  int iwidth1  = image->getWidth ();
  int iheight1 = image->getHeight();

  int x1 = x;
  int x2 = std::min(x + iwidth1  - 1, iwidth  - 1);
  int y1 = y;
  int y2 = std::min(y + iheight1 - 1, iheight - 1);

  if (hasColormap())
    convertToRGB();

  // image is mask, get alpha from rgba of mask set alpha of image pixel from mask
  for (int y = y1; y <= y2; ++y) {
    for (int x = x1; x <= x2; ++x) {
      CRGBA rgba1;

      getRGBAPixel(x, y, rgba1);

      int xx = x - x1;
      int yy = y - y1;

      if (image->validPixel(xx, yy)) {
        CRGBA rgba2;

        image->getRGBAPixel(xx, yy, rgba2);

        //double a = rgba2.getGray()*rgba2.getAlpha();
        double a = rgba2.getAlpha();

        rgba1.setAlpha(rgba1.getAlpha()*a);

        setRGBAPixel(x, y, rgba1);
      }
    }
  }

  return true;
}
