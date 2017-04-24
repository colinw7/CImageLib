#ifndef CIMAGE_SGI_H
#define CIMAGE_SGI_H

#include <CImageFmt.h>

#define CImageSGIInst CImageSGI::getInstance()

struct SGIImage;

class CImageSGI : public CImageFmt {
 public:
  static CImageSGI *getInstance() {
    static CImageSGI *instance;

    if (! instance)
      instance = new CImageSGI;

    return instance;
  }

  bool read(CFile *file, CImagePtr &image);

  bool readHeader(CFile *file, CImagePtr &image);

  bool write(CFile *file, CImagePtr image);

 private:
  CImageSGI() :
   CImageFmt(CFILE_TYPE_IMAGE_SGI) {
  }

 ~CImageSGI() { }

  CImageSGI(const CImageSGI &sgi);

  const CImageSGI &operator=(const CImageSGI &sgi);

 private:
  void convertDataBW(uchar *data, SGIImage *sgi_image,
                     uint **data1, CRGBA **colors, int *num_colors);
  void convertDataRGB(uchar *data, CImagePtr image,
                      SGIImage *sgi_image, uint **data1);

  uchar *readCompressedData(CFile *file, SGIImage *sgi_image);
  uchar *readRawData(CFile *file, SGIImage *sgi_image);

  void interleaveRow(uchar *outp, uchar *inp, int z, int len);
  void expandRow(uchar *outp, uchar *inp, int z);

  void readTable(CFile *file, uint *table, int len);

  uint   getLong(CFile *file);
  ushort getShort(CFile *file);
  uchar  getByte(CFile *file);
};

#endif
