#ifndef CIMAGE_XBM_H
#define CIMAGE_XBM_H

#include <CImageFmt.h>

#define CImageXBMInst CImageXBM::getInstance()

class CImageXBM : public CImageFmt {
 public:
  static CImageXBM *getInstance() {
    static CImageXBM *instance;

    if (! instance)
      instance = new CImageXBM;

    return instance;
  }

  bool read(CFile *file, CImagePtr &image);

  bool read(const uchar *data, CImagePtr &image, int width, int height);

  bool readHeader(CFile *file, CImagePtr &image);

  bool write(CFile *file, CImagePtr image);

 private:
  CImageXBM() :
   CImageFmt(CFILE_TYPE_IMAGE_XBM) {
  }

 ~CImageXBM() { }

  CImageXBM(const CImageXBM &xbm);

  const CImageXBM &operator=(const CImageXBM &xbm);

 private:
  bool readBitmap(CFile *file, uint *width, uint *height,
                  uint **data, int *x_hot, int *y_hot);

  uint *expandBitmapData(const uchar *data, int width, int height);

  void skipSpace(char *data, uint *i);
};

#endif
