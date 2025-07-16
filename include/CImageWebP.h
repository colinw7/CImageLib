#ifndef CIMAGE_WebP_H
#define CIMAGE_WebP_H

#include <CImageFmt.h>

#define CImageWebPInst CImageWebP::getInstance()

class CImageWebP : public CImageFmt {
 public:
  static CImageWebP *getInstance() {
    static CImageWebP *instance;

    if (! instance)
      instance = new CImageWebP;

    return instance;
  }

  bool read(CFile *file, CImagePtr &image) override;
  bool readHeader(CFile *file, CImagePtr &image) override;

  bool write(CFile *file, CImagePtr image) override;

 private:
  CImageWebP() :
   CImageFmt(CFILE_TYPE_IMAGE_WEBP) {
  }

 ~CImageWebP() { }

  CImageWebP(const CImageWebP &webp);

  CImageWebP &operator=(const CImageWebP &webp);
};

#endif
