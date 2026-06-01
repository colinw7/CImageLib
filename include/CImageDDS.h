#ifndef CIMAGE_DDS_H
#define CIMAGE_DDS_H

#include <CImageFmt.h>

#define CImageDDSInst CImageDDS::getInstance()

class CImageDDS : public CImageFmt {
 public:
  static CImageDDS *getInstance() {
    static CImageDDS *instance;

    if (! instance)
      instance = new CImageDDS;

    return instance;
  }

  bool read(CFile *file, CImagePtr &image) override;
  bool readHeader(CFile *file, CImagePtr &image) override;

  bool write(CFile *file, CImagePtr image) override;

 private:
  CImageDDS() :
   CImageFmt(CFILE_TYPE_IMAGE_DDS) {
  }

 ~CImageDDS() { }

  CImageDDS(const CImageDDS &six);

  CImageDDS &operator=(const CImageDDS &six);
};

#endif
