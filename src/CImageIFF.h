#ifndef CIMAGE_IFF_H
#define CIMAGE_IFF_H

#include <CImageFmt.h>

#define CImageIFFInst CImageIFF::getInstance()

typedef uchar  IFF_UBYTE;
typedef short  IFF_WORD;
typedef ushort IFF_UWORD;
typedef int    IFF_LONG;
typedef uint   IFF_ULONG;

struct IFFBitMapHeader;
struct IFFColorRegister;
struct IFFCommodoreAmiga;
struct IFFColorRange;

class CImageIFF : public CImageFmt {
 public:
  static CImageIFF *getInstance() {
    static CImageIFF *instance;

    if (! instance)
      instance = new CImageIFF;

    return instance;
  }

  bool read(CFile *file, CImagePtr &image);

  bool readHeader(CFile *file, CImagePtr &image);

  bool write(CFile *file, CImagePtr image);

 private:
  CImageIFF() :
    CImageFmt(CFILE_TYPE_IMAGE_IFF) {
  }

 ~CImageIFF() { }

  CImageIFF(const CImageIFF &iff);

  const CImageIFF &operator=(const CImageIFF &iff);

 private:
  bool readFORM(CFile *file);
  bool readILBM(CFile *file);
  bool readBMHD(CFile *file, IFFBitMapHeader *bitmap_header);
  bool readCMAP(CFile *file, IFFColorRegister *cregs, int *num_colors);
  bool readCAMG(CFile *file, IFFCommodoreAmiga *commodore_amiga);
  bool readCRNG(CFile *file, IFFColorRange *color_range);
  bool readBody(CFile *file, IFF_UBYTE **screen_memory,
                IFF_ULONG *screen_memory_size);
  bool readUnknown(CFile *file);
  bool readHeaderName(CFile *file, char *name);
  bool readHeaderLength(CFile *file, IFF_ULONG *length);
  bool readBytes(CFile *file, int length);
  bool readStorage(CFile *file, int length, IFF_UBYTE *buffer);

  IFF_UBYTE *decompressScreenMemory(IFF_UBYTE *screen_memory,
                                    IFF_ULONG screen_memory_size,
                                    int width, int height, int depth);
  IFF_UBYTE *convertScreenMemory8(IFF_UBYTE *screen_memory,
                                  int width, int height, int depth);
  IFF_UBYTE *convertScreenMemory24(IFF_UBYTE *screen_memory,
                                   int width, int height);
  IFF_UBYTE *convertHAM(IFF_UBYTE *screen_memory, int width,
                        int height, IFFColorRegister *cregs,
                        CRGBA **colors, int *num_colors);
  IFF_UBYTE *convertHAM8(IFF_UBYTE *screen_memory, int width,
                         int height, IFFColorRegister *cregs,
                         CRGBA **colors, int *num_colors);
  IFF_UBYTE *convert24Bit(IFF_UBYTE *screen_memory, int width,
                          int height, CRGBA **colors, int *num_colors);

  CRGBA *convertColors(IFFColorRegister *cregs, int num_colors);

  void convertWord(IFF_UWORD *integer);
};

#endif
