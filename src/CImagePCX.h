#ifndef CIMAGE_PCX_H
#define CIMAGE_PCX_H

#include <CImageFmt.h>

#define CImagePCXInst CImagePCX::getInstance()

struct PCXHeader;

class CImagePCX : public CImageFmt {
 public:
  static CImagePCX *getInstance() {
    static CImagePCX *instance;

    if (! instance)
      instance = new CImagePCX;

    return instance;
  }

  bool read(CFile *file, CImagePtr &image);

  bool readHeader(CFile *file, CImagePtr &image);

  bool write(CFile *file, CImagePtr image);

 private:
  CImagePCX() :
   CImageFmt(CFILE_TYPE_IMAGE_PCX) {
  }

 ~CImagePCX() { }

  CImagePCX(const CImagePCX &pcx);

  const CImagePCX &operator=(const CImagePCX &pcx);

 private:
  bool readHeader(CFile *file, CImagePtr &image, PCXHeader *header);
};

#endif
