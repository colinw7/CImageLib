#include <CImageLib.h>
#include <CImageSGI.h>
#include <CThrow.h>
#include <CStrUtil.h>

#include <cstring>

struct SGIImage {
  ushort  magic;
  uchar   compressed;
  uchar   bytes_per_pixel;
  ushort  dimension;
  ushort  x_size;
  ushort  y_size;
  ushort  z_size;
  uint    min;
  uint    max;
  uint    wastebytes;
  char    name[80];
  uint    colormap;
  int     file;
  ushort  flags;
  short   dorev;
  short   x;
  short   y;
  short   z;
  short   cnt;
  ushort *ptr;
  ushort *base;
  ushort *tmpbuf;
  uint    offset;
  uint    rleend;
  uint   *rowstart;
  int    *rowsize;
};

bool
CImageSGI::
read(CFile *file, CImagePtr &image)
{
  file->rewind();

  SGIImage sgi_image;

  sgi_image.magic = getShort(file);

  if (sgi_image.magic != 474) {
    CImage::errorMsg("Invalid SGI File");
    return false;
  }

  sgi_image.compressed      = getByte(file);
  sgi_image.bytes_per_pixel = getByte(file);
  sgi_image.dimension       = getShort(file);
  sgi_image.x_size          = getShort(file);
  sgi_image.y_size          = getShort(file);
  sgi_image.z_size          = getShort(file);

  if (CImageState::getDebug()) {
    CImage::infoMsg("Magic       " + std::to_string(sgi_image.magic));
    CImage::infoMsg("Compressed  " + std::to_string((int) sgi_image.compressed));
    CImage::infoMsg("Bytes/Pixel " + std::to_string((int) sgi_image.bytes_per_pixel));
    CImage::infoMsg("Dimension   " + std::to_string(sgi_image.dimension));
    CImage::infoMsg("X Size      " + std::to_string(sgi_image.x_size));
    CImage::infoMsg("Y Size      " + std::to_string(sgi_image.y_size));
    CImage::infoMsg("Z Size      " + std::to_string(sgi_image.z_size));
  }

  //------

  if (sgi_image.bytes_per_pixel != 1) {
    CImage::errorMsg(CStrUtil::toString(sgi_image.bytes_per_pixel) +
                     " Bytes Per Pixel not Supported");
    return false;
  }

  //------

  uchar *data;

  if (sgi_image.compressed)
    data = readCompressedData(file, &sgi_image);
  else
    data = readRawData(file, &sgi_image);

  //------

  int   depth;
  uint *data1;
  CRGBA *colors;
  int    num_colors;

  if (sgi_image.z_size < 3) {
    convertDataBW(data, &sgi_image, &data1, &colors, &num_colors);

    depth = 8;
  }
  else {
    convertDataRGB(data, image, &sgi_image, &data1);

    depth = 24;
  }

  //------

  delete [] data;

  if (data1 == 0) {
    CImage::errorMsg("Unhandled Data Type");
    return false;
  }

  //------

  image->setType(CFILE_TYPE_IMAGE_SGI);

  image->setDataSize(sgi_image.x_size, sgi_image.y_size);

  if (depth <= 8) {
    for (int i = 0; i < num_colors; ++i)
      image->addColor(colors[i]);

    delete [] colors;

    image->setColorIndexData(data1);
  }
  else
    image->setRGBAData(data1);

  delete [] data1;

  //------

  return true;
}

bool
CImageSGI::
readHeader(CFile *file, CImagePtr &image)
{
  file->rewind();

  SGIImage sgi_image;

  sgi_image.magic = getShort(file);

  if (sgi_image.magic != 474) {
    CImage::errorMsg("Invalid SGI File");
    return false;
  }

  sgi_image.compressed      = getByte(file);
  sgi_image.bytes_per_pixel = getByte(file);
  sgi_image.dimension       = getShort(file);
  sgi_image.x_size          = getShort(file);
  sgi_image.y_size          = getShort(file);
  sgi_image.z_size          = getShort(file);

  if (CImageState::getDebug()) {
    CImage::infoMsg("Magic       " + std::to_string(sgi_image.magic));
    CImage::infoMsg("Compressed  " + std::to_string((int) sgi_image.compressed));
    CImage::infoMsg("Bytes/Pixel " + std::to_string((int) sgi_image.bytes_per_pixel));
    CImage::infoMsg("Dimension   " + std::to_string(sgi_image.dimension));
    CImage::infoMsg("X Size      " + std::to_string(sgi_image.x_size));
    CImage::infoMsg("Y Size      " + std::to_string(sgi_image.y_size));
    CImage::infoMsg("Z Size      " + std::to_string(sgi_image.z_size));
  }

  //------

  image->setType(CFILE_TYPE_IMAGE_SGI);

  image->setSize(sgi_image.x_size, sgi_image.y_size);

  //------

  return true;
}

void
CImageSGI::
convertDataBW(uchar *data, SGIImage *sgi_image, uint **data1,
              CRGBA **colors, int *num_colors)
{
  *data1      = 0;
  *colors     = 0;
  *num_colors = 0;

  uchar *c_flags  = new uchar [256];
  uint  *c_flags1 = new uint  [256];

  int num_c = 0;

  memset(c_flags , '\0', 256*sizeof(uchar));
  memset(c_flags1, '\0', 256*sizeof(uint ));

  uchar *p = data;

  for (int i = 0; i < sgi_image->y_size; ++i) {
    for (int j = 0; j < sgi_image->x_size; ++j) {
      p += 3;

      uchar g = *p++;

      if (c_flags[g] == 1)
        continue;

      ++num_c;

      c_flags [g        ] = 1;
      c_flags1[num_c - 1] = g;
    }
  }

  if (CImageState::getDebug())
    CImage::infoMsg(std::to_string(num_c) + " Grays");

  *num_colors = num_c;
  *colors     = new CRGBA [num_c];

  memset(c_flags, '\0', 256*sizeof(uchar));

  uchar g;

  for (int i = 0; i < num_c; ++i) {
    g = (uchar) c_flags1[i];

    (*colors)[i].setRGBAI(g, g, g);

    if (CImageState::getDebug())
      CImage::infoMsg("Color : Gray " + std::to_string(g));

    c_flags[g] = i;
  }

  *data1 = new uint [sgi_image->x_size*sgi_image->y_size];

  p = data;

  for (int i = 0; i < sgi_image->y_size; ++i) {
    uint *p1 = *data1 + (sgi_image->y_size - 1 - i)*sgi_image->x_size;

    for (int j = 0; j < sgi_image->x_size; ++j) {
      p += 3;

      uchar g = *p++;

      *p1++ = c_flags[g];
    }
  }

  delete [] c_flags;
  delete [] c_flags1;
}

void
CImageSGI::
convertDataRGB(uchar *data, CImagePtr image, SGIImage *sgi_image, uint **data1)
{
  *data1 = new uint [sgi_image->x_size*sgi_image->y_size];

  uchar *p = data;

  int r, g, b;

  for (int i = 0; i < sgi_image->y_size; ++i) {
    uint *p1 = *data1 + (sgi_image->y_size - 1 - i)*sgi_image->x_size;

    for (int j = 0; j < sgi_image->x_size; ++j) {
      ++p;

      b = *p++;
      g = *p++;
      r = *p++;

      *p1++ = image->rgbaToPixelI(r, g, b);
    }
  }
}

uchar *
CImageSGI::
readCompressedData(CFile *file, SGIImage *sgi_image)
{
  uchar *data = 0;

  uint buffer_len = 2*sgi_image->x_size + 10;
  uint table_len  = sgi_image->y_size*sgi_image->z_size;

  uchar *buffer       = new uchar [buffer_len];
  uint  *start_table  = new uint  [table_len ];
  uint  *length_table = new uint  [table_len ];

  file->setPos(512);

  readTable(file, start_table , table_len);
  readTable(file, length_table, table_len);

  uint pos       = 0;
  bool bad_order = false;

  for (int y = 0; y < sgi_image->y_size; ++y) {
    for (int z = 0; z < sgi_image->z_size; ++z) {
      if (start_table[y + z*sgi_image->y_size] < pos) {
        bad_order = true;
        break;
      }

      pos = start_table[y + z*sgi_image->y_size];
    }

    if (bad_order)
      break;
  }

  pos = 512 + 2*table_len*4;

  file->setPos(pos);

  data = new uchar [sgi_image->x_size*sgi_image->y_size*4];

  if (bad_order) {
    for (int z = 0; z < sgi_image->z_size; ++z) {
      uchar *p = data;

      for (int y = 0; y < sgi_image->y_size; ++y) {
        if (pos != start_table[y + z*sgi_image->y_size]) {
          pos = start_table[y + z*sgi_image->y_size];

          file->setPos(pos);
        }

        if (length_table[y + z*sgi_image->y_size] > buffer_len) {
          delete [] data;

          CImage::warnMsg("Bad Data");
        }

        file->read(buffer, length_table[y + z*sgi_image->y_size]);

        pos += length_table[y + z*sgi_image->y_size];

        expandRow(p, buffer, 3 - z);

        p += 4*sgi_image->x_size;
      }
    }
  }
  else {
    uchar *p = data;

    for (int y = 0; y < sgi_image->y_size; ++y) {
      for (int z = 0; z < sgi_image->z_size; ++z) {
        if (pos != start_table[y + z*sgi_image->y_size]) {
          pos = start_table[y + z*sgi_image->y_size];

          file->setPos(pos);
        }

        file->read(buffer, length_table[y + z*sgi_image->y_size]);

        pos += length_table[y + z*sgi_image->y_size];

        expandRow(p, buffer, 3 - z);
      }

      p += 4*sgi_image->x_size;
    }
  }

  delete [] buffer;
  delete [] start_table;
  delete [] length_table;

  return data;
}

uchar *
CImageSGI::
readRawData(CFile *file, SGIImage *sgi_image)
{
  uchar *buffer = new uchar [sgi_image->x_size];

  uchar *data = new uchar [sgi_image->x_size*sgi_image->y_size*4];

  file->setPos(512);

  for (int z = 0; z < sgi_image->z_size; ++z) {
    uchar *p = data;

    for (int y = 0; y < sgi_image->y_size; ++y) {
      file->read(buffer, sgi_image->x_size);

      interleaveRow(p, buffer, 3 - z, sgi_image->x_size);

      p += sgi_image->x_size*4;
    }
  }

  delete [] buffer;

  return data;
}

void
CImageSGI::
interleaveRow(uchar *outp, uchar *inp, int z, int len)
{
  outp += z;

  while (len--) {
    *outp = *inp++;

    outp += 4;
  }
}

void
CImageSGI::
expandRow(uchar *outp, uchar *inp, int z)
{
  outp += z;

  while (true) {
    uchar pixel = *inp++;

    uchar count = (pixel & 0x7f);

    if (count == 0)
      break;

    if (pixel & 0x80) {
      while (count >= 8) {
        outp[0*4] = inp[0];
        outp[1*4] = inp[1];
        outp[2*4] = inp[2];
        outp[3*4] = inp[3];
        outp[4*4] = inp[4];
        outp[5*4] = inp[5];
        outp[6*4] = inp[6];
        outp[7*4] = inp[7];

        outp  += 8*4;
        inp   += 8;
        count -= 8;
      }

      while (count--) {
        *outp = *inp++;

        outp += 4;
      }
    }
    else {
      pixel = *inp++;

      while (count >= 8) {
        outp[0*4] = pixel;
        outp[1*4] = pixel;
        outp[2*4] = pixel;
        outp[3*4] = pixel;
        outp[4*4] = pixel;
        outp[5*4] = pixel;
        outp[6*4] = pixel;
        outp[7*4] = pixel;

        outp  += 8*4;
        count -= 8;
      }

      while (count--) {
        *outp = pixel;

        outp += 4;
      }
    }
  }
}

void
CImageSGI::
readTable(CFile *file, uint *table, int len)
{
  for ( ; len > 0; --len)
    *table++ = getLong(file);
}

uint
CImageSGI::
getLong(CFile *file)
{
  uchar buf[4];

  file->read(buf, 4);

  uint l = (buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3];

  return l;
}

ushort
CImageSGI::
getShort(CFile *file)
{
  uchar buf[2];

  file->read(buf, 2);

  ushort s = (buf[0] << 8) + buf[1];

  return s;
}

uchar
CImageSGI::
getByte(CFile *file)
{
  uchar c;

  file->read(&c, 1);

  return c;
}

bool
CImageSGI::
write(CFile *, CImagePtr)
{
  return false;
}
