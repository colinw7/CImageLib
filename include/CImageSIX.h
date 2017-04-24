#ifndef CIMAGE_SIX_H
#define CIMAGE_SIX_H

#include <CImageFmt.h>

#define CImageSIXInst CImageSIX::getInstance()

class CImageSIX : public CImageFmt {
 public:
  static CImageSIX *getInstance() {
    static CImageSIX *instance;

    if (! instance)
      instance = new CImageSIX;

    return instance;
  }

  bool read(CFile *file, CImagePtr &image);

  bool readHeader(CFile *file, CImagePtr &image);

  bool write(CFile *file, CImagePtr image);

 private:
  CImageSIX() :
   CImageFmt(CFILE_TYPE_IMAGE_SIX) {
  }

 ~CImageSIX() { }

  CImageSIX(const CImageSIX &six);

  const CImageSIX &operator=(const CImageSIX &six);
};

#endif
