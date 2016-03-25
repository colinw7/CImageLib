#ifndef CIMAGE_JPG_H
#define CIMAGE_JPG_H

#include <CImageFmt.h>

#define CImageJPGInst CImageJPG::getInstance()

class CImageJPG : public CImageFmt {
 public:
  static CImageJPG *getInstance() {
    static CImageJPG *instance;

    if (! instance)
      instance = new CImageJPG;

    return instance;
  }

  bool read(CFile *file, CImagePtr &image);
  bool readHeader(CFile *file, CImagePtr &image);

  bool write(CFile *file, CImagePtr image);

 private:
  CImageJPG() :
   CImageFmt(CFILE_TYPE_IMAGE_JPG) {
  }

 ~CImageJPG() { }

  CImageJPG(const CImageJPG &jpg);

  const CImageJPG &operator=(const CImageJPG &jpg);

 private:
#ifdef IMAGE_JPEG
  int jpgProcessMarker(struct jpeg_decompress_struct *);
  int jpgGetC(struct jpeg_decompress_struct *);

  static void jpgErrorProc(struct jpeg_common_struct *);
  static void jpgMessageProc(struct jpeg_common_struct *);
#endif

 private:
  static std::vector<char> errorBuffer_;
};

#endif
