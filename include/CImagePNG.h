#ifndef CIMAGE_PNG_H
#define CIMAGE_PNG_H

#include <CImageFmt.h>

#define CImagePNGInst CImagePNG::getInstance()

class CImagePNG : public CImageFmt {
 public:
  static CImagePNG *getInstance() {
    static CImagePNG *instance;

    if (! instance)
      instance = new CImagePNG;

    return instance;
  }

  bool read(CFile *file, CImagePtr &image);
  bool readHeader(CFile *file, CImagePtr &image);

  bool write(CFile *file, CImagePtr image);

 private:
  CImagePNG() :
   CImageFmt(CFILE_TYPE_IMAGE_PNG) {
  }

 ~CImagePNG() { }

  CImagePNG(const CImagePNG &png);

  const CImagePNG &operator=(const CImagePNG &png);
};

#endif
