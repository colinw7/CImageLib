#ifndef CIMAGE_H
#define CIMAGE_H

#include <CRGBA.h>
#include <CRGB.h>
#include <CRGBACombineDef.h>
#include <CAlignType.h>
#include <CISize2D.h>
#include <CIBBox2D.h>
#include <CFileType.h>
#include <COptVal.h>
#include <CImagePtr.h>

#include <cstddef>
#include <vector>

typedef unsigned char uchar;

//--------------------

typedef uint   CIMAGE_INT32;
typedef ushort CIMAGE_INT16;
typedef uchar  CIMAGE_INT8;

enum CImageResizeType {
  CIMAGE_RESIZE_NEAREST,
  CIMAGE_RESIZE_AVERAGE,
  CIMAGE_RESIZE_BILINEAR
};

enum CImageCopyType {
  CIMAGE_COPY_ALL              = 0,
  CIMAGE_COPY_SKIP_TRANSPARENT = (1<<0)
};

class CFile;
class CLinearGradient;
class CRadialGradient;
class CImageAnim;
class CImage;
class CImagePixelIterator;

//--------------------

struct CImageTile {
  CImageTile() { }

  CImageTile(CHAlignType halign1, CVAlignType valign1) :
   halign(halign1), valign(valign1) {
  }

  CHAlignType halign = CHALIGN_TYPE_CENTER;
  CVAlignType valign = CVALIGN_TYPE_CENTER;
};

struct CImageConvolveData {
  // TODO: edge mode

  typedef std::vector<double> Kernel;

  CImageConvolveData() { }

  Kernel kernel;
  int    xsize         = -1;
  int    ysize         = -1;
  double divisor       = -1;
  double bias          = -1;
  bool   preserveAlpha = false;
};

struct CImageDiffData {
  CRGBA  bg        { 0, 0, 0, 0};
  CRGBA  fg        { 0, 0, 0, 0};
  bool   grayScale { true };
  double diff      { 0.0 };
};

//--------------------

class CImage {
 private:
  class Border {
   private:
    int left_;
    int bottom_;
    int right_;
    int top_;

   public:
    Border(int left=0, int bottom=0, int right=0, int top=0) :
     left_(left), bottom_(bottom), right_(right), top_(top) {
    }

    void set(int left, int bottom, int right, int top) {
      left_ = left; bottom_ = bottom; right_ = right; top_ = top;
    }

    void get(int *left, int *bottom, int *right, int *top) {
      *left = left_; *bottom = bottom_; *right = right_; *top = top_;
    }

    int getLeft  () const { return left_  ; }
    int getBottom() const { return bottom_; }
    int getRight () const { return right_ ; }
    int getTop   () const { return top_   ; }
  };

  typedef std::vector<CRGBA>     ColorList;
  typedef std::vector<CImagePtr> ImagePtrList;

  static bool            combine_enabled_;
  static CRGBACombineDef combine_def_;

  CFileType  type_ { CFILE_TYPE_NONE };
  CISize2D   size_ { 0, 0 };
  Border     border_;
  uint      *data_ { nullptr };
  ColorList  colors_;
  CIBBox2D   window_;
  CRGBA      bg_   { 0, 0, 0 };

  friend class CImageMgr;

 public:
  class PixelIterator {
   public:
    typedef std::forward_iterator_tag iterator_category;
    typedef uint                      value_type;
    typedef ptrdiff_t                 difference_type;
    typedef uint *                    pointer;
    typedef uint &                    reference;

   private:
    struct current {
      const CImage *image;
      uint          pos1, pos2;
      uint          width;

      current() :
       image(0), pos1(0), pos2(0), width(0) {
      }

      void inc() {
        ++pos1;
      }
    };

    current cur_;

   public:
    PixelIterator() :
     cur_() {
    }

    PixelIterator(const CImage *image, bool is_end) :
     cur_() {
      cur_.image = image;

      cur_.pos2  = cur_.image->size_.area();
      cur_.width = cur_.image->size_.width;

      if (! is_end)
        cur_.pos1 = 0;
      else
        cur_.pos1 = cur_.pos2;
    }

    uint *operator->() {
      const_cast<CImage *>(cur_.image)->updateData();

      return &cur_.image->data_[cur_.pos1];
    }

    const uint *operator->() const {
      const_cast<CImage *>(cur_.image)->updateData();

      return &cur_.image->data_[cur_.pos1];
    }

    uint *operator* () {
      const_cast<CImage *>(cur_.image)->updateData();

      return &cur_.image->data_[cur_.pos1];
    }

    const uint *operator* () const {
      const_cast<CImage *>(cur_.image)->updateData();

      return &cur_.image->data_[cur_.pos1];
    }

    PixelIterator &operator++() {
      cur_.inc();

      return *this;
    }

    PixelIterator operator++(int) {
      PixelIterator t = *this;

      cur_.inc();

      return t;
    }

    void getRGBPixel(CRGB &rgb) const {
      return cur_.image->getRGBPixel(cur_.pos1, rgb);
    }

    bool operator==(const PixelIterator &i) const {
      return (cur_.pos1 == i.cur_.pos2);
    }

    bool operator!=(const PixelIterator &i) const {
      return (cur_.pos1 != i.cur_.pos2);
    }
  };

  // Combine Mode
 public:
  static void setCombineEnabled(bool enabled) {
    combine_enabled_ = enabled;
  }

  static void setCombineSrcMode(CRGBACombineMode mode) {
    combine_def_.src_mode = mode;
  }

  static CRGBACombineMode getCombineSrcMode() {
    return combine_def_.src_mode;
  }

  static void setCombineDstMode(CRGBACombineMode mode) {
    combine_def_.dst_mode = mode;
  }

  static CRGBACombineMode getCombineDstMode() {
    return combine_def_.dst_mode;
  }

  static void setCombineFunc(CRGBACombineFunc combine_func) {
    combine_def_.func = combine_func;
  }

  static CRGBACombineFunc getCombineFunc() {
    return combine_def_.func;
  }

  static void setCombineFactor(const CRGBA &rgba) {
    combine_def_.factor = rgba;
  }

  static CRGBA &getCombineFactor() {
    return combine_def_.factor;
  }

  // Create
 protected:
  CImage();
  explicit CImage(const CISize2D &size);
  CImage(int width, int height);
  CImage(const CImage &image, int x, int y, int width, int height);

 public:
  CImage(const CImage &image);

  virtual ~CImage();

 protected:
  CImage &operator=(const CImage &image);

 public:
  bool isValid() const {
    return (size_.area() > 0);
  }

  virtual CImagePtr dup() const;

  void replace(CImagePtr image);

  //----

  // Get/Set Data
 public:
  CFileType getType() const { return type_; }
  void      setType(CFileType type) { type_ = type; }

  std::string getTypeName() const;

  void setSize(const CISize2D &size);
  void setSize(int width, int height);

  void setDataSize(const CISize2D &size);
  void setDataSize(int width, int height);

  virtual void setDataSizeV(int width, int height);

  uint getWidth () const { return size_.width ; }
  uint getHeight() const { return size_.height; }

  void getSize(CISize2D &size) const {
    size = size_;
  }

  void getSize(int *width, int *height) const {
    *width  = size_.width;
    *height = size_.height;
  }

  CISize2D getSize() const {
    return size_;
  }

  void setBorder(int  left, int  bottom, int  right, int  top);
  void getBorder(int *left, int *bottom, int *right, int *top);

  int getLeft  () const { return border_.getLeft  (); }
  int getBottom() const { return border_.getBottom(); }
  int getRight () const { return border_.getRight (); }
  int getTop   () const { return border_.getTop   (); }

  void setWindow(const CIBBox2D &bbox);
  void setWindow(int  left, int  bottom, int  right, int  top);

  bool getWindow(int *left, int *bottom, int *right, int *top) const;

  void resetWindow();

  void setBackground(const CRGBA &bg) { bg_ = bg; }

  const CRGBA &getBackground() const { return bg_; }

  //----

  // Read/Write
 public:
  bool read(const std::string &filename, CFileType type=CFILE_TYPE_NONE);

  bool read(const uchar *data, size_t len, CFileType type=CFILE_TYPE_NONE);

  bool read(CFile *file);
  bool read(CFile *file, CFileType type);

  bool readHeader(const std::string &filename);
  bool readHeader(const uchar *data, size_t len);

  bool readHeader(CFile *file);
  bool readHeader(CFile *file, CFileType type);

  static CImagePtr create(const std::string &filename);
  static CImagePtr create(const std::string &filename, CFileType type);

  static CImagePtr create(const char *filename);
  static CImagePtr create(const char *filename, CFileType type);

  static CImagePtr create(const uchar *data, size_t len);

  static CImagePtr create(CFile *file);
  static CImagePtr create(CFile *file, CFileType type);

  static CImagePtr create(const char **strings, uint num_strings, CFileType type);

  static CImagePtr createHeader(const std::string &filename);
  static CImagePtr createHeader(const std::string &filename, CFileType type);

  static CImagePtr createHeader(CFile *file);
  static CImagePtr createHeader(CFile *file, CFileType type);

 public:
  bool write(const std::string &filename);
  bool write(const std::string &filename, CFileType type);

  bool write(const char *filename);
  bool write(const char *filename, CFileType type);

  bool write(CFile *file);
  bool write(CFile *file, CFileType type);

  //----

  // Functions
 public:
  void clamp(int *x, int *y) {
    *x = std::min(std::max(0, *x), size_.width  - 1);
    *y = std::min(std::max(0, *y), size_.height - 1);
  }

  bool validPixel(const CIPoint2D &point) const;
  bool validPixel(int x, int y) const;
  bool validPixel(int pos) const;

  void setColorIndexData(uchar *data);
  void setColorIndexData(uint *data);
  void setColorIndexData(uint pixel);
  void setColorIndexData(uint pixel, int left, int bottom, int right, int top);

  int getColorIndexPixel(int x, int y) const;
  int getColorIndexPixel(int pos) const;

  virtual bool setColorIndexPixel(int x, int y, uint pixel);
  virtual bool setColorIndexPixel(int pos, uint pixel);

  virtual void setRGBAData(uint *data);

  virtual void setRGBAData(const CRGBA &rgba);
  virtual void setRGBAData(const CRGBA &rgba, int left, int bottom, int right, int top);

  bool setRGBAPixel(int x, int y, double r, double g, double b, double a=1.0);
  bool setRGBAPixel(int pos, double r, double g, double b, double a=1.0);

  bool setRGBAPixel(const CIPoint2D &point, const CRGBA &rgba);

  virtual bool setRGBAPixel(int x, int y, const CRGBA &rgba);
  virtual bool setRGBAPixel(int pos, const CRGBA &rgba);

  virtual bool clearRGBAPixel(int x, int y);
  virtual bool clearRGBAPixel(int pos);

  bool setGrayPixel(int x, int y, double gray);
  bool setGrayPixel(int pos, double gray);

  void getRGBAPixel(int x, int y, double *r, double *g, double *b, double *a) const;
  void getRGBAPixel(int ind, double *r, double *g, double *b, double *a) const;
  void getRGBAPixel(const CIPoint2D &point, CRGBA &rgba) const;
  void getRGBAPixel(int x, int y, CRGBA &rgba) const;
  void getRGBAPixel(int ind, CRGBA &rgba) const;

  void getRGBAPixelI(int ind, uint *r, uint *g, uint *b, uint *a) const;
  void getRGBAPixelI(int x, int y, uint *r, uint *g, uint *b, uint *a) const;

  void getRGBPixel(int ind, CRGB &rgb) const;
  void getRGBPixel(int x, int y, CRGB &rgb) const;

  void getGrayPixel(int ind, double *gray) const;
  void getGrayPixel(int x, int y, double *gray) const;
  void getGrayPixelI(int ind, uint *gray) const;
  void getGrayPixelI(int x, int y, uint *gray) const;

  bool hasColormap() const;

  int addColor(double r, double g, double b, double a=1.0);
  int addColorI(uint r, uint g, uint b, uint a=255);
  int addColor(const CRGB &rgb);
  int addColor(const CRGBA &rgba);

  CRGBA getColor(uint pixel) const;
  void  getColor(uint pixel, CRGBA &rgba) const;
  void  setColor(uint pixel, const CRGBA &rgba);

  void getColorRGBA(uint pixel, double *r, double *g, double *b, double *a) const;
  void getColorRGBA(uint pixel, CRGBA &rgba) const;

  void getColorRGBAI(uint pixel, uint *r, uint *g, uint *b, uint *a) const;

  void getColorRGB(uint pixel, CRGB &rgb) const;

  void getColorGray(uint pixel, double *gray) const;

  int getNumColors() const;

  void setTransparentColor(const CRGBA &rgba);
  void setTransparentColor(uint pixel);

  int  getTransparentColor() const;

  bool isTransparentColor(uint pixel) const;

  bool isTransparent(COptReal tol=COptReal(0.1)) const;
  bool isTransparent(int x, int y, COptReal tol=COptReal(0.1)) const;
  bool isTransparent(int pos, COptReal tol=COptReal(0.1)) const;

  bool isTransparentI(COptInt tol=COptInt(5)) const;
  bool isTransparentI(int x, int y, COptInt tol=COptInt(5)) const;
  bool isTransparentI(int pos, COptInt tol=COptInt(5)) const;

  double getAlpha(int x, int y) const;
  double getAlpha(int pos) const;

  uint getAlphaI(int x, int y) const;
  uint getAlphaI(int pos) const;

  void deleteColors();

  void setAlphaByGray(bool positive=true);
  void setGrayByAlpha(bool positive=true);

  void setAlphaByColor(const CRGB &rgb, double a=1.0);

  void setAlpha();
  void setAlphaGray(double gray);
  void setAlpha(double a);
  void setAlphaByImage(CImagePtr image);

  void scaleAlpha(double a);

  //----

  static uint rgbaToPixel(const CRGBA &rgba);

  static uint rgbaToPixel(double r, double g, double b, double a=1.0);

  static uint grayToPixel(double gray);
  static uint grayToPixelI(uint gray);

  static uint rgbaToPixelI(uint r, uint g, uint b, uint a=255);

  //----

  static void pixelToRGBA(uint pixel, CRGBA &rgba);

  static void pixelToRGBA(uint pixel, double *r, double *g, double *b, double *a);

  static void pixelToGray(uint pixel, double *gray);
  static void pixelToGrayI(uint pixel, uint *gray);

  static void pixelToRGBAI(uint pixel, uint *r, uint *g, uint *b, uint *a);
  static void pixelToRGBI(uint pixel, uint *r, uint *g, uint *b);

  static void pixelToAlpha(uint pixel, double *a);
  static void pixelToAlphaI(uint pixel, uint *a);

  //----

  int findColor(const CRGBA &rgba);

  //----

  uint memUsage() const;

  //----

 public:
  virtual void updateData() { }

  uint *getData() const {
    const_cast<CImage *>(this)->updateData();

    return data_;
  }

  uint getData(int ind) const {
    const_cast<CImage *>(this)->updateData();

    return data_[ind];
  }

  uint getData(int x, int y) const {
    const_cast<CImage *>(this)->updateData();

    return data_[y*size_.width + x];
  }

  void setData(int ind, uint data) {
    updateData();

    data_[ind] = data;
  }

  void setData(int x, int y, uint data) {
    updateData();

    data_[y*size_.width + x] = data;
  }

  uint getDataSize() const {
    return size_.width*size_.height;
  }

  //------

  virtual void dataChanged() { }

  //------

 public:
  typedef PixelIterator pixel_iterator;

  pixel_iterator       pixel_begin();
  const pixel_iterator pixel_begin() const;
  pixel_iterator       pixel_end  ();
  const pixel_iterator pixel_end  () const;

  //------

 public:
  double boxScale(int w, int h) const;

  //------

  // Resize (New Image), Reshape (Existing Image)
 private:
  static CImageResizeType resize_type;

 public:
  static CImageResizeType setResizeType(CImageResizeType type);

  CImagePtr scale(double s) const;
  CImagePtr scale(double xs, double ys) const;

  CImagePtr scaleKeepAspect(double s) const;
  CImagePtr scaleKeepAspect(double xs, double ys) const;

  CImagePtr resize(const CISize2D &size) const;
  CImagePtr resize(int width, int height) const;

  CImagePtr resizeWidth(int width) const;
  CImagePtr resizeHeight(int height) const;

  CImagePtr resizeMax(int size) const;
  CImagePtr resizeMin(int size) const;

  CImagePtr resizeKeepAspect(const CISize2D &size) const;
  CImagePtr resizeKeepAspect(int width, int height) const;

  bool reshape(int width, int height);

  bool reshapeWidth(int width);
  bool reshapeHeight(int height);

  bool reshapeMax(int size);
  bool reshapeMin(int size);

  bool reshapeKeepAspect(const CISize2D &size);
  bool reshapeKeepAspect(int width, int height);

  void sampleNearest(double x, double y, CRGBA &rgb) const;
  void sampleBilinear(double x, double y, CRGBA &rgb) const;

  CRGBA getBilinearRGBAPixel(double xx, double yy) const;

 private:
  static void reshapeNearest(CImagePtr old_image, CImagePtr &new_image);
  void reshapeNearest(CImagePtr &new_image) const;

  static void reshapeAverage(CImagePtr old_image, CImagePtr &new_image);
  void reshapeAverage(CImagePtr &new_image) const;

  static void reshapeBilinear(CImagePtr old_image, CImagePtr &new_image);
  void reshapeBilinear(CImagePtr &new_image) const;

  CRGBA getBilinearRGBAPixel(double xx, int x1, int x2, double yy, int y1, int y2) const;

  //------

  // Rotate
 public:
  CImagePtr rotate(double angle);

  //------

  // Scroll
 public:
  void scroll(int dx, int dy);

  void scrollX(int offset);
  void scrollY(int offset);

  //------

  // Flip
 public:
  CImagePtr flippedH() const;
  CImagePtr flippedV() const;
  CImagePtr flippedHV() const;

  void flipH();
  void flipV();
  void flipHV();

  //------

  // Tile
 public:
  CImagePtr tile(int width, int height, const CImageTile &tile=CImageTile());

  //------

  // Convert
 private:
  static CRGBA  convertBg_;
  static double convertAlphaTol_;

 public:
  enum ConvertMethod {
    CONVERT_NEAREST_LOGICAL,  // replace with nearest RGB of new colors
    CONVERT_NEAREST_PHYSICAL  // replace with nearest pixel of new color
  };

  static const CRGBA &getConvertBg() { return convertBg_; }

  static void setConvertBg(const CRGBA &bg) { convertBg_ = bg; }

  static void setConvertAlphaTol(double tol) {
    assert(tol >= 0.0 && tol <= 1.0);

    convertAlphaTol_ = tol;
  }

  static bool isConvertTransparent(const CRGBA &rgba) {
    return rgba.getAlpha() <= convertAlphaTol_;
  }

  static bool isConvertTransparent(double a) {
    return a <= convertAlphaTol_;
  }

  void convertToNColors(uint ncolors=256, ConvertMethod method=CONVERT_NEAREST_LOGICAL);

  void convertToColorIndex();

  void convertToRGB();

  //------

  // Mask
 public:
  CImagePtr createMask() const;

  void alphaMask(CImagePtr mask, int xo = 0, int yo = 0);
  void alphaMaskRGBA(CImagePtr mask, const CRGBA &rgba, int xo = 0, int yo = 0);

  CImagePtr createRGBAMask(const CRGBA &rgba = CRGBA(0.2125, 0.7154, 0.0721));

  void clipOutside(int x1, int y1, int x2, int y2);

  //------

  // Gray Scale
 public:
  CImagePtr grayScaled() const;

  void grayScale();

  //------

  // Draw
 public:
  void fillRGBARectangle(int x1, int y1, int x2, int y2, const CRGBA &rgba);
  void fillColorIndexRectangle(int x1, int y1, int x2, int y2, int ind);

  void drawColorIndexPoint(int x, int y, int color_ind);
  void drawColorIndexPoint(int i, int color_ind);

  void drawRGBAPoint(int x, int y, const CRGBA &rgba);
  void drawRGBAPoint(int i, const CRGBA &rgba);

  //------

  // Line Art
 public:
  void lineArt(double tolerance);

  //------

  // Combine
 public:
  static bool combine(CImagePtr image1, CImagePtr image2, const CRGBACombineDef &def);

  bool combine(CImagePtr image, const CRGBACombineDef &def);

  bool combine(CImagePtr image, CRGBABlendMode mode);

  static bool combineDef(CImagePtr image1, CImagePtr image2);

  bool combineDef(CImagePtr image);

  static bool combine(CImagePtr image1, CImagePtr image2);

  bool combine(CImagePtr image);

  bool combine(int x, int y, CImagePtr image);

  //------

  // Merge
 public:
  static CImagePtr merge(CImagePtr image1, CImagePtr image2);

  //------

  // Invert
 public:
  CImagePtr inverted() const;

  void invert();

  //------

  // Copy
 private:
  static CImageCopyType copy_type;

 public:
  static CImageCopyType setCopyType(CImageCopyType type);

  CImagePtr subImage(int x=0, int y=0, int width=-1, int height=-1) const;

  void copyFrom(CImagePtr src);
  void copyFrom(CImagePtr src, int dest_x, int dest_y);

  void copyTo(CImagePtr &dst, int dest_x=0, int dest_y=0) const;

  void subCopyFrom(CImagePtr  src, int src_x=0, int src_y=0, int width=-1, int height=-1,
                   int dest_x=0, int dest_y=0);
  void subCopyTo  (CImagePtr &dst, int src_x=0, int src_y=0, int width=-1, int height=-1,
                   int dest_x=0, int dest_y=0) const;

  bool copy(CImagePtr image, int x=0, int y=0);

  static CImagePtr copy(CImagePtr image1, CImagePtr image2, int x=0, int y=0);

  bool copyAlpha(CImagePtr image, int x=0, int y=0);

  //------

  // Process
 public:
  void removeSinglePixels();

  ImagePtrList colorSplit();
  CImagePtr    colorSplit(int i);

  void setNumColors(int num_colors);

  void gray();
  void sepia();
  void monochrome();
  void twoColor(const CRGBA &bg, const CRGBA &fg);

  void applyColorMatrix(const std::vector<double> &m);

  void rotateHue(double dh);

  void saturate(double ds);

  void luminanceToAlpha();

  void linearFunc(CRGBAComponent component, double scale, double offset);

  void gammaFunc(CRGBAComponent component, double amplitude, double exponent, double offset);

  void tableFunc(CRGBAComponent component, const std::vector<double> &values);

  void discreteFunc(CRGBAComponent component, const std::vector<double> &values);

  CImagePtr erode(int r=1, bool isAlpha=false) const;
  CImagePtr erode(const std::vector<int> &mask, bool isAlpha=false) const;

  CImagePtr dilate(int r=1, bool isAlpha=false) const;
  CImagePtr dilate(const std::vector<int> &mask, bool isAlpha=false) const;

 private:
  CImagePtr colorSplitByData(uint data);

  void getClosestColors(int &i1, int &i2);

  void replaceColor(int i1, int i2);

  CImagePtr erodeDilate(const std::vector<int> &mask, bool isAlpha, bool isErode) const;

  bool isErodePixel(int x, int y, bool isAlpha, CRGBA &rgba) const;

  //------

  // Filter
 public:
  static void unsharpMask(CImagePtr src, CImagePtr &dst, double strength=2.0);

  CImagePtr unsharpMask(double strength=2.0);

  void unsharpMask(CImagePtr &dst, double strength=2.0);

  //--

  CImagePtr sobel(bool feldman=false);

  void sobel(CImagePtr &dst, bool feldman=false);

  CImagePtr sobelGradient();

  void sobelGradient(CImagePtr &dst);

  void sobelPixelGradient(int x, int y, int dx, int dy,
                          double &xgray, double &ygray, double &xf, double &yf);

  //--

  static void convolve(CImagePtr src, CImagePtr &dst, const std::vector<double> &kernel);

  CImagePtr convolve(const std::vector<double> &kernel);

  void convolve(CImagePtr &dst, const std::vector<double> &kernel);

  static void convolve(CImagePtr src, CImagePtr &dst, int xsize, int ysize,
                       const std::vector<double> &kernel);

  CImagePtr convolve(int xsize, int ysize, const std::vector<double> &kernel);

  void convolve(CImagePtr &dst, int xsize, int ysize, const std::vector<double> &kernel);

  void convolve(CImagePtr &dst, const CImageConvolveData &convolve);

  //--

  static bool gaussianBlur(CImagePtr src, CImagePtr &dst,
                           double bx=1, double by=1, int nx=0, int ny=0);

  bool gaussianBlur(double bx=1, double by=1, int nx=0, int ny=0);

  bool gaussianBlur(CImagePtr &dst, double bx=1, double by=1, int nx=0, int ny=0);

  virtual bool gaussianBlurExec(CImagePtr &dst, double bx, double by, int nx, int ny);

  //--

  void turbulence(bool fractal, double baseFreq, int numOctaves, int seed);
  void turbulence(bool fractal, double baseFreqX, double baseFreqY, int numOctaves, int seed);

  CImagePtr displacementMap(CImagePtr dispImage, CRGBAComponent xcolor, CRGBAComponent ycolor,
                            double scale);

  void displacementMap(CImagePtr dispImage, CRGBAComponent xcolor, CRGBAComponent ycolor,
                       double scale, CImagePtr dst);

  //------

  // Flood Fill
 public:
  void floodFill(int x, int y, const CRGBA &rgba);
  void floodFill(int x, int y, int pixel);

  void fillLargestRect(int x, int y, const CRGBA &rgba);
  void fillLargestRect(int x, int y, int pixel);

  //------

  // Gradient
 public:
  void linearGradient(const CLinearGradient &gradient);
  void radialGradient(const CRadialGradient &gradient);

  //------

  // Util
 public:
  static CIMAGE_INT32 swapBytes32(CIMAGE_INT32 i);
  static CIMAGE_INT16 swapBytes16(CIMAGE_INT16 i);

  //--------------------------

  // BMP
 protected:
  static CImagePtr createBMP(CFile *file);
  static CImagePtr createBMPHeader(CFile *file);

 public:
  bool readBMP(CFile *file);
  bool readBMPHeader(CFile *file);

  bool writeBMP(CFile *file);

  //------

  // EPS
 protected:
  static CImagePtr createEPS(CFile *file);
  static CImagePtr createEPSHeader(CFile *file);

 public:
  bool readEPS(CFile *file);
  bool readEPSHeader(CFile *file);

  bool writeEPS(CFile *file);

  //------

  // GIF
 protected:
  static CImagePtr createGIF(CFile *file);
  static CImagePtr createGIFHeader(CFile *file);

 public:
  bool readGIF(CFile *file);
  bool readGIFHeader(CFile *file);

  bool writeGIF(CFile *file);

  static CImageAnim *createGIFAnim(CFile *file);

  //------

  // ICO
 protected:
  static CImagePtr createICO(CFile *file);
  static CImagePtr createICOHeader(CFile *file);

 public:
  bool readICO(CFile *file);
  bool readICOHeader(CFile *file);

  bool writeICO(CFile *file);

  //------

  // IFF
 protected:
  static CImagePtr createIFF(CFile *file);
  static CImagePtr createIFFHeader(CFile *file);

 public:
  bool readIFF(CFile *file);
  bool readIFFHeader(CFile *file);

  bool writeIFF(CFile *file);

  //------

  // JPG
 protected:
  static CImagePtr createJPG(CFile *file);
  static CImagePtr createJPGHeader(CFile *file);

 public:
  bool readJPG(CFile *file);
  bool readJPGHeader(CFile *file);

  bool writeJPG(CFile *file);

  //------

  // PCX
 protected:
  static CImagePtr createPCX(CFile *file);
  static CImagePtr createPCXHeader(CFile *file);

 public:
  bool readPCX(CFile *file);
  bool readPCXHeader(CFile *file);

  bool writePCX(CFile *file);

  //------

  // PNG
 protected:
  static CImagePtr createPNG(CFile *file);
  static CImagePtr createPNGHeader(CFile *file);

 public:
  bool readPNG(CFile *file);
  bool readPNGHeader(CFile *file);

  bool writePNG(const std::string &filename);
  bool writePNG(CFile *file);

  //------

  // PPM
 protected:
  static CImagePtr createPPM(CFile *file);
  static CImagePtr createPPMHeader(CFile *file);

 public:
  bool readPPM(CFile *file);
  bool readPPMHeader(CFile *file);

  bool writePPM(CFile *file);

  //------

  // PS
 protected:
  static CImagePtr createPS(CFile *file);
  static CImagePtr createPSHeader(CFile *file);

 public:
  bool readPS(CFile *file);
  bool readPSHeader(CFile *file);

  bool writePS(CFile *file);

  //------

  // PSP
 protected:
  static CImagePtr createPSP(CFile *file);
  static CImagePtr createPSPHeader(CFile *file);

 public:
  bool readPSP(CFile *file);
  bool readPSPHeader(CFile *file);

  bool writePSP(CFile *file);

  //------

  // SGI
 protected:
  static CImagePtr createSGI(CFile *file);
  static CImagePtr createSGIHeader(CFile *file);

 public:
  bool readSGI(CFile *file);
  bool readSGIHeader(CFile *file);

  bool writeSGI(CFile *file);

  //------

  // SIX
 protected:
  static CImagePtr createSIX(CFile *file);
  //static CImagePtr createSIXHeader(CFile *file);

 public:
  bool readSIX(CFile *file);
  //bool readSIXHeader(CFile *file);

  bool writeSIX(CFile *file);

  //------

  // SVG
 protected:
  static CImagePtr createSVG(CFile *file);
  static CImagePtr createSVGHeader(CFile *file);

 public:
  bool readSVG(CFile *file);
  bool readSVGHeader(CFile *file);

  bool writeSVG(CFile *file);

  //------

  // TGA
 protected:
  static CImagePtr createTGA(CFile *file);
  static CImagePtr createTGAHeader(CFile *file);

 public:
  bool readTGA(CFile *file);
  bool readTGAHeader(CFile *file);

  bool writeTGA(CFile *file);

  //------

  // TIF
 protected:
  static CImagePtr createTIF(CFile *file);
  static CImagePtr createTIFHeader(CFile *file);

 public:
  bool readTIF(CFile *file);
  bool readTIFHeader(CFile *file);

  bool writeTIF(CFile *file);

  //------

  // XBM
 protected:
  static CImagePtr createXBM(CFile *file);
  static CImagePtr createXBM(uchar *data, int width, int height);
  static CImagePtr createXBMHeader(CFile *file);

 public:
  bool readXBM(CFile *file);
  bool readXBMHeader(CFile *file);

  bool writeXBM(CFile *file);

  bool readXBM(const uchar *data, int w, int h);

  //------

  // XPM
 public:
  static void setXPMHotSpot(int x, int y);

 protected:
  static CImagePtr createXPM(CFile *file);
  static CImagePtr createXPM(const char **strings, uint num_strings);

  static CImagePtr createXPMHeader(CFile *file);

 public:
  bool readXPM(CFile *file);
  bool readXPMHeader(CFile *file);

  bool writeXPM(CFile *file);

  bool readXPM(const char **strs, uint num_strs);

  //------

  // XWD
 protected:
  static CImagePtr createXWD(CFile *file);
  static CImagePtr createXWDHeader(CFile *file);

 public:
  bool readXWD(CFile *file);
  bool readXWDHeader(CFile *file);

  bool writeXWD(CFile *file);

  //------

  // Diff
 public:
  bool diffValue(const CImagePtr &image, double &d);
  bool diffImage(const CImagePtr &image, CImagePtr &dest, CImageDiffData &diffData);

  //------

  // Misc
  static void errorMsg(const std::string &msg);
  static void warnMsg (const std::string &msg);
  static void debugMsg(const std::string &msg);
  static void infoMsg (const std::string &msg, bool newline=true);
};

#endif
