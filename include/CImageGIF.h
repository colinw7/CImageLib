#ifndef CIMAGE_GIF_H
#define CIMAGE_GIF_H

#include <CImageFmt.h>

#define CImageGIFInst CImageGIF::getInstance()

struct CImageGIFDict {
  uint code_value;
  uint parent_code;
  uint character;
};

struct CImageGIFHeader;
struct CImageGIFImageHeader;
struct CImageGIFData;

class CImageGIF : public CImageFmt {
 public:
  static CImageGIF *getInstance() {
    static CImageGIF *instance;

    if (! instance)
      instance = new CImageGIF;

    return instance;
  }

  bool read(CFile *file, CImagePtr &image);
  bool readHeader(CFile *file, CImagePtr &image);

  bool write(CFile *file, CImagePtr image);

  static bool writeAnim(CFile *file, const std::vector<CImagePtr> &images, int delay=0);

 private:
  CImageGIF() :
    CImageFmt(CFILE_TYPE_IMAGE_GIF) {
  }

 ~CImageGIF() { }

  CImageGIF(const CImageGIF &gif);

  const CImageGIF &operator=(const CImageGIF &gif);

 public:
  static CImageAnim *createAnim(CFile *file);

 private:
  static bool readHeader(CFile *file, CImagePtr &image, CImageGIFHeader *header);

  static void readGlobalColors(CFile *file, CImageGIFData *gif_data);

  static bool readAnimData(CFile *file, CImageAnim *image_anim, CImageGIFData *gif_data);

  static bool decompressData(uchar *in_data, int in_data_size, uchar *out_data, int out_data_size);

  static uint readCode(uint *bit_offset, uchar *data);

  static void deInterlace(uchar *image_data, CImageGIFImageHeader *image_header);

  static void writeHeader(CFile *file, CImagePtr image);
  static void writeGraphicsBlock(CFile *file, CImagePtr image, int delay=0);
  static void writeData(CFile *file, CImagePtr image);

  static uint findChildCode(uint parent_code, uint character);

  static void clearDictionary();
  static void outputCode(CFile *file, uint code);
  static void writeCodeByte(CFile *file, int data);
  static void flushCodeBytes(CFile *file);
  static void writeChars(CFile *file, const char *chars, int len);
  static void writeShort(CFile *file, int data);
  static void writeByte(CFile *file, int data);
};

#endif
