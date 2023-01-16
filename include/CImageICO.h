#ifndef CIMAGE_ICO_H
#define CIMAGE_ICO_H

#include <CImageFmt.h>

#define CImageICOInst CImageICO::getInstance()

class CImageICO : public CImageFmt {
 public:
  static CImageICO *getInstance() {
    static CImageICO *instance;

    if (! instance)
      instance = new CImageICO;

    return instance;
  }

  bool read(CFile *file, CImagePtr &image) override;
  bool readHeader(CFile *file, CImagePtr &image) override;

  bool write(CFile *file, CImagePtr image) override;

 private:
  CImageICO() :
    CImageFmt(CFILE_TYPE_IMAGE_ICO) {
  }

 ~CImageICO() { }

  CImageICO(const CImageICO &ico);

  CImageICO &operator=(const CImageICO &ico);

 private:
  void writeInteger(CFile *file, int data);
  void writeShort(CFile *file, int data);
  void writeByte(CFile *file, int data);
};

#endif
