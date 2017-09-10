#include <CImageProcess.h>
#include <CRGBUtil.h>

void
CImage::
removeSinglePixels()
{
  uint data[9];

  int width  = getWidth ();
  int height = getHeight();

  for (int y = 1; y < height - 1; ++y) {
    for (int x = 1; x < width - 1; ++x) {
      data[0] = getData(x - 1, y - 1);
      data[1] = getData(x    , y - 1);
      data[2] = getData(x + 1, y - 1);
      data[3] = getData(x - 1, y    );
      data[4] = getData(x    , y    );
      data[5] = getData(x + 1, y    );
      data[6] = getData(x - 1, y + 1);
      data[7] = getData(x    , y + 1);
      data[8] = getData(x + 1, y + 1);

      if (data[0] == data[1] && data[0] == data[2] &&
          data[0] == data[3] && data[0] != data[4] &&
          data[0] == data[5] && data[0] == data[6] &&
          data[0] == data[7] && data[0] == data[8])
        setData(x, y, data[0]);
    }
  }
}

CImage::ImagePtrList
CImage::
colorSplit()
{
  std::map<uint, bool> used;

  int width  = getWidth ();
  int height = getHeight();

  int size = width*height;

  for (int i = 0; i < size; ++i) {
    uint data = getData(i);

    if (used.find(data) == used.end())
      used[data] = true;
  }

  ImagePtrList images;

  for (const auto &i : used)
    images.push_back(colorSplitByData(i.first));

  return images;
}

CImagePtr
CImage::
colorSplit(int color_num)
{
  std::map<uint, bool> used;

  int width  = getWidth ();
  int height = getHeight();

  int size = width*height;

  for (int i = 0; i < size; ++i) {
    uint data = getData(i);

    if (used.find(data) == used.end())
      used[data] = true;
  }

  int num_colors = used.size();

  if (color_num < 0 || color_num >= num_colors)
    CImage::warnMsg("Bad Color Number " + std::to_string(color_num));

  ImagePtrList images;

  auto pused = used.begin();

  for (int i = 0; i < color_num; ++i)
    ++pused;

  CImagePtr image = colorSplitByData((*pused).first);

  return image;
}

CImagePtr
CImage::
colorSplitByData(uint data)
{
  CRGBA rgba;

  if (hasColormap())
    getColorRGBA(data, rgba);
  else
    pixelToRGBA(data, rgba);

  CRGBA rgba1(0, 0, 0);

  if (rgba.getGray() <= 0.5)
    rgba1 = CRGBA(1, 1, 1);

  int width  = getWidth ();
  int height = getHeight();

  CImagePtr image = CImageMgrInst->createImage();

  image->setDataSize(width, height);

  image->addColor(rgba);
  image->addColor(rgba1);

  image->setTransparentColor(1);

  int size = width*height;

  for (int i = 0; i < size; ++i) {
    uint data2 = getData(i);

    if (data2 == data)
      image->setData(i, 0);
    else
      image->setData(i, 1);
  }

  return image;
}

void
CImage::
setNumColors(int num_colors)
{
  convertToColorIndex();

  int num_colors1 = colors_.size();

  while (num_colors1 > num_colors) {
    int i1, i2;

    getClosestColors(i1, i2);

    replaceColor(i1, i2);

    num_colors1 = colors_.size();
  }
}

void
CImage::
getClosestColors(int &i1, int &i2)
{
  double min_d = 1E50;

  int num_colors = colors_.size();

  for (int i = 0; i < num_colors; ++i) {
    double min_d1 = 1E50;
    int    min_i1 = 0;

    double r1 = colors_[i].getRed  ();
    double g1 = colors_[i].getGreen();
    double b1 = colors_[i].getBlue ();

    for (int j = 0; j < num_colors; ++j) {
      if (i == j) continue;

      double r2 = colors_[j].getRed  ();
      double g2 = colors_[j].getGreen();
      double b2 = colors_[j].getBlue ();

      double d = fabs(r2 - r1) + fabs(g2 - g1) + fabs(b2 - b1);

      if (d < min_d1) {
        min_d1 = d;
        min_i1 = j;
      }
    }

    if (min_d1 < min_d) {
      min_d = min_d1;
      i1    = i;
      i2    = min_i1;
    }
  }
}

void
CImage::
replaceColor(int i1, int i2)
{
  double r = (colors_[i1].getRed  () + colors_[i2].getRed  ())/2.0;
  double g = (colors_[i1].getGreen() + colors_[i2].getGreen())/2.0;
  double b = (colors_[i1].getBlue () + colors_[i2].getBlue ())/2.0;

  colors_[i1].setRed  (r);
  colors_[i1].setGreen(g);
  colors_[i1].setBlue (b);

  int num_colors = colors_.size();

  for (int i = i2 + 1; i < num_colors; ++i)
    colors_[i - 1] = colors_[i];

  colors_.resize(num_colors - 1);

  int size = size_.area();

  for (int i = 0; i < size; ++i) {
    int pixel = getData(i);

    if      (pixel == i2)
      setData(i, i1);
    else if (pixel > i2)
      setData(i, pixel - 1);
  }
}

void
CImage::
gray()
{
  CRGBA rgba;

  if (hasColormap()) {
    int num_colors = colors_.size();

    for (int i = 0; i < num_colors; ++i)
      colors_[i].toGray();
  }
  else {
    CRGBA rgba;

    int x1, y1, x2, y2;

    getWindow(&x1, &y1, &x2, &y2);

    for (int y = y1; y <= y2; ++y) {
      for (int x = x1; x <= x2; ++x) {
        getRGBAPixel(x, y, rgba);

        rgba.toGray();

        setRGBAPixel(x, y, rgba);
      }
    }
  }
}

void
CImage::
sepia()
{
  CRGBA rgba;

  if (hasColormap()) {
    int num_colors = colors_.size();

    for (int i = 0; i < num_colors; ++i)
      colors_[i].toSepia();
  }
  else {
    CRGBA rgba;

    int x1, y1, x2, y2;

    getWindow(&x1, &y1, &x2, &y2);

    for (int y = y1; y <= y2; ++y) {
      for (int x = x1; x <= x2; ++x) {
        getRGBAPixel(x, y, rgba);

        rgba.toSepia();

        setRGBAPixel(x, y, rgba);
      }
    }
  }
}

void
CImage::
monochrome()
{
  twoColor(CRGBA(1,1,1), CRGBA(0,0,0));
}

void
CImage::
twoColor(const CRGBA &bg, const CRGBA &fg)
{
  CRGBA rgba;

  if (hasColormap()) {
    CRGBA rgba;

    int x1, y1, x2, y2;

    getWindow(&x1, &y1, &x2, &y2);

    for (int y = y1; y <= y2; ++y) {
      for (int x = x1; x <= x2; ++x) {
        int ind = getColorIndexPixel(x, y);

        if (colors_[ind].getAlpha() < 0.5 ||
            colors_[ind].getGray () > 0.5)
          setColorIndexPixel(x, y, 0);
        else
          setColorIndexPixel(x, y, 1);
      }
    }

    deleteColors();

    addColor(bg);
    addColor(fg);
  }
  else {
    CRGBA rgba;

    int x1, y1, x2, y2;

    getWindow(&x1, &y1, &x2, &y2);

    for (int y = y1; y <= y2; ++y) {
      for (int x = x1; x <= x2; ++x) {
        getRGBAPixel(x, y, rgba);

        if (rgba.getAlpha() < 0.5 || rgba.getGray () > 0.5)
          setRGBAPixel(x, y, bg);
        else
          setRGBAPixel(x, y, fg);
      }
    }
  }
}

void
CImage::
applyColorMatrix(const std::vector<double> &m)
{
  assert(m.size() == 20);

  convertToRGB();

  int x1, y1, x2, y2;

  getWindow(&x1, &y1, &x2, &y2);

  for (int y = y1; y <= y2; ++y) {
    for (int x = x1; x <= x2; ++x) {
      CRGBA rgba;

      getRGBAPixel(x, y, rgba);

      double r, g, b, a;

      rgba.getRGBA(&r, &g, &b, &a);

      double a1 = m[15]*r + m[16]*g + m[17]*b + m[18]*a + m[19];

      if (a1 > 0) {
        double r1 = m[ 0]*r + m[ 1]*g + m[ 2]*b + m[ 3]*a + m[ 4];
        double g1 = m[ 5]*r + m[ 6]*g + m[ 7]*b + m[ 8]*a + m[ 9];
        double b1 = m[10]*r + m[11]*g + m[12]*b + m[13]*a + m[14];

        CRGBA rgba1(r1, g1, b1, a1);

        setRGBAPixel(x, y, rgba1);
      }
      else
        clearRGBAPixel(x, y);
    }
  }
}

void
CImage::
rotateHue(double dh)
{
  convertToRGB();

  int x1, y1, x2, y2;

  getWindow(&x1, &y1, &x2, &y2);

  for (int y = y1; y <= y2; ++y) {
    for (int x = x1; x <= x2; ++x) {
      CRGBA rgba;

      getRGBAPixel(x, y, rgba);

      CHSV hsv = CRGBUtil::RGBtoHSV(rgba.getRGB());

      double h = hsv.getHue();

      h += dh;

      while (h <  0    ) h += 360.0;
      while (h >= 360.0) h -= 360.0;

      hsv.setHue(h);

      CRGB rgb = CRGBUtil::HSVtoRGB(hsv);

      setRGBAPixel(x, y, CRGBA(rgb, rgba.getAlpha()));
    }
  }
}

void
CImage::
saturate(double ds)
{
  convertToRGB();

  int x1, y1, x2, y2;

  getWindow(&x1, &y1, &x2, &y2);

  for (int y = y1; y <= y2; ++y) {
    for (int x = x1; x <= x2; ++x) {
      CRGBA rgba;

      getRGBAPixel(x, y, rgba);

      CHSV hsv = CRGBUtil::RGBtoHSV(rgba.getRGB());

      double s = hsv.getSaturation();

      s *= ds;

      hsv.setSaturation(s);

      CRGB rgb = CRGBUtil::HSVtoRGB(hsv);

      setRGBAPixel(x, y, CRGBA(rgb, rgba.getAlpha()));
    }
  }
}

void
CImage::
luminanceToAlpha()
{
  convertToRGB();

  int x1, y1, x2, y2;

  getWindow(&x1, &y1, &x2, &y2);

  for (int y = y1; y <= y2; ++y) {
    for (int x = x1; x <= x2; ++x) {
      CRGBA rgba;

      getRGBAPixel(x, y, rgba);

      //CHSV hsv = CRGBUtil::RGBtoHSV(rgba.getRGB()();
      //double a1 = hsv.getValue();
      //double a1 = rgba.getGray();
      double a = 0.2125*rgba.getRed() + 0.7154*rgba.getGreen() + 0.0721*rgba.getBlue();
      double a1 = a*rgba.getAlpha();

      //rgba.setAlpha(a1);
      //setRGBAPixel(x, y, rgba);

      CRGBA rgba1(a1, a1, a1, a1);
      setRGBAPixel(x, y, rgba1);
    }
  }
}

void
CImage::
linearFunc(CRGBAComponent component, double scale, double offset)
{
  convertToRGB();

  int x1, y1, x2, y2;

  getWindow(&x1, &y1, &x2, &y2);

  for (int y = y1; y <= y2; ++y) {
    for (int x = x1; x <= x2; ++x) {
      CRGBA rgba;

      getRGBAPixel(x, y, rgba);

      double value = rgba.getComponent(component);

      value = value*scale + offset;

      rgba.setComponent(component, value);

      setRGBAPixel(x, y, rgba.clamp());
    }
  }
}

void
CImage::
gammaFunc(CRGBAComponent component, double amplitude, double exponent, double offset)
{
  convertToRGB();

  int x1, y1, x2, y2;

  getWindow(&x1, &y1, &x2, &y2);

  for (int y = y1; y <= y2; ++y) {
    for (int x = x1; x <= x2; ++x) {
      CRGBA rgba;

      getRGBAPixel(x, y, rgba);

      double value = rgba.getComponent(component);

      value = amplitude*pow(value, exponent) + offset,

      rgba.setComponent(component, value);

      setRGBAPixel(x, y, rgba.clamp());
    }
  }
}

void
CImage::
tableFunc(CRGBAComponent component, const std::vector<double> &values)
{
  int num_ranges = values.size() - 1;

  if (num_ranges < 1) return;

  double delta = 1.0/num_ranges;

  convertToRGB();

  int x1, y1, x2, y2;

  getWindow(&x1, &y1, &x2, &y2);

  for (int y = y1; y <= y2; ++y) {
    for (int x = x1; x <= x2; ++x) {
      // get component color value
      CRGBA rgba;

      getRGBAPixel(x, y, rgba);

      double value = rgba.getComponent(component);

      // get associated range index
      int    i = 0;
      double value1, value2;

      for ( ; i < num_ranges; ++i) {
        value1 = i*delta;
        value2 = value1 + delta;

        if (value >= value1 && value < value2) break;
      }

      if (i >= num_ranges) continue;

      // remap to new range
      double m = (values[i + 1] - values[i])/(value2 - value1);

      value = (value - value1)*m + values[i];

      // update color
      rgba.setComponent(component, value);

      setRGBAPixel(x, y, rgba.clamp());
    }
  }
}

void
CImage::
discreteFunc(CRGBAComponent component, const std::vector<double> &values)
{
  uint num_ranges = values.size();

  if (num_ranges < 1) return;

  double delta = 1.0/num_ranges;

  convertToRGB();

  int x1, y1, x2, y2;

  getWindow(&x1, &y1, &x2, &y2);

  for (int y = y1; y <= y2; ++y) {
    for (int x = x1; x <= x2; ++x) {
      // get component color value
      CRGBA rgba;

      getRGBAPixel(x, y, rgba);

      double value = rgba.getComponent(component);

      // get associated range index
      uint i = 0;

      for ( ; i < num_ranges; ++i) {
        double value1 = i*delta;
        double value2 = value1 + delta;

        if (value >= value1 && value < value2) break;
      }

      if (i >= num_ranges) continue;

      // set to new value
      value = values[i];

      // update color
      rgba.setComponent(component, value);

      setRGBAPixel(x, y, rgba.clamp());
    }
  }
}

CImagePtr
CImage::
erode(int r, bool isAlpha) const
{
  int r1 = 2*r + 1;
  int r2 = r1*r1;

  std::vector<int> mask;

  mask.resize(r2);

  for (int i = 0, iy = -r; iy <= r; ++iy) {
    for (int ix = -r; ix <= r; ++ix, ++i) {
      double r1 = hypot(ix, iy);

      mask[i] = (r1 <= r ? 1 : 0);
    }
  }

  return erode(mask, isAlpha);
}

CImagePtr
CImage::
erode(const std::vector<int> &mask, bool isAlpha) const
{
  return erodeDilate(mask, isAlpha, true);
}

CImagePtr
CImage::
dilate(int r, bool isAlpha) const
{
  int r1 = 2*r + 1;
  int r2 = r1*r1;

  std::vector<int> mask;

  mask.resize(r2);

  for (int i = 0, iy = -r; iy <= r; ++iy) {
    for (int ix = -r; ix <= r; ++ix, ++i) {
      double r1 = hypot(ix, iy);

      mask[i] = (r1 <= r ? 1 : 0);
    }
  }

  return dilate(mask, isAlpha);
}

CImagePtr
CImage::
dilate(const std::vector<int> &mask, bool isAlpha) const
{
  return erodeDilate(mask, isAlpha, false);
}

CImagePtr
CImage::
erodeDilate(const std::vector<int> &mask, bool isAlpha, bool isErode) const
{
  int r = (sqrt(mask.size()) - 1)/2;

  // count mask bits
  int num_hits = 0;

  for (const auto &m : mask)
    num_hits += m;

  //---

  int width  = getWidth ();
  int height = getHeight();

  CImagePtr image = CImageMgrInst->createImage();

  image->setDataSize(width, height);

  int x1, y1, x2, y2;

  getWindow(&x1, &y1, &x2, &y2);

  for (int y = y1; y <= y2; ++y) {
    bool yinside = (y >= y1 + r && y <= y2 - r);

    for (int x = x1; x <= x2; ++x) {
      bool xinside = (x >= x1 + r && x <= x2 - r);

      bool isSet = (xinside && yinside);

      CRGBA rgba;

      if (isSet) {
        // count mask hits
        int hits = 0;

        for (int i = 0, ix = -r; ix <= r; ++ix)
          for (int iy = -r; iy <= r; ++iy, ++i)
            if (mask[i] && isErodePixel(x + ix, y + iy, isAlpha, rgba))
              ++hits;

        if (isErode)
          isSet = (hits == num_hits);
        else
          isSet = (hits > 0);
      }

      int x2 = x - x1;
      int y2 = y - y1;

      if (isAlpha) {
        if (isSet) {
          rgba /= num_hits;

          image->setRGBAPixel(x2, y2, rgba);
        }
        else
          image->setRGBAPixel(x2, y2, CRGBA(0, 0, 0, 0));
      }
      else {
        if (isSet)
          image->setRGBAPixel(x2, y2, CRGBA(1, 1, 1));
        else
          image->setRGBAPixel(x2, y2, CRGBA(0, 0, 0));
      }
    }
  }

  return image;
}

bool
CImage::
isErodePixel(int x, int y, bool isAlpha, CRGBA &rgba) const
{
  CRGBA rgba1;

  getRGBAPixel(x, y, rgba1);

  if (isAlpha) {
    if (rgba1.getAlpha() > 0.5) {
      rgba += rgba1;

      return true;
    }
  }
  else {
    if (rgba1.getGray() > 0.5)
      return true;
  }

  return false;
}
