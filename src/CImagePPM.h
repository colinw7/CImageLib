#ifndef CIMAGE_PPM_H
#define CIMAGE_PPM_H

#include <CImageFmt.h>

#define CImagePPMInst CImagePPM::getInstance()

class CImagePPM : public CImageFmt {
 public:
  static CImagePPM *getInstance() {
    static CImagePPM *instance;

    if (! instance)
      instance = new CImagePPM;

    return instance;
  }

  bool read(CFile *file, CImagePtr &image);
  bool readHeader(CFile *file, CImagePtr &image);

  bool write(CFile *file, CImagePtr image);

 private:
  CImagePPM() :
   CImageFmt(CFILE_TYPE_IMAGE_PPM) {
  }

 ~CImagePPM() { }

  CImagePPM(const CImagePPM &ppm);

  const CImagePPM &operator=(const CImagePPM &ppm);

 private:
  bool readV3(CFile *file, CImagePtr &image);
  bool readV6(CFile *file, CImagePtr &image);
};

#endif
