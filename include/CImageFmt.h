#ifndef CIMAGE_FMT_H
#define CIMAGE_FMT_H

#include <CFileType.h>
#include <CImagePtr.h>

class CFile;

class CImageFmt {
 private:
  CFileType type_;

 public:
  CImageFmt(CFileType type) : type_(type) { }

  virtual ~CImageFmt() { }

  CFileType getType() const { return type_; }

  virtual bool read(CFile *, CImagePtr &) = 0;
  virtual bool readHeader(CFile *, CImagePtr &) = 0;

  virtual bool write(CFile *, CImagePtr) = 0;
};

#endif
