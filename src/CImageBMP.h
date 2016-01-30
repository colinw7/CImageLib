#ifndef CIMAGE_BMP_H
#define CIMAGE_BMP_H

#include <CImageFmt.h>

#define CImageBMPInst CImageBMP::getInstance()

class CImageBMPHeader;

class CImageBMP : public CImageFmt {
 public:
  static CImageBMP *getInstance() {
    static CImageBMP *instance;

    if (! instance)
      instance = new CImageBMP;

    return instance;
  }

  bool read(CFile *file, CImagePtr &image);

  bool readHeader(CFile *file, CImagePtr &image);

  bool write(CFile *file, CImagePtr image);

 private:
  CImageBMP() :
   CImageFmt(CFILE_TYPE_IMAGE_BMP) {
  }

 ~CImageBMP() { }

  CImageBMP(const CImageBMP &bmp);

  const CImageBMP &operator=(const CImageBMP &bmp);

 private:
  bool readHeader(CFile *file, CImageBMPHeader *header);

  void readColors(CFile *file, CImageBMPHeader *header, CRGBA **colors);

  bool readData(CFile *file, CImagePtr &image, CImageBMPHeader *header, uint **data);

  void readCmp0Data8(CFile *file, int width, int height, uint **data, int *num_data);
  void readCmp1Data8(CFile *file, int width, int height, uint **data, int *num_data);

  void readInteger(uchar *, int *);
  void readShort(uchar *, int *);

  void writeInteger(CFile *file, int data);
  void writeShort(CFile *file, int data);
  void writeByte(CFile *file, int data);
};

#endif
