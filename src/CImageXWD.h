#ifndef CIMAGE_XWD_H
#define CIMAGE_XWD_H

#include <CImageFmt.h>

#define CImageXWDInst CImageXWD::getInstance()

struct XWDFileHeader;

class CImageXWD : public CImageFmt {
 public:
  static CImageXWD *getInstance() {
    static CImageXWD *instance;

    if (! instance)
      instance = new CImageXWD;

    return instance;
  }

  bool read(CFile *file, CImagePtr &image);
  bool readHeader(CFile *file, CImagePtr &image);

  bool write(CFile *file, CImagePtr image);

 private:
  CImageXWD() :
   CImageFmt(CFILE_TYPE_IMAGE_XWD) {
  }

 ~CImageXWD() { }

  CImageXWD(const CImageXWD &xwd);

  const CImageXWD &operator=(const CImageXWD &xwd);

 private:
  bool readHeader(CFile *file, CImagePtr &image, XWDFileHeader *hdr, bool *swap_flag);

  uint getPixel(XWDFileHeader *hdr, uchar *data, uint *pos);

  uchar *expandData(uchar *data, int width, int height);
};

#endif
