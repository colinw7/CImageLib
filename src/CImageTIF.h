#ifndef CIMAGE_TIF_H
#define CIMAGE_TIF_H

#include <CImageFmt.h>

#define CImageTIFInst CImageTIF::getInstance()

class CImageTIF : public CImageFmt {
 public:
  static CImageTIF *getInstance() {
    static CImageTIF *instance;

    if (! instance)
      instance = new CImageTIF;

    return instance;
  }

  bool read(CFile *file, CImagePtr &image);

  bool readHeader(CFile *file, CImagePtr &image);

  bool write(CFile *file, CImagePtr image);

 private:
  CImageTIF() :
   CImageFmt(CFILE_TYPE_IMAGE_TIF) {
  }

 ~CImageTIF() { }

  CImageTIF(const CImageTIF &tif);

  const CImageTIF &operator=(const CImageTIF &tif);

 private:
  void setInteger(int tag_id, int value);
  void setIntegerArray(int tag_id, int *values, int num_values);
  void readInteger(CFile *file, int *integer);
  void readShort(CFile *file, int *integer);

  void writeShortTag(CFile *file, int tag, int data);
  void writeLongTag(CFile *file, int tag, int data);
  void writeRationalTag(CFile *file, int tag, int offset);
  void writeShortArrayTag(CFile *file, int tag, int num_data, int offset);
  void writeLongArrayTag(CFile *file, int tag, int num_data, int offset);
  void writeData(CFile *file, uchar *data, int num_data);
  void writeInteger(CFile *file, int data);
  void writeShort(CFile *file, int data);
  void writeByte(CFile *file, int data);
};

#endif
