#ifndef CIMAGE_PSP_H
#define CIMAGE_PSP_H

#include <CImageFmt.h>

#define CImagePSPInst CImagePSP::getInstance()

class CImagePSP : public CImageFmt {
 public:
  static CImagePSP *getInstance() {
    static CImagePSP *instance;

    if (! instance)
      instance = new CImagePSP;

    return instance;
  }

  bool read(CFile *file, CImagePtr &image);

  bool readHeader(CFile *file, CImagePtr &image);

  bool write(CFile *file, CImagePtr image);

 private:
  CImagePSP() :
   CImageFmt(CFILE_TYPE_IMAGE_PSP) {
  }

 ~CImagePSP() { }

  CImagePSP(const CImagePSP &psp);

  const CImagePSP &operator=(const CImagePSP &psp);

 private:
  bool readImageBlock(uchar *buffer, int *width, int *height,
                      int *depth, int *compression, int *grey_scale);
  bool readColorBlock(uchar *buffer, CRGBA **colors, int *num_colors);
  bool readLayerStartBlock(uchar *buffer, int compression, std::vector<uchar *> *layers,
                           std::vector<int> *layer_sizes);
  void readLayerInformationChunk(uchar *buffer, int *num_channels);
  bool readLayerChannelSubBlock(uchar *buffer, uchar **block_data, int *block_size1,
                                int *block_size2, int *block_len);
  bool uncompressData(int compression, uchar *block_data, int block_size1, int block_size2,
                      uchar **block_data1);

  void getInteger(uchar *buffer, int *integer);
  void getShort  (uchar *buffer, int *integer);
  void getByte   (uchar *buffer, int *integer);

  bool isBlockID(uchar *buffer);
};

#endif
