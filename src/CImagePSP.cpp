#include <CImageLib.h>
#include <CImagePSP.h>
#include <CThrow.h>

#include <cstring>

enum PSPBlockID {
  PSP_IMAGE_BLOCK = 0,
  PSP_CREATOR_BLOCK,
  PSP_COLOR_BLOCK,
  PSP_LAYER_START_BLOCK,
  PSP_LAYER_BLOCK,
  PSP_CHANNEL_BLOCK,
  PSP_SELECTION_BLOCK,
  PSP_ALPHA_BANK_BLOCK,
  PSP_ALPHA_CHANNEL_BLOCK,
  PSP_THUMBNAIL_BLOCK,
  PSP_EXTENDED_DATA_BLOCK,
  PSP_TUBE_BLOCK
};

enum PSPCompression {
  PSP_COMP_NONE = 0,
  PSP_COMP_RLE,
  PSP_COMP_LZ77
};

bool
CImagePSP::
read(CFile *file, CImagePtr &image)
{
  file->rewind();

  uchar buffer[128];

  file->read(buffer, 32);

  if (strncmp((char *) buffer, "Paint Shop Pro Image File", 25) != 0) {
    CImage::errorMsg("Invalid Paint Shop Pro Image File");
    return false;
  }

  file->read(buffer, 4);

  int version1;
  int version2;

  getShort(&buffer[0], &version1);
  getShort(&buffer[2], &version2);

  CRGBA *colors     = 0;
  int    num_colors = 0;

  std::vector<uchar *> layers;
  std::vector<int>     layer_sizes;

  int depth;
  int width;
  int height;
  int compression;

  while (true) {
    try {
      file->read(buffer, 14);
    }
    catch (...) {
      break;
    }

    if (! isBlockID(&buffer[0])) {
      CImage::errorMsg("Invalid Paint Shop Pro Image File");
      return false;
    }

    int id;

    getShort(&buffer[4], &id);

    int len1;
    int len2;

    getInteger(&buffer[6 ], &len1);
    getInteger(&buffer[10], &len2);

    uchar *buffer1 = new uchar [len2];

    file->read(buffer1, len2);

    bool flag;

    if      (id == PSP_IMAGE_BLOCK) {
      int grey_scale;

      flag = readImageBlock(buffer1, &width, &height, &depth,
                            &compression, &grey_scale);
    }
    else if (id == PSP_COLOR_BLOCK)
      flag = readColorBlock(buffer1, &colors, &num_colors);
    else if (id == PSP_LAYER_START_BLOCK)
      flag = readLayerStartBlock(buffer1, compression,
                                 &layers, &layer_sizes);
    else
      flag = true;

    delete [] buffer1;

    if (! flag) {
      CImage::errorMsg("Invalid Paint Shop Pro Image File");
      return false;
    }
  }

  /*------*/

  if      (depth == 1) {
    if (layers.size() < 1) {
      CImage::errorMsg("Invalid Paint Shop Pro Image File");
      return false;
    }
  }
  else if (depth == 4) {
    if (layers.size() < 1) {
      CImage::errorMsg("Invalid Paint Shop Pro Image File");
      return false;
    }
  }
  else if (depth == 8) {
    if (layers.size() < 1) {
      CImage::errorMsg("Invalid Paint Shop Pro Image File");
      return false;
    }
  }
  else if (depth == 24) {
    if (layers.size() < 3) {
      CImage::errorMsg("Invalid Paint Shop Pro Image File");
      return false;
    }
  }
  else {
    CImage::errorMsg("Invalid Paint Shop Pro Image File");
    return false;
  }

  /*------*/

  uint *data = new uint [width*height];

  if      (depth == 1) {
    int j = 0;

    for (int i = 0; i < width*height/8; ++i) {
      data[j++] = ((layers[0][i] & 0x80) >> 7);
      data[j++] = ((layers[0][i] & 0x40) >> 6);
      data[j++] = ((layers[0][i] & 0x20) >> 5);
      data[j++] = ((layers[0][i] & 0x10) >> 4);
      data[j++] = ((layers[0][i] & 0x08) >> 3);
      data[j++] = ((layers[0][i] & 0x04) >> 2);
      data[j++] = ((layers[0][i] & 0x03) >> 1);
      data[j++] = ((layers[0][i] & 0x01) >> 0);
    }
  }
  else if (depth == 4) {
    int j = 0;

    for (int i = 0; i < width*height/2; ++i) {
      data[j++] = ((layers[0][i] & 0xF0) >> 4);
      data[j++] = ((layers[0][i] & 0x0F) >> 0);
    }
  }
  else if (depth == 8) {
    for (int i = 0; i < width*height; ++i)
      data[i] = layers[0][i];
  }
  else if (depth == 24) {
    for (int i = 0; i < width*height; ++i)
      data[i] = image->rgbaToPixelI(layers[0][i],
                                    layers[1][i],
                                    layers[2][i]);
  }

  /*------*/

  image->setType(CFILE_TYPE_IMAGE_PSP);

  image->setDataSize(width, height);

  if (depth <= 8) {
    image->setColorIndexData(data);

    for (int i = 0; i < num_colors; ++i)
      image->addColor(colors[i]);

    delete [] colors;
  }
  else
    image->setRGBAData(data);

  delete [] data;

  /*------*/

  for (uint i = 0; i < layers.size(); ++i)
    delete [] layers[i];

  return true;
}

bool
CImagePSP::
readHeader(CFile *, CImagePtr &)
{
  return false;
}

bool
CImagePSP::
readImageBlock(uchar *buffer, int *width, int *height,
               int *depth, int *compression, int *grey_scale)
{
  getInteger(&buffer[0 ], width      );
  getInteger(&buffer[4 ], height     );
  getShort  (&buffer[19], depth      );
  getShort  (&buffer[17], compression);
  getByte   (&buffer[27], grey_scale );

  if (CImageState::getDebug()) {
    CImage::infoMsg("Width       = " + std::to_string(*width));
    CImage::infoMsg("Height      = " + std::to_string(*height));
    CImage::infoMsg("Depth       = " + std::to_string(*depth));
    CImage::infoMsg("Compression = " + std::to_string(*compression));
    CImage::infoMsg("Gray Scale  = " + std::to_string(*grey_scale));
  }

  return true;
}

bool
CImagePSP::
readColorBlock(uchar *buffer, CRGBA **colors, int *num_colors)
{
  getInteger(&buffer[0], num_colors);

  if (CImageState::getDebug())
    CImage::infoMsg("Num Colors = " + std::to_string(*num_colors));

  *colors = new CRGBA [*num_colors];

  int r, g, b;

  for (int i = 0; i < *num_colors; ++i) {
    getByte(&buffer[4*(i + 1) + 2], &r);
    getByte(&buffer[4*(i + 1) + 1], &g);
    getByte(&buffer[4*(i + 1) + 0], &b);

    (*colors)[i].setRGBAI(r, g, b);

    if (CImageState::getDebug())
      CImage::infoMsg("Colors[" + std::to_string(i + 1) + "] = " +
                      std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b));
  }

  return true;
}

bool
CImagePSP::
readLayerStartBlock(uchar *buffer, int compression,
                    std::vector<uchar *> *layers, std::vector<int> *layer_sizes)
{
  if (! isBlockID(&buffer[0])) {
    CImage::errorMsg("Invalid Paint Shop Pro Image File");
    return false;
  }

  int id;

  getShort(&buffer[4], &id);

  if (id != PSP_LAYER_BLOCK) {
    CImage::errorMsg("Invalid Paint Shop Pro Image File");
    return false;
  }

  int len1;
  int len2;

  getInteger(&buffer[6 ], &len1);
  getInteger(&buffer[10], &len2);

  int pos = 14;

  int num_channels;

  readLayerInformationChunk(&buffer[pos], &num_channels);

  pos += len1;

  uchar *block_data1 = 0;
  int    block_size2 = 0;

  for (int i = 0; i < num_channels; ++i) {
    int    block_len;
    uchar *block_data;
    int    block_size1;

    readLayerChannelSubBlock(&buffer[pos], &block_data,
                             &block_size1, &block_size2, &block_len);

    bool flag = uncompressData(compression, block_data, block_size1,
                               block_size2, &block_data1);

    delete [] block_data;

    if (! flag) {
      CImage::errorMsg("Invalid Paint Shop Pro Image File");
      return false;
    }

    pos += block_len;

    (*layers     ).push_back(block_data1);
    (*layer_sizes).push_back(block_size2);
  }

  return true;
}

void
CImagePSP::
readLayerInformationChunk(uchar *buffer, int *num_channels)
{
  getShort(&buffer[373], num_channels);
}

bool
CImagePSP::
readLayerChannelSubBlock(uchar *buffer, uchar **block_data,
                            int *block_size1, int *block_size2,
                            int *block_len)
{
  if (! isBlockID(&buffer[0])) {
    CImage::errorMsg("Invalid Paint Shop Pro Image File");
    return false;
  }

  int id;

  getShort(&buffer[4], &id);

  if (id != PSP_CHANNEL_BLOCK) {
    CImage::errorMsg("Invalid Paint Shop Pro Image File");
    return false;
  }

  int len1;
  int len2;

  getInteger(&buffer[6 ], &len1);
  getInteger(&buffer[10], &len2);

  getInteger(&buffer[14], block_size1);
  getInteger(&buffer[18], block_size2);

  *block_data = new uchar [*block_size1];

  memcpy(*block_data, &buffer[26], *block_size1);

  *block_len = len2 + 14;

  return true;
}

bool
CImagePSP::
uncompressData(int compression, uchar *block_data,
                  int block_size1, int block_size2,
                  uchar **block_data1)
{
  *block_data1 = new uchar [block_size2];

  if (compression == PSP_COMP_NONE) {
    for (int j = 0; j < block_size1; ++j)
      (*block_data1)[j] = block_data[j];

    return true;
  }

  if (compression == PSP_COMP_LZ77)
    return false;

  int i1 = 0;
  int i2 = 0;

  while (i1 < block_size1) {
    int count = block_data[i1++];

    if      (count > 128) {
      count -= 128;

      int byte = block_data[i1++];

      for (int j = 0; j < count; ++j)
        (*block_data1)[i2++] = byte;
    }
    else if (count < 128) {
      if (count == 0)
        count = 128;

      for (int j = 0; j < count; ++j)
        (*block_data1)[i2++] = block_data[i1++];
    }
  }

  return true;
}

void
CImagePSP::
getInteger(uchar *buffer, int *integer)
{
  *integer = ((buffer[0] & 0xFF)      ) |
             ((buffer[1] & 0xFF) <<  8) |
             ((buffer[2] & 0xFF) << 16) |
             ((buffer[3] & 0xFF) << 24);
}

void
CImagePSP::
getShort(uchar *buffer, int *integer)
{
  *integer = ((buffer[0] & 0xFF)     ) |
             ((buffer[1] & 0xFF) << 8);
}

void
CImagePSP::
getByte(uchar *buffer, int *integer)
{
  *integer = buffer[0];
}

bool
CImagePSP::
isBlockID(uchar *buffer)
{
  return (buffer[0] == 0x7E && buffer[1] == 0x42 &&
          buffer[2] == 0x4B && buffer[3] == 0x00);
}

bool
CImagePSP::
write(CFile *, CImagePtr)
{
  return false;
}
