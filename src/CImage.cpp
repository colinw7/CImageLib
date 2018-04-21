#include <CImageLibI.h>

#include <cstring>

static double rgb_scale = 1.0/255.0;

bool            CImage::combine_enabled_  = true;
CRGBACombineDef CImage::combine_def_;

CImage::
CImage()
{
  CImageMgrInst->addImage(this);
}

CImage::
CImage(int width, int height) :
 size_(width, height)
{
  int size = size_.area();

  if (size > 0) {
    data_ = new uint [size];

    memset(data_, 0, size*sizeof(uint));
  }

  CImageMgrInst->addImage(this);
}

CImage::
CImage(const CISize2D &size) :
 size_(size)
{
  int dsize = size_.area();

  if (dsize > 0) {
    data_ = new uint [dsize];

    memset(data_, 0, dsize*sizeof(uint));
  }

  CImageMgrInst->addImage(this);
}

CImage::
CImage(const CImage &image) :
 type_  (image.type_),
 size_  (image.size_),
 border_(image.border_),
 data_  (nullptr),
 colors_(),
 window_(image.window_),
 bg_    (image.bg_)
{
  int size = size_.area();

  if (size > 0 && image.data_) {
    const_cast<CImage &>(image).updateData();

    data_ = new uint [size];

    memcpy(data_, image.data_, size*sizeof(uint));
  }

  int num_colors = (int) image.colors_.size();

  for (int i = 0; i < num_colors; ++i)
    addColor(image.colors_[i]);

  CImageMgrInst->addImage(this);
}

CImage::
CImage(const CImage &image, int x, int y, int width, int height) :
 size_(width, height)
{
  int size = size_.area();

  if (size > 0)
    data_ = new uint [size];

  int num_colors = (int) image.colors_.size();

  for (int i = 0; i < num_colors; ++i)
    addColor(image.colors_[i]);

  if (x + width  <= 0 || x >= image.size_.width  ||
      y + height <= 0 || y >= image.size_.height) {
    memset(data_, 0, size*sizeof(uint));
  }
  else {
    const_cast<CImage &>(image).updateData();

    uint *ps = &image.data_[0];
    uint *pe = &image.data_[image.size_.area() - 1];

    uint *p1 = &data_[0];

    uint *px;
    uint *py = &image.data_[y*image.size_.width];

    for (int y1 = 0; y1 < height; ++y1) {
      px = py + x;

      for (int x1 = 0; x1 < width; ++x1, ++p1, ++px) {
        if (px >= ps && px <= pe)
          *p1 = *px;
        else
          *p1 = 0;
      }

      py += image.size_.width;
    }
  }

  CImageMgrInst->addImage(this);
}

CImage::
~CImage()
{
  CImageMgrInst->deleteImage(this);

  delete [] data_;

  deleteColors();
}

//-----------

CImage &
CImage::
operator=(const CImage &image)
{
  if (&image == this)
    return *this;

  type_   = image.type_;
  size_   = image.size_;
  border_ = image.border_;
  window_ = image.window_;

  delete [] data_;

  int size = size_.area();

  if (size > 0 && image.data_) {
    const_cast<CImage &>(image).updateData();

    data_ = new uint [size];

    memcpy(data_, image.data_, size*sizeof(uint));
  }
  else
    data_ = 0;

  colors_.clear();

  int num_colors = (int) image.colors_.size();

  for (int i = 0; i < num_colors; ++i)
    addColor(image.colors_[i]);

  dataChanged();

  return *this;
}

void
CImage::
replace(CImagePtr image)
{
  this->operator=(*image);
}

CImagePtr
CImage::
dup() const
{
  CImagePtr pimage = CImageMgrInst->createImage();

  pimage->setDataSize(size_);

  *pimage = *this;

  return pimage;
}

//-----------

std::string
CImage::
getTypeName() const
{
  return CStrUtil::toUpper(CFileUtil::getPrefix(type_));
}

//-----------

void
CImage::
setSize(const CISize2D &size)
{
  size_ = size;
}

void
CImage::
setSize(int width, int height)
{
  size_.set(width, height);
}

void
CImage::
setDataSize(const CISize2D &size)
{
  setDataSizeV(size.width, size.height);
}

void
CImage::
setDataSize(int width, int height)
{
  setDataSizeV(width, height);
}

void
CImage::
setDataSizeV(int width, int height)
{
  if (width != size_.width || height != size_.height) {
    size_ = CISize2D(width, height);

    delete [] data_;

    int size = size_.area();

    if (size > 0) {
      data_ = new uint [size];

      memset(data_, 0, size*sizeof(uint));
    }

    dataChanged();
  }
}

void
CImage::
setBorder(int left, int bottom, int right, int top)
{
  border_.set(left, bottom, right, top);
}

void
CImage::
getBorder(int *left, int *bottom, int *right, int *top)
{
  border_.get(left, bottom, right, top);
}

void
CImage::
setWindow(const CIBBox2D &bbox)
{
  window_.set(bbox);

  dataChanged();
}

void
CImage::
setWindow(int left, int bottom, int right, int top)
{
  window_.set(left, bottom, right, top);

  dataChanged();
}

bool
CImage::
getWindow(int *left, int *bottom, int *right, int *top) const
{
  if (window_.isSet()) {
    window_.get(left, bottom, right, top);

    *left   = std::min(std::max(*left  , 0), size_.width  - 1);
    *bottom = std::min(std::max(*bottom, 0), size_.height - 1);
    *right  = std::min(std::max(*right , 0), size_.width  - 1);
    *top    = std::min(std::max(*top   , 0), size_.height - 1);

    return true;
  }
  else {
    *left   = 0;
    *bottom = 0;
    *right  = size_.width  - 1;
    *top    = size_.height - 1;

    return false;
  }
}

void
CImage::
resetWindow()
{
  window_.reset();
}

//-----------

bool
CImage::
validPixel(const CIPoint2D &point) const
{
  return validPixel(point.x, point.y);
}

bool
CImage::
validPixel(int x, int y) const
{
  return (x >= 0 && x < size_.width && y >= 0 && y < size_.height);
}

bool
CImage::
validPixel(int ind) const
{
  return (ind >= 0 && ind < size_.area());
}

void
CImage::
setColorIndexData(uchar *data)
{
  uchar *p1 = data;
  uint  *p2 = data_;

  for (int y = 0; y < size_.height; ++y)
    for (int x = 0; x < size_.width; ++x, ++p1, ++p2)
      *p2 = *p1;
}

void
CImage::
setColorIndexData(uint *data)
{
  uint *p1 = data;
  uint *p2 = data_;

  for (int y = 0; y < size_.height; ++y)
    for (int x = 0; x < size_.width; ++x, ++p1, ++p2)
      *p2 = *p1;
}

void
CImage::
setColorIndexData(uint pixel)
{
  if (window_.isSet()) {
    int left, bottom, right, top;

    getWindow(&left, &bottom, &right, &top);

    setColorIndexData(pixel, left, bottom, right, top);
  }
  else {
    uint *p1 = data_;

    for (int y = 0; y < size_.height; ++y) {
      for (int x = 0; x < size_.width; ++x, ++p1)
        *p1 = pixel;
    }
  }
}

void
CImage::
setColorIndexData(uint pixel, int left, int bottom, int right, int top)
{
  updateData();

  uint *p1 = data_ + bottom*size_.width;

  for (int y = bottom; y <= top; ++y) {
    uint *p2 = p1 + left;

    for (int x = left; x <= right; ++x, ++p2)
      *p2 = pixel;

    p1 += size_.width;
  }
}

int
CImage::
getColorIndexPixel(int ind) const
{
  if (! CASSERT(ind >= 0 && ind < size_.area(), "Invalid Index"))
    return 0;

  const_cast<CImage *>(this)->updateData();

  uint pixel = data_[ind];

  if (! CASSERT(pixel < colors_.size(), "Invalid Color Ind"))
    return 0;

  return pixel;
}

int
CImage::
getColorIndexPixel(int x, int y) const
{
  return getColorIndexPixel(y*size_.width + x);
}

bool
CImage::
setColorIndexPixel(int ind, uint pixel)
{
  if (! CASSERT(ind >= 0 && ind < size_.area(), "Invalid Index"))
    return false;

  if (! CASSERT(pixel < colors_.size(), "Invalid Color Ind"))
    return false;

  updateData();

  if (data_[ind] == pixel)
    return false;

  data_[ind] = pixel;

  return true;
}

bool
CImage::
setColorIndexPixel(int x, int y, uint pixel)
{
  return setColorIndexPixel(y*size_.width + x, pixel);
}

void
CImage::
setRGBAData(uint *data)
{
  int size = size_.area();

  if (size > 0)
    memcpy(data_, data, size*sizeof(data_[0]));

#if 0
  uint *p1 = data;
  uint *p2 = data_;

  for (int y = 0; y < size_.height; ++y)
    for (int x = 0; x < size_.width; ++x, ++p1, ++p2)
      *p2 = *p1;
#endif

  dataChanged();
}

void
CImage::
setRGBAData(const CRGBA &rgba)
{
  if (window_.isSet()) {
    int left, bottom, right, top;

    getWindow(&left, &bottom, &right, &top);

    setRGBAData(rgba, left, bottom, right, top);
  }
  else {
    uint pixel = rgbaToPixel(rgba);

    uint *p1 = data_;

    for (int y = 0; y < size_.height; ++y) {
      for (int x = 0; x < size_.width; ++x, ++p1)
        *p1 = pixel;
    }
  }

  dataChanged();
}

void
CImage::
setRGBAData(const CRGBA &rgba, int left, int bottom, int right, int top)
{
  updateData();

  uint pixel = rgbaToPixel(rgba);

  uint *p1 = data_ + bottom*size_.width;

  for (int y = bottom; y <= top; ++y) {
    uint *p2 = p1 + left;

    for (int x = left; x <= right; ++x, ++p2)
      *p2 = pixel;

    p1 += size_.width;
  }

  dataChanged();
}

bool
CImage::
setRGBAPixel(int ind, double r, double g, double b, double a)
{
  CRGBA rgba(r, g, b, a);

  return setRGBAPixel(ind, rgba);
}

bool
CImage::
setRGBAPixel(const CIPoint2D &point, const CRGBA &rgba)
{
  return setRGBAPixel(point.x, point.y, rgba);
}

bool
CImage::
setRGBAPixel(int ind, const CRGBA &rgba)
{
  if (! CASSERT(ind >= 0 && ind < size_.area(), "Invalid Index"))
    return false;

  updateData();

  uint pixel = rgbaToPixel(rgba);

  if (data_[ind] == pixel)
    return false;

  data_[ind] = pixel;

  dataChanged();

  return true;
}

bool
CImage::
setRGBAPixel(int x, int y, double r, double g, double b, double a)
{
  return setRGBAPixel(y*size_.width + x, r, g, b, a);
}

bool
CImage::
setRGBAPixel(int x, int y, const CRGBA &rgba)
{
  return setRGBAPixel(y*size_.width + x, rgba);
}

bool
CImage::
clearRGBAPixel(int x, int y)
{
  return clearRGBAPixel(y*size_.width + x);
}

bool
CImage::
clearRGBAPixel(int ind)
{
  if (! CASSERT(ind >= 0 && ind < size_.area(), "Invalid Index"))
    return false;

  updateData();

  if (! data_[ind])
    return false;

  data_[ind] = 0;

  dataChanged();

  return true;
}

bool
CImage::
setGrayPixel(int ind, double gray)
{
  CRGBA rgba(gray, gray, gray, 1.0);

  return setRGBAPixel(ind, rgba);
}

bool
CImage::
setGrayPixel(int x, int y, double gray)
{
  return setGrayPixel(y*size_.width + x, gray);
}

void
CImage::
getRGBAPixel(const CIPoint2D &point, CRGBA &rgba) const
{
  getRGBAPixel(point.x, point.y, rgba);
}

void
CImage::
getRGBAPixel(int ind, double *r, double *g, double *b, double *a) const
{
  if (! CASSERT(ind >= 0 && ind < size_.area(), "Invalid Index")) {
    if (r != 0) *r = 0.0;
    if (g != 0) *g = 0.0;
    if (b != 0) *b = 0.0;
    if (a != 0) *a = 0.0;

    return;
  }

  const_cast<CImage *>(this)->updateData();

  if (hasColormap()) {
    uint pixel = data_[ind];

    if (! CASSERT(pixel < colors_.size(), "Invalid Color Ind")) {
      if (r != 0) *r = 0.0;
      if (g != 0) *g = 0.0;
      if (b != 0) *b = 0.0;
      if (a != 0) *a = 0.0;

      return;
    }

    if (r != 0) *r = colors_[pixel].getRed  ();
    if (g != 0) *g = colors_[pixel].getGreen();
    if (b != 0) *b = colors_[pixel].getBlue ();
    if (a != 0) *a = colors_[pixel].getAlpha();
  }
  else {
    pixelToRGBA(data_[ind], r, g, b, a);
  }
}

void
CImage::
getRGBAPixel(int ind, CRGBA &rgba) const
{
  if (! CASSERT(ind >= 0 && ind < size_.area(), "Invalid Index"))
    return;

  const_cast<CImage *>(this)->updateData();

  if (hasColormap()) {
    uint pixel = data_[ind];

    if (pixel < colors_.size())
      rgba = colors_[pixel];
    else {
      errorMsg("Invalid Color Ind: " + std::to_string(pixel));
      rgba = CRGBA(0,0,0);
    }
  }
  else {
    pixelToRGBA(data_[ind], rgba);
  }
}

void
CImage::
getRGBAPixel(int x, int y, double *r, double *g, double *b, double *a) const
{
  getRGBAPixel(y*size_.width + x, r, g, b, a);
}

void
CImage::
getRGBAPixel(int x, int y, CRGBA &rgba) const
{
  getRGBAPixel(y*size_.width + x, rgba);
}

void
CImage::
getRGBAPixelI(int ind, uint *r, uint *g, uint *b, uint *a) const
{
  if (! CASSERT(ind >= 0 && ind < size_.area(), "Invalid Index")) {
    if (r != 0) *r = 0;
    if (g != 0) *g = 0;
    if (b != 0) *b = 0;
    if (a != 0) *a = 0;

    return;
  }

  const_cast<CImage *>(this)->updateData();

  if (hasColormap()) {
    uint pixel = data_[ind];

    if (! CASSERT(pixel < colors_.size(), "Invalid Color Ind")) {
      if (r != 0) *r = 0;
      if (g != 0) *g = 0;
      if (b != 0) *b = 0;
      if (a != 0) *a = 0;

      return;
    }

    if (r != 0) *r = colors_[pixel].getRedI  ();
    if (g != 0) *g = colors_[pixel].getGreenI();
    if (b != 0) *b = colors_[pixel].getBlueI ();
    if (a != 0) *a = colors_[pixel].getAlphaI();
  }
  else {
    pixelToRGBAI(data_[ind], r, g, b, a);
  }
}

void
CImage::
getRGBPixel(int ind, CRGB &rgb) const
{
  CRGBA rgba;

  getRGBAPixel(ind, rgba);

  rgb = rgba.getRGB();
}

void
CImage::
getRGBPixel(int x, int y, CRGB &rgb) const
{
  getRGBPixel(y*size_.width + x, rgb);
}

void
CImage::
getGrayPixel(int ind, double *gray) const
{
  CRGBA rgba;

  getRGBAPixel(ind, rgba);

  *gray = rgba.getGray();
}

void
CImage::
getGrayPixel(int x, int y, double *gray) const
{
  getGrayPixel(y*size_.width + x, gray);
}

void
CImage::
getGrayPixelI(int ind, uint *gray) const
{
  CRGBA rgba;

  getRGBAPixel(ind, rgba);

  *gray = rgba.getGrayI();
}

void
CImage::
getGrayPixelI(int x, int y, uint *gray) const
{
  getGrayPixelI(y*size_.width + x, gray);
}

bool
CImage::
hasColormap() const
{
  return (! colors_.empty());
}

int
CImage::
addColor(double r, double g, double b, double a)
{
  return addColor(CRGBA(r, g, b, a));
}

int
CImage::
addColor(const CRGB &rgb)
{
  return addColor(CRGBA(rgb));
}

int
CImage::
addColorI(uint r, uint g, uint b, uint a)
{
  CRGBA rgba;

  rgba.setRGBAI(r, g, b, a);

  return addColor(rgba);
}

int
CImage::
addColor(const CRGBA &rgba)
{
  if (colors_.size() >= 256)
    return -1;

  colors_.push_back(rgba);

  return colors_.size() - 1;
}

CRGBA
CImage::
getColor(uint pixel) const
{
  if (! CASSERT(pixel < colors_.size(), "Invalid Color Ind"))
    return CRGBA(0,0,0,0);

  return colors_[pixel];
}

void
CImage::
getColor(uint pixel, CRGBA &rgba) const
{
  if (! CASSERT(pixel < colors_.size(), "Invalid Color Ind"))
    return;

  rgba = colors_[pixel];
}

void
CImage::
setColor(uint pixel, const CRGBA &rgba)
{
  if (! CASSERT(pixel < colors_.size(), "Invalid Color Ind"))
    return;

  colors_[pixel] = rgba;
}

void
CImage::
getColorRGBA(uint pixel, double *r, double *g, double *b, double *a) const
{
  CRGBA rgba;

  getColorRGBA(pixel, rgba);

  rgba.getRGBA(r, g, b, a);
}

void
CImage::
getColorRGBA(uint pixel, CRGBA &rgba) const
{
  if (! CASSERT(pixel < colors_.size(), "Invalid Color Ind"))
    return;

  rgba = colors_[pixel];
}

void
CImage::
getColorRGBAI(uint pixel, uint *r, uint *g, uint *b, uint *a) const
{
  CRGBA rgba;

  getColorRGBA(pixel, rgba);

  rgba.getRGBAI(r, g, b, a);
}

void
CImage::
getColorRGB(uint pixel, CRGB &rgb) const
{
  CRGBA rgba;

  getColorRGBA(pixel, rgba);

  rgb = rgba.getRGB();
}

void
CImage::
getColorGray(uint pixel, double *gray) const
{
  CRGBA rgba;

  getColorRGBA(pixel, rgba);

  *gray = rgba.getGray();
}

int
CImage::
getNumColors() const
{
  if (hasColormap())
    return colors_.size();
  else {
    const_cast<CImage *>(this)->updateData();

    std::map<uint, bool> used;

    int size = size_.area();

    auto p2 = used.end();

    for (int i = 0; i < size; ++i) {
      auto p1 = used.find(data_[i]);

      if (p1 == p2) {
        used[data_[i]] = true;

        p2 = used.end();
      }
    }

    return used.size();
  }
}

void
CImage::
setTransparentColor(uint pixel)
{
  if (! CASSERT(hasColormap(), "Colormap required"))
    return;

  uint num_colors = colors_.size();

  if (! CASSERT(pixel <= num_colors, "Invalid Color Ind"))
    return;

  for (uint i = 0; i < num_colors; ++i) {
    if (i == pixel)
      colors_[i].setAlpha(0.0);
    else
      colors_[i].setAlpha(1.0);
  }
}

void
CImage::
setTransparentColor(const CRGBA &rgba)
{
  if (hasColormap()) {
    uint num_colors = colors_.size();

    for (uint i = 0; i < num_colors; ++i)
      if (colors_[i] == rgba) {
        setTransparentColor(i);
        break;
      }
  }
  else {
    updateData();

    uint pixel = rgbaToPixel(rgba);

    int i = 0;

    for (int y = 0; y < size_.height; ++y) {
      for (int x = 0; x < size_.width; ++x, ++i) {
        double r, g, b, a;

        pixelToRGBA(data_[i], &r, &g, &b, &a);

        uint pixel1 = rgbaToPixel(r, g, b);

        if (pixel == pixel1)
          data_[i] = rgbaToPixel(r, g, b, 0.0);
        else
          data_[i] = rgbaToPixel(r, g, b, 1.0);
      }
    }
  }
}

int
CImage::
getTransparentColor() const
{
  if (! CASSERT(hasColormap(), "Colormap required"))
    return -1;

  int num_colors = colors_.size();

  for (int i = 0; i < num_colors; ++i)
    if (colors_[i].isTransparent())
      return i;

  return -1;
}

bool
CImage::
isTransparentColor(uint ind) const
{
  int i = getTransparentColor();

  return (i >= 0 && i == int(ind));
}

bool
CImage::
isTransparent(COptReal tol) const
{
  int i = 0;

  for (int y = 0; y < size_.height; ++y) {
    for (int x = 0; x < size_.width; ++x, ++i)
      if (isTransparent(i, tol))
        return true;
  }

  return false;
}

bool
CImage::
isTransparent(int x, int y, COptReal tol) const
{
  return isTransparent(y*size_.width + x, tol);
}

bool
CImage::
isTransparent(int pos, COptReal tol) const
{
  return (getAlpha(pos) <= tol);
}

bool
CImage::
isTransparentI(COptInt tol) const
{
  int i = 0;

  for (int y = 0; y < size_.height; ++y) {
    for (int x = 0; x < size_.width; ++x, ++i)
      if (isTransparentI(i, tol))
        return true;
  }

  return false;
}

bool
CImage::
isTransparentI(int x, int y, COptInt tol) const
{
  return isTransparentI(y*size_.width + x, tol);
}

bool
CImage::
isTransparentI(int pos, COptInt tol) const
{
  return (getAlphaI(pos) <= tol);
}

double
CImage::
getAlpha(int x, int y) const
{
  return getAlpha(y*size_.width + x);
}

double
CImage::
getAlpha(int pos) const
{
  const_cast<CImage *>(this)->updateData();

  uint pixel = data_[pos];

  if (hasColormap()) {
    if (! CASSERT(pixel <= colors_.size(), "Invalid Color Ind"))
      return 0.0;

    if (colors_[pixel].isTransparent())
      return 0.0;
    else
      return 1.0;
  }
  else {
    double a;

    pixelToAlpha(pixel, &a);

    return a;
  }
}

uint
CImage::
getAlphaI(int x, int y) const
{
  return getAlphaI(y*size_.width + x);
}

uint
CImage::
getAlphaI(int pos) const
{
  const_cast<CImage *>(this)->updateData();

  uint pixel = data_[pos];

  if (hasColormap()) {
    if (! CASSERT(pixel <= colors_.size(), "Invalid Color Ind"))
      return 0.0;

    if (colors_[pixel].isTransparent())
      return 0;
    else
      return 255;
  }
  else {
    uint a;

    pixelToAlphaI(pixel, &a);

    return a;
  }
}

void
CImage::
deleteColors()
{
  colors_.clear();
}

//-------------

uint
CImage::
rgbaToPixel(const CRGBA &rgba)
{
  double r, g, b, a;

  rgba.getRGBA(&r, &g, &b, &a);

  return rgbaToPixel(r, g, b, a);
}

uint
CImage::
rgbaToPixel(double r, double g, double b, double a)
{
  uint ir = uint(r*255.0);
  uint ig = uint(g*255.0);
  uint ib = uint(b*255.0);
  uint ia = uint(a*255.0);

  return rgbaToPixelI(ir, ig, ib, ia);
}

uint
CImage::
grayToPixel(double gray)
{
  uint igray = uint(gray*255.0);

  return grayToPixelI(igray);
}

uint
CImage::
grayToPixelI(uint igray)
{
  return rgbaToPixelI(igray, igray, igray, 255);
}

uint
CImage::
rgbaToPixelI(uint r, uint g, uint b, uint a)
{
  uint data = ((a & 0xFF) << 24) |
              ((r & 0xFF) << 16) |
              ((g & 0xFF) <<  8) |
              ((b & 0xFF) <<  0);

  return data;
}

//-------------

void
CImage::
pixelToRGBA(uint pixel, CRGBA &rgba)
{
  uint ir, ig, ib, ia;

  pixelToRGBAI(pixel, &ir, &ig, &ib, &ia);

  rgba.setRGBAI(ir, ig, ib, ia);
}

void
CImage::
pixelToRGBA(uint pixel, double *r, double *g, double *b, double *a)
{
  uint ir, ig, ib, ia;

  pixelToRGBAI(pixel, &ir, &ig, &ib, &ia);

  *r = ir*rgb_scale;
  *g = ig*rgb_scale;
  *b = ib*rgb_scale;
  *a = ia*rgb_scale;
}

void
CImage::
pixelToGray(uint pixel, double *gray)
{
  uint ir, ig, ib, ia;

  pixelToRGBAI(pixel, &ir, &ig, &ib, &ia);

  *gray = (ir + ig + ib)*rgb_scale/3.0;
}

void
CImage::
pixelToGrayI(uint pixel, uint *gray)
{
  uint ir, ig, ib, ia;

  pixelToRGBAI(pixel, &ir, &ig, &ib, &ia);

  *gray = (ir + ig + ib)/3;
}

void
CImage::
pixelToRGBAI(uint pixel, uint *r, uint *g, uint *b, uint *a)
{
  *a = (pixel >> 24) & 0xFF;
  *r = (pixel >> 16) & 0xFF;
  *g = (pixel >>  8) & 0xFF;
  *b = (pixel      ) & 0xFF;
}

void
CImage::
pixelToRGBI(uint pixel, uint *r, uint *g, uint *b)
{
  uint a;

  pixelToRGBAI(pixel, r, g, b, &a);

  double ra = a/255.0;

  *r = uint(ra*(*r));
  *g = uint(ra*(*g));
  *b = uint(ra*(*b));
}

void
CImage::
pixelToAlpha(uint pixel, double *a)
{
  uint ia;

  pixelToAlphaI(pixel, &ia);

  *a = ia*rgb_scale;
}

void
CImage::
pixelToAlphaI(uint pixel, uint *a)
{
  *a = (pixel >> 24) & 0xFF;
}

//-------------

void
CImage::
setAlpha()
{
  setAlphaGray(1);
}

void
CImage::
setAlphaGray(double gray)
{
  if (hasColormap())
    convertToRGB();

  updateData();

  int i = 0;

  for (int y = 0; y < size_.height; ++y) {
    for (int x = 0; x < size_.width; ++x, ++i) {
      double r, g, b, a1;

      pixelToRGBA(data_[i], &r, &g, &b, &a1);

      data_[i] = rgbaToPixel(gray, gray, gray, a1);
    }
  }
}

void
CImage::
setAlpha(double a)
{
  if (hasColormap())
    convertToRGB();

  updateData();

  int i = 0;

  for (int y = 0; y < size_.height; ++y) {
    for (int x = 0; x < size_.width; ++x, ++i) {
      double r, g, b, a1;

      pixelToRGBA(data_[i], &r, &g, &b, &a1);

      data_[i] = rgbaToPixel(r, g, b, a);
    }
  }
}

void
CImage::
setAlphaByGray(bool positive)
{
  if (hasColormap()) {
    int num_colors = colors_.size();

    for (int i = 0; i < num_colors; ++i)
      colors_[i].setAlphaByGray(positive);
  }
  else {
    updateData();

    int i = 0;

    for (int y = 0; y < size_.height; ++y) {
      for (int x = 0; x < size_.width; ++x, ++i) {
        double r, g, b, a;

        pixelToRGBA(data_[i], &r, &g, &b, &a);

        pixelToGray(data_[i], &a);

        if (! positive)
          a = 1.0 - a;

        data_[i] = rgbaToPixel(r, g, b, a);
      }
    }
  }
}

void
CImage::
setGrayByAlpha(bool positive)
{
  if (hasColormap()) {
    int num_colors = colors_.size();

    for (int i = 0; i < num_colors; ++i)
      colors_[i].setGrayByAlpha(positive);
  }
  else {
    updateData();

    int i = 0;

    for (int y = 0; y < size_.height; ++y) {
      for (int x = 0; x < size_.width; ++x, ++i) {
        double r, g, b, a;

        pixelToRGBA(data_[i], &r, &g, &b, &a);

        if (! positive)
          a = 1.0 - a;

        data_[i] = rgbaToPixel(a, a, a, 1);
      }
    }
  }
}

void
CImage::
setAlphaByColor(const CRGB &rgb, double a)
{
  if (hasColormap()) {
    int num_colors = colors_.size();

    for (int i = 0; i < num_colors; ++i)
      colors_[i].setAlphaByColor(rgb);
  }
  else {
    updateData();

    CRGBA rgba(rgb);

    rgba.setAlpha(a);

    CRGBA rgba1;

    int i = 0;

    for (int y = 0; y < size_.height; ++y) {
      for (int x = 0; x < size_.width; ++x, ++i) {
        pixelToRGBA(data_[i], rgba1);

        if (rgba == rgba1)
          data_[i] = rgbaToPixel(rgba);
      }
    }
  }
}

void
CImage::
setAlphaByImage(CImagePtr image)
{
  if (hasColormap())
    convertToRGB();

  if (image->hasColormap())
    image->convertToRGB();

  updateData();

  CRGBA rgba1, rgba2;

  int i = 0;

  for (int y = 0; y < image->size_.height; ++y) {
    for (int x = 0; x < image->size_.width; ++x, ++i) {
      if (validPixel(x, y)) {
        pixelToRGBA(image->data_[i], rgba1);

        getRGBAPixel(x, y, rgba2);

        rgba2.setAlpha(rgba1.getAlpha());

        setRGBAPixel(x, y, rgba2);
      }
    }
  }
}

void
CImage::
scaleAlpha(double a)
{
  if (hasColormap()) {
    int num_colors = colors_.size();

    for (int i = 0; i < num_colors; ++i)
      colors_[i].setAlpha(a*colors_[i].getAlpha());
  }
  else {
    updateData();

    int i = 0;

    for (int y = 0; y < size_.height; ++y) {
      for (int x = 0; x < size_.width; ++x, ++i) {
        double r, g, b, a1;

        pixelToRGBA(data_[i], &r, &g, &b, &a1);

        data_[i] = rgbaToPixel(r, g, b, a*a1);
      }
    }
  }
}

//-----------

int
CImage::
findColor(const CRGBA &rgba)
{
  if (! hasColormap())
    return -1;

  int num_colors = colors_.size();

  for (int i = 0; i < num_colors; ++i)
    if (colors_[i] == rgba)
      return i;

  return -1;
}

//-----------

uint
CImage::
memUsage() const
{
  uint mem = 0;

  mem += sizeof(CImage);

  if (data_ != 0)
    mem += size_.area()*sizeof(uint);

  int num_colors = colors_.size();

  for (int i = 0; i < num_colors; ++i)
    mem += sizeof(CRGBA);

  return mem;
}

//-----------

CImage::pixel_iterator
CImage::
pixel_begin()
{
  return pixel_iterator(this, false);
}

const CImage::pixel_iterator
CImage::
pixel_begin() const
{
  return pixel_iterator(this, false);
}

CImage::pixel_iterator
CImage::
pixel_end()
{
  return pixel_iterator(this, true);
}

const CImage::pixel_iterator
CImage::
pixel_end() const
{
  return pixel_iterator(this, true);
}

//-----------

double
CImage::
boxScale(int w, int h) const
{
  int iw = getWidth ();
  int ih = getHeight();

  return std::min((1.0*w)/iw, (1.0*h)/ih);
}

//----------

void
CImage::
errorMsg(const std::string &msg)
{
  std::cerr << "Error: " + msg << std::endl;
}

void
CImage::
warnMsg(const std::string &msg)
{
  std::cerr << "Warning: " + msg << std::endl;
}

void
CImage::
debugMsg(const std::string &msg)
{
  std::cerr << msg << std::endl;
}

void
CImage::
infoMsg(const std::string &msg, bool newline)
{
  std::cerr << msg;

  if (newline)
    std::cerr << std::endl;
}
