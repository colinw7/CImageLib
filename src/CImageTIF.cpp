#include <CImageLib.h>
#include <CImageTIF.h>
#include <CThrow.h>

#include <cstring>

#define NEW_SUBFILE_TYPE  254
#define IMAGE_WIDTH       256
#define IMAGE_HEIGHT      257
#define BITS_PER_SAMPLE   258
#define COMPRESSION       259
#define PHOTOMETRIC       262
#define STRIP_OFFSETS     273
#define SAMPLES_PER_PIXEL 277
#define ROWS_PER_STRIP    278
#define STRIP_BYTE_COUNTS 279
#define X_RESOLUTION      282
#define Y_RESOLUTION      283
#define PLANAR_CONFIG     284
#define RESOLUTION_UNIT   296
#define COLORMAP          320

#define TIF_WHITE_IS_ZERO 0
#define TIF_BLACK_IS_ZERO 1
#define TIF_RGB           2
#define TIF_PALETTERGB    3

#define TAG_SHORT    3
#define TAG_LONG     4
#define TAG_RATIONAL 5

struct CImageTIFData {
  int    byte_order;
  int    image_width;
  int    image_height;
  int    bits_per_sample;
  int    compression;
  int    photometric;
  int    samples_per_pixel;
  int    rows_per_strip;
  int    planar_config;
  int    resolution_unit;
  int    num_strip_offsets;
  int   *strip_offsets;
  int    num_strip_byte_counts;
  int   *strip_byte_counts;
  int    num_colors;
  CRGBA *colors;
};

CImageTIFData tif_data;

bool
CImageTIF::
read(CFile *file, CImagePtr &image)
{
  try {
    tif_data.byte_order            = 0;
    tif_data.image_width           = -1;
    tif_data.image_height          = -1;
    tif_data.bits_per_sample       = -1;
    tif_data.compression           = 1;
    tif_data.photometric           = -1;
    tif_data.samples_per_pixel     = -1;
    tif_data.rows_per_strip        = -1;
    tif_data.planar_config         = -1;
    tif_data.resolution_unit       = -1;
    tif_data.num_strip_offsets     = 0;
    tif_data.strip_offsets         = 0;
    tif_data.num_strip_byte_counts = 0;
    tif_data.strip_byte_counts     = 0;
    tif_data.num_colors            = 0;
    tif_data.colors                = 0;

    //------

    file->rewind();

    //------

    uchar buffer[4];

    file->read(buffer, 4);

    if (buffer[0] == 0x49 && buffer[1] == 0x49)
      tif_data.byte_order = 0;
    else
      tif_data.byte_order = 1;

    if (CImageState::getDebug())
      CImage::infoMsg("Byte Order = " + std::to_string(tif_data.byte_order));

    //------

    int offset;

    readInteger(file, &offset);

    //------

    int num_tags;

    file->setPos(offset);

    readShort(file, &num_tags);

    if (CImageState::getDebug())
      CImage::infoMsg("Num Tags = " + std::to_string(num_tags));

    //------

    for (int i = 0; i < num_tags; ++i) {
      int tag_id;
      int data_type;
      int data_count;

      readShort  (file, &tag_id     );
      readShort  (file, &data_type  );
      readInteger(file, &data_count );

      if (CImageState::getDebug()) {
        CImage::infoMsg("Tag Id      = " + std::to_string(tag_id));
        CImage::infoMsg("Data Type   = " + std::to_string(data_type));
        CImage::infoMsg("Data Count  = " + std::to_string(data_count));
      }

      int data_offset;

      if      (data_type == TAG_SHORT) {
        if      (data_count == 1) {
          int dummy;

          readShort(file, &data_offset);
          readShort(file, &dummy      );

          setInteger(tag_id, data_offset);
        }
        else if (data_count > 1) {
          readInteger(file, &data_offset);

          int pos = file->getPos();

          file->setPos(data_offset);

          int *integer_array = new int [data_count];

          for (int j = 0; j < data_count; ++j)
            readShort(file, &integer_array[j]);

          setIntegerArray(tag_id, integer_array, data_count);

          delete [] integer_array;

          file->setPos(pos);
        }
        else
          readInteger(file, &data_offset);
      }
      else if (data_type == TAG_LONG) {
        if      (data_count == 1) {
          readInteger(file, &data_offset);

          setInteger(tag_id, data_offset);
        }
        else if (data_count > 1) {
          readInteger(file, &data_offset);

          int pos = file->getPos();

          file->setPos(data_offset);

          int *integer_array = new int [data_count];

          for (int j = 0; j < data_count; ++j)
            readInteger(file, &integer_array[j]);

          setIntegerArray(tag_id, integer_array, data_count);

          delete [] integer_array;

          file->setPos(pos);
        }
        else
          readInteger(file, &data_offset);
      }
      else {
        readInteger(file, &data_offset);

        if (CImageState::getDebug())
          CImage::infoMsg("Data Offset = " + std::to_string(data_offset));
      }
    }

    //------

    if (tif_data.image_width == -1 || tif_data.image_height == -1) {
      CImage::errorMsg("Invalid Image Width/Height");
      return false;
    }

    if (tif_data.bits_per_sample == -1) {
      CImage::errorMsg("Invalid Bits per Sample");
      return false;
    }

    if (tif_data.num_strip_byte_counts != tif_data.num_strip_offsets) {
      CImage::errorMsg("Invalid Strip Byte Counts");
      return false;
    }

    if (tif_data.photometric != TIF_WHITE_IS_ZERO &&
        tif_data.photometric != TIF_BLACK_IS_ZERO &&
        tif_data.photometric != TIF_RGB           &&
        tif_data.photometric != TIF_PALETTERGB) {
      CImage::errorMsg("Unsupported Photometric Type");
      return false;
    }

    if (tif_data.compression != 1) {
      CImage::errorMsg("Unsupported Compression Type");
      return false;
    }

    //------

    if (tif_data.samples_per_pixel == -1)
      tif_data.samples_per_pixel = 1;

    int depth = tif_data.bits_per_sample*tif_data.samples_per_pixel;

    if (depth != 1 && depth != 8 && depth != 16 && depth != 24) {
      CImage::errorMsg("Unsupported Depth");
      return false;
    }

    if (tif_data.photometric == TIF_RGB && depth < 16) {
      CImage::errorMsg("Invalid Photometric Type for Depth < 16");
      return false;
    }

    if ((tif_data.photometric == TIF_WHITE_IS_ZERO ||
         tif_data.photometric == TIF_BLACK_IS_ZERO ||
         tif_data.photometric == TIF_PALETTERGB) && depth > 8) {
      CImage::errorMsg("Invalid Photometric Type for Depth > 8");
      return false;
    }

    if (tif_data.photometric == TIF_PALETTERGB && tif_data.colors == 0) {
      CImage::errorMsg("No Colors for RGB");
      return false;
    }

    if (tif_data.photometric == TIF_WHITE_IS_ZERO ||
        tif_data.photometric == TIF_BLACK_IS_ZERO) {
      if (depth == 8) {
        tif_data.num_colors = 256;

        tif_data.colors = new CRGBA [tif_data.num_colors];

        for (int i = 0; i < 256; ++i)
          tif_data.colors[i].setRGBAI(i, i, i);
      }
      else {
        tif_data.num_colors = 2;

        tif_data.colors = new CRGBA [tif_data.num_colors];

        tif_data.colors[0].setRGBA(0, 0, 0);
        tif_data.colors[1].setRGBA(1, 1, 1);
      }
    }

    int row_size = 0;

    if      (depth == 24)
      row_size = 4*tif_data.image_width;
    else if (depth == 16)
      row_size = 2*tif_data.image_width;
    else if (depth == 8)
      row_size = tif_data.image_width;
    else
      row_size = ((tif_data.image_width + 7)/8);

    assert(row_size > 0);

    int strips_per_image =
      (tif_data.image_height + tif_data.rows_per_strip - 1)/tif_data.rows_per_strip;

    if (tif_data.num_strip_offsets     != strips_per_image ||
        tif_data.num_strip_byte_counts != strips_per_image) {
      CImage::errorMsg("Invalid Strip Array Size");
      return false;
    }

    uint *data = new uint [tif_data.image_width*tif_data.image_height];

    int buffer_size = 0;

    for (int i = 0; i < strips_per_image; ++i)
      if (tif_data.strip_byte_counts[i] > buffer_size)
        buffer_size = tif_data.strip_byte_counts[i];

    uchar *buffer1 = new uchar [buffer_size];

    //------

    for (int i = 0; i < strips_per_image; ++i) {
      file->setPos(tif_data.strip_offsets[i]);

      file->read(buffer1, tif_data.strip_byte_counts[i]);

      uint *p = &data[i*tif_data.rows_per_strip*tif_data.image_width];

      uchar *p1 = buffer1;

      if      (depth == 24) {
        for (int j = 0; j < tif_data.strip_byte_counts[i]/4; ++j, ++p, p1 += 4)
          *p = image->rgbaToPixelI(p1[0], p1[1], p1[2]);
      }
      else if (depth == 16) {
        for (int j = 0; j < tif_data.strip_byte_counts[i]/2; ++j, ++p, p1 += 2)
          *p = image->rgbaToPixelI(( p1[0] & 0xF0      ),
                                   ((p1[0] & 0x0F) << 4),
                                   ( p1[1] & 0xF0      ));
      }
      else if (depth == 8) {
        for (int j = 0; j < tif_data.strip_byte_counts[i]; ++j, ++p, p1++)
          *p = *p1;
      }
      else {
        for (int j = 0; j < tif_data.strip_byte_counts[i]; ++j, p += 8, p1++) {
          p[0] = (*p1 & 0x01) >> 0;
          p[1] = (*p1 & 0x02) >> 1;
          p[2] = (*p1 & 0x04) >> 2;
          p[3] = (*p1 & 0x08) >> 3;
          p[4] = (*p1 & 0x10) >> 4;
          p[5] = (*p1 & 0x20) >> 5;
          p[6] = (*p1 & 0x40) >> 6;
          p[7] = (*p1 & 0x80) >> 7;
        }
      }
    }

    delete [] buffer1;

    //------

    image->setType(CFILE_TYPE_IMAGE_TIF);

    image->setDataSize(tif_data.image_width, tif_data.image_height);

    if (depth <= 8) {
      for (int i = 0; i < tif_data.num_colors; ++i)
        image->addColor(tif_data.colors[i]);

      delete [] tif_data.colors;

      image->setColorIndexData(data);
    }
    else
      image->setRGBAData(data);

    delete [] data;

    return true;
  }
  catch (...) {
    delete [] tif_data.strip_offsets;
    delete [] tif_data.strip_byte_counts;
    delete [] tif_data.colors;

    return false;
  }
}

bool
CImageTIF::
readHeader(CFile *, CImagePtr &)
{
  return false;
}

void
CImageTIF::
setInteger(int tag_id, int value)
{
  if (CImageState::getDebug())
    CImage::infoMsg("Integer     = " + std::to_string(value));

  switch (tag_id) {
    case IMAGE_WIDTH:
      tif_data.image_width = value;
      break;
    case IMAGE_HEIGHT:
      tif_data.image_height = value;
      break;
    case BITS_PER_SAMPLE:
      tif_data.bits_per_sample = value;
      break;
    case COMPRESSION:
      tif_data.compression = value;
      break;
    case PHOTOMETRIC:
      tif_data.photometric = value;
      break;
    case SAMPLES_PER_PIXEL:
      tif_data.samples_per_pixel = value;
      break;
    case ROWS_PER_STRIP:
      tif_data.rows_per_strip = value;
      break;
    case PLANAR_CONFIG:
      tif_data.planar_config = value;
      break;
    case RESOLUTION_UNIT:
      tif_data.resolution_unit = value;
      break;
    case STRIP_OFFSETS:
      if (tif_data.strip_offsets != 0)
        delete [] tif_data.strip_offsets;

      tif_data.num_strip_offsets = 1;
      tif_data.strip_offsets     = new int [1];

      tif_data.strip_offsets[0] = value;

      break;
    case STRIP_BYTE_COUNTS:
      if (tif_data.strip_byte_counts != 0)
        delete [] tif_data.strip_byte_counts;

      tif_data.num_strip_byte_counts = 1;
      tif_data.strip_byte_counts     = new int [1];

      tif_data.strip_byte_counts[0] = value;

      break;
    default:
      break;
  }
}

void
CImageTIF::
setIntegerArray(int tag_id, int *values, int num_values)
{
  if (CImageState::getDebug()) {
    std::string msg;

    for (int i = 0; i < num_values; ++i) {
      if (i > 0 && (i % 8) == 0)
        msg += "\n";

      msg += std::to_string(values[i]) + " ";
    }

    if (num_values > 0 && ((num_values - 1) % 8) != 0)
      msg += "\n";

    CImage::infoMsg(msg, false);
  }

  switch (tag_id) {
    case STRIP_OFFSETS: {
      if (tif_data.strip_offsets != 0)
        delete [] tif_data.strip_offsets;

      tif_data.num_strip_offsets = num_values;
      tif_data.strip_offsets     = new int [num_values];

      memcpy(tif_data.strip_offsets, values, num_values*sizeof(int));

      break;
    }
    case STRIP_BYTE_COUNTS: {
      if (tif_data.strip_byte_counts != 0)
        delete [] tif_data.strip_byte_counts;

      tif_data.num_strip_byte_counts = num_values;
      tif_data.strip_byte_counts     = new int [num_values];

      memcpy(tif_data.strip_byte_counts, values, num_values*sizeof(int));

      break;
    }
    case COLORMAP: {
      if (tif_data.colors != 0)
        delete [] tif_data.colors;

      tif_data.num_colors = num_values/3;
      tif_data.colors     = new CRGBA [tif_data.num_colors];

      int j1 = 0;
      int j2 = j1 + tif_data.num_colors;
      int j3 = j2 + tif_data.num_colors;

      for (int i = 0; i < tif_data.num_colors; ++i, j1++, j2++, j3++)
        tif_data.colors[i].setRGBA(values[j1]/65535.0,
                                   values[j2]/65535.0,
                                   values[j3]/65535.0);

      break;
    }
    default:
      break;
  }
}

void
CImageTIF::
readInteger(CFile *file, int *integer)
{
  uchar buffer[4];

  file->read(buffer, 4);

  if (tif_data.byte_order == 0)
    *integer = ((buffer[0] & 0xFF)      ) |
               ((buffer[1] & 0xFF) <<  8) |
               ((buffer[2] & 0xFF) << 16) |
               ((buffer[3] & 0xFF) << 24);
  else
    *integer = ((buffer[3] & 0xFF)      ) |
               ((buffer[2] & 0xFF) <<  8) |
               ((buffer[1] & 0xFF) << 16) |
               ((buffer[0] & 0xFF) << 24);
}

void
CImageTIF::
readShort(CFile *file, int *integer)
{
  uchar buffer[2];

  file->read(buffer, 2);

  if (tif_data.byte_order == 0)
    *integer = ((buffer[0] & 0xFF)     ) |
               ((buffer[1] & 0xFF) << 8);
  else
    *integer = ((buffer[1] & 0xFF)     ) |
               ((buffer[0] & 0xFF) << 8);
}

bool
CImageTIF::
write(CFile *file, CImagePtr image)
{
  int row_size = 0;

  int depth;

  if (image->hasColormap())
    depth = 8;
  else
    depth = 24;

  if      (depth == 24)
    row_size = 4*image->getWidth();
  else if (depth == 16)
    row_size = 2*image->getWidth();
  else if (depth == 8)
    row_size = image->getWidth();
  else
    row_size = ((image->getWidth() + 7)/8);

  int num_colors = 0;

  if (depth <= 8)
    num_colors = (1 << depth);

  //------

  int start_offset = 8;

  start_offset += image->getHeight()*row_size;

  if (depth <= 8)
    start_offset += 6*num_colors;

  start_offset += 4*image->getHeight();

  start_offset += 4*image->getHeight();

  start_offset += 8;

  start_offset += 8;

  //------

  writeShort  (file, 0x4949      );
  writeShort  (file, 0x002A      );
  writeInteger(file, start_offset);

  //------

  int offset = 8;

  //------

  int *strip_offsets = new int [image->getHeight()];

  uchar *strip_data = new uchar [row_size];

  int j = 0;

  uint r, g, b, a;

  for (uint i = 0; i < image->getHeight(); ++i) {
    strip_offsets[i] = offset;

    if      (depth == 24) {
      for (int k = 0; k < row_size; k += 4, ++j) {
        image->getRGBAPixelI(j, &r, &g, &b, &a);

        strip_data[k + 0] = r;
        strip_data[k + 1] = g;
        strip_data[k + 2] = b;
        strip_data[k + 3] = a;
      }
    }
    else if (depth == 16) {
      for (int k = 0; k < row_size; k += 2, ++j) {
        image->getRGBAPixelI(j, &r, &g, &b, &a);

        r >>= 4;
        g >>= 4;
        b >>= 4;

        strip_data[k + 0] = ((r & 0x0F) << 4) | (g & 0x0F);
        strip_data[k + 1] = (b & 0x0F);
      }
    }
    else if (depth == 8) {
      for (int k = 0; k < row_size; ++k, ++j)
        strip_data[k] = image->getColorIndexPixel(j);
    }
    else {
      for (int k = 0; k < row_size; ++k) {
        int pixel1 = image->getColorIndexPixel(j + 0);
        int pixel2 = image->getColorIndexPixel(j + 1);
        int pixel3 = image->getColorIndexPixel(j + 2);
        int pixel4 = image->getColorIndexPixel(j + 3);
        int pixel5 = image->getColorIndexPixel(j + 4);
        int pixel6 = image->getColorIndexPixel(j + 5);
        int pixel7 = image->getColorIndexPixel(j + 6);
        int pixel8 = image->getColorIndexPixel(j + 7);

        strip_data[k] = ((pixel1 & 0x01) << 0) |
                        ((pixel2 & 0x01) << 1) |
                        ((pixel3 & 0x01) << 2) |
                        ((pixel4 & 0x01) << 3) |
                        ((pixel5 & 0x01) << 4) |
                        ((pixel6 & 0x01) << 5) |
                        ((pixel7 & 0x01) << 6) |
                        ((pixel8 & 0x01) << 7);

        j += 8;
      }
    }

    writeData(file, strip_data, row_size);

    offset += row_size;
  }

  delete [] strip_data;

  //------

  int colormap_offset = offset;

  if (depth <= 8) {
    ushort *colors = new ushort [3*num_colors];

    double r, g, b, a;

    int i = 0;

    for (int j = 0; j < image->getNumColors(); ++j) {
      image->getColorRGBA(j, &r, &g, &b, &a);

      colors[i++] = (int) (r*65535);
    }

    i = num_colors;

    for (int j = 0; j < image->getNumColors(); ++j) {
      image->getColorRGBA(j, &r, &g, &b, &a);

      colors[i++] = (int) (g*65535);
    }

    i = 2*num_colors;

    for (int j = 0; j < image->getNumColors(); ++j) {
      image->getColorRGBA(j, &r, &g, &b, &a);

      colors[i++] = (int) (b*65535);
    }

    for (i = 0; i < 3*num_colors; ++i)
      writeShort(file, colors[i]);

    delete [] colors;

    offset += 6*num_colors;
  }

  //------

  int strip_offsets_offset = offset;

  for (uint i = 0; i < image->getHeight(); ++i)
    writeInteger(file, strip_offsets[i]);

  offset += 4*image->getHeight();

  //------

  int strip_byte_counts_offset = offset;

  for (uint i = 0; i < image->getHeight(); ++i)
    writeInteger(file, image->getWidth());

  offset += 4*image->getHeight();

  //------

  int x_resolution_offset = offset;

  writeInteger(file, 100);
  writeInteger(file, 1);

  offset += 8;

  //------

  int y_resolution_offset = offset;

  writeInteger(file, 100);
  writeInteger(file, 1);

  offset += 8;

  //------

  int num_tags = 14;

  writeShort(file, num_tags);

  writeLongTag(file, NEW_SUBFILE_TYPE, 0);
  writeLongTag(file, IMAGE_WIDTH     , image->getWidth());
  writeLongTag(file, IMAGE_HEIGHT    , image->getHeight());

  if (depth <= 8)
    writeShortTag(file, BITS_PER_SAMPLE, depth);
  else
    writeShortTag(file, BITS_PER_SAMPLE, 8);

  writeShortTag(file, COMPRESSION, 1);

  if (depth <= 8)
    writeShortTag(file, PHOTOMETRIC, TIF_PALETTERGB);
  else
    writeShortTag(file, PHOTOMETRIC, TIF_RGB);

  writeLongArrayTag(file, STRIP_OFFSETS, image->getHeight(),
                    strip_offsets_offset);

  if (depth <= 8)
    writeShortTag(file, SAMPLES_PER_PIXEL, 1);
  else
    writeShortTag(file, SAMPLES_PER_PIXEL, 3);

  writeLongTag(file, ROWS_PER_STRIP, 1);

  writeLongArrayTag(file, STRIP_BYTE_COUNTS, image->getHeight(),
                    strip_byte_counts_offset);

  writeRationalTag(file, X_RESOLUTION, x_resolution_offset);
  writeRationalTag(file, Y_RESOLUTION, y_resolution_offset);

  if (depth > 8)
    writeShortTag(file, PLANAR_CONFIG, 1);

  writeShortTag(file, RESOLUTION_UNIT, 1);

  if (depth <= 8)
    writeShortArrayTag(file, COLORMAP, 3*num_colors, colormap_offset);

  writeShort(file, 0);

  return true;
}

void
CImageTIF::
writeShortTag(CFile *file, int tag, int data)
{
  writeShort  (file, tag      );
  writeShort  (file, TAG_SHORT);
  writeInteger(file, 1        );
  writeShort  (file, data     );
  writeShort  (file, 0        );
}

void
CImageTIF::
writeLongTag(CFile *file, int tag, int data)
{
  writeShort  (file, tag     );
  writeShort  (file, TAG_LONG);
  writeInteger(file, 1       );
  writeInteger(file, data    );
}

void
CImageTIF::
writeRationalTag(CFile *file, int tag, int offset)
{
  writeShort  (file, tag         );
  writeShort  (file, TAG_RATIONAL);
  writeInteger(file, 1           );
  writeInteger(file, offset      );
}

void
CImageTIF::
writeShortArrayTag(CFile *file, int tag, int num_data, int offset)
{
  writeShort  (file, tag      );
  writeShort  (file, TAG_SHORT);
  writeInteger(file, num_data );
  writeInteger(file, offset   );
}

void
CImageTIF::
writeLongArrayTag(CFile *file, int tag, int num_data, int offset)
{
  writeShort  (file, tag     );
  writeShort  (file, TAG_LONG);
  writeInteger(file, num_data);
  writeInteger(file, offset  );
}

void
CImageTIF::
writeData(CFile *file, uchar *data, int num_data)
{
  for (int i = 0; i < num_data; ++i)
    writeByte(file, data[i]);
}

void
CImageTIF::
writeInteger(CFile *file, int data)
{
  uint i = data;

  file->putC( i        & 0xff);
  file->putC((i >>  8) & 0xff);
  file->putC((i >> 16) & 0xff);
  file->putC((i >> 24) & 0xff);
}

void
CImageTIF::
writeShort(CFile *file, int data)
{
  ushort s = data;

  file->putC( s       & 0xff);
  file->putC((s >> 8) & 0xff);
}

void
CImageTIF::
writeByte(CFile *file, int data)
{
  uchar c = data;

  file->putC(c & 0xff);
}
