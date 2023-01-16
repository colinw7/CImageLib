#ifndef CIMAGE_TGA_H
#define CIMAGE_TGA_H

#include <CImageFmt.h>

#define CImageTGAInst CImageTGA::getInstance()

struct TGAHeader;

class CImageTGA : public CImageFmt {
 public:
  static CImageTGA *getInstance() {
    static CImageTGA *instance;

    if (! instance)
      instance = new CImageTGA;

    return instance;
  }

  bool read(CFile *file, CImagePtr &image) override;
  bool readHeader(CFile *file, CImagePtr &image) override;

  bool write(CFile *file, CImagePtr image) override;

 private:
  CImageTGA() :
   CImageFmt(CFILE_TYPE_IMAGE_TGA) {
  }

 ~CImageTGA() { }

  CImageTGA(const CImageTGA &pcx);

  CImageTGA &operator=(const CImageTGA &pcx);

 private:
  bool readHeader(CFile *file, CImagePtr &image, TGAHeader *header);
};

#endif
