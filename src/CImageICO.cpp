#include <CImageLib.h>
#include <CImageICO.h>

struct CImageICOHeader {
  ushort pad1;
  ushort type;
  ushort count;
};

struct CImageICOCursor {
  uchar  width;
  uchar  height;
  uchar  num_colors;
  uchar  pad1;
  ushort planes;
  ushort bits_per_pixel;
  uint   size;
  uint   offset;
};

struct CImageICOInfoHeader {
  uint   size;
  uint   width;
  uint   height;
  ushort depth;
  ushort bitcount;
  uint   compression;
  uint   image_size;
  uint   xpixels_per_m;
  uint   ypixels_per_m;
  uint   num_colors;
  uint   num_important_colors;
};

struct CImageICOColor {
  uchar r;
  uchar g;
  uchar b;
  uchar a;
};

bool
CImageICO::
read(CFile *file, CImagePtr &image)
{
  file->rewind();

  CImageICOHeader header;

  file->read((uchar *) &header, sizeof(header));

  if (header.type != 1)
    return false;

  CImageICOCursor      cursor;
  CImageICOInfoHeader  info_header;
  CImageICOColor      *colors;

  uint *data;
  uint  old_pos;
  uint  num_data;
  uint  bytes_per_line;
  uint  i, j, k, pad;
  uint  cursor_width, cursor_height;

  for (i = 0; i < header.count; ++i) {
    file->read((uchar *) &cursor, sizeof(cursor));

    int num_colors = cursor.num_colors;

    //----

    old_pos = file->getPos();

    file->setPos(cursor.offset);

    //----

    file->read((uchar *) &info_header, sizeof(info_header));

    if (info_header.size != 40) {
      CImage::errorMsg("Invalid Header Size");
      goto next;
    }

    if (info_header.compression != 0) {
      CImage::errorMsg("Compressed ICO not supported");
      goto next;
    }

    //-----

    if (info_header.bitcount != 32) {
      if (num_colors == 0)
        num_colors = 256;
    }
    else
      num_colors = 0;

    if (num_colors > 0) {
      colors = new CImageICOColor [num_colors];

      file->read((uchar *) colors, num_colors*sizeof(CImageICOColor));
    }
    else
      colors = 0;

    //-----

    cursor_width  = cursor.width;
    cursor_height = cursor.height;

    if (cursor_width  == 0) cursor_width  = 256;
    if (cursor_height == 0) cursor_height = 256;

    if      (info_header.bitcount == 32) {
      // data is 256*256 image rgb values followed by mask
      num_data = cursor_width*cursor_height;

      data = new uint [num_data];

      j = cursor_width*cursor_height;

      for (k = 0; k < cursor_height; ++k) {
        j -= cursor_width;

        file->read((uchar *) &data[j], 4*cursor_width);
      }
    }
    else if (info_header.bitcount == 8) {
      num_data = cursor_width*cursor_height;

      data = new uint [num_data];

      bytes_per_line = (cursor_width + 3)/4;

      pad = bytes_per_line*4 - cursor_width;

      std::vector<uchar> buffer;

      buffer.resize(cursor_width);

      j = cursor_width*cursor_height;

      for (k = 0; k < cursor_height; ++k) {
        j -= cursor_width;

        file->read(&buffer[0], cursor_width);

        uint  *p1 = &data[j];
        uchar *p2 = &buffer[0];

        for (k = 0; k < cursor_width; ++k)
          *p1++ = *p2++;

        int pad1 = pad;

        while (pad1--)
          file->read(&buffer[0], 1);
      }
    }
    else if (info_header.bitcount == 4) {
      num_data = cursor_width*cursor_height;

      data = new uint [num_data];

      bytes_per_line = (cursor_width + 7)/8;

      uint width1 = 4*bytes_per_line;

      std::vector<uchar> buffer1;

      buffer1.resize(width1);

      j = cursor_width*cursor_height;

      for (k = 0; k < cursor_height; ++k) {
        j -= cursor_width;

        file->read(&buffer1[0], width1);

        uint k1 = 0;
        uint k2 = j;

        for ( ; k1 < uint(cursor_width/2); k1++) {
          data[k2++] = (buffer1[k1] & 0xF0) >> 4;
          data[k2++] = (buffer1[k1] & 0x0F) >> 0;
        }

        if (k1 < uint(cursor_width + 1)/2)
          data[k2++] = (buffer1[k1] & 0xF0) >> 4;
      }
    }
    else {
      data = new uint [cursor_width*cursor_height];

      int width1 = ((cursor_width + 31)/32)*32;

      int width2 = width1/8;

      std::vector<uchar> buffer1;

      buffer1.resize(width2);

      int j = cursor_width*cursor_height;

      for (k = 0; k < cursor_height; ++k) {
        j -= cursor_width;

        file->read(&buffer1[0], width2);

        uint k1 = 0;
        uint k2 = j;

        for ( ; k1 < uint(cursor_width/8); k1++) {
          data[k2++] = buffer1[k1] & 0x80;
          data[k2++] = buffer1[k1] & 0x40;
          data[k2++] = buffer1[k1] & 0x20;
          data[k2++] = buffer1[k1] & 0x10;
          data[k2++] = buffer1[k1] & 0x08;
          data[k2++] = buffer1[k1] & 0x04;
          data[k2++] = buffer1[k1] & 0x02;
          data[k2++] = buffer1[k1] & 0x01;
        }

        if (k1 < uint(cursor_width + 7)/8) {
          data[k2++] = buffer1[k1] & 0x80;
          data[k2++] = buffer1[k1] & 0x40;
          data[k2++] = buffer1[k1] & 0x20;
          data[k2++] = buffer1[k1] & 0x10;
          data[k2++] = buffer1[k1] & 0x08;
          data[k2++] = buffer1[k1] & 0x04;
          data[k2++] = buffer1[k1] & 0x02;
          data[k2++] = buffer1[k1] & 0x01;
        }
      }
    }

    //-----

    image->setType(CFILE_TYPE_IMAGE_ICO);

    image->setDataSize(cursor_width, cursor_height);

    if (num_colors > 0) {
      for (int k = 0; k < num_colors; ++k)
        image->addColorI(colors[k].r, colors[k].g, colors[k].b);

      image->setColorIndexData(data);
    }
    else
      image->setRGBAData(data);

    break;

    //-----

 next:
    file->setPos(old_pos);
  }

  return true;
}

bool
CImageICO::
readHeader(CFile *file, CImagePtr &image)
{
  file->rewind();

  CImageICOHeader header;

  file->read((uchar *) &header, sizeof(header));

  if (header.type != 1)
    return false;

  if (header.count < 1)
    return false;

  CImageICOCursor cursor;

  file->read((uchar *) &cursor, sizeof(cursor));

  file->setPos(cursor.offset);

  CImageICOInfoHeader info_header;

  file->read((uchar *) &info_header, sizeof(info_header));

  if (info_header.size != 40 || info_header.compression != 0)
    return false;

  //------

  image->setType(CFILE_TYPE_IMAGE_ICO);

  uint cursor_width  = cursor.width;
  uint cursor_height = cursor.height;

  if (cursor_width  == 0) cursor_width  = 256;
  if (cursor_height == 0) cursor_height = 256;

  image->setSize(cursor.width, cursor.height);

  return true;
}

bool
CImageICO::
write(CFile *file, CImagePtr image)
{
  int depth = 8;

  if (! image->hasColormap())
    depth = 24;

  //------

  int data_size = 0;

  if      (depth == 24) {
    int line_width = 3*image->getWidth();

    int pad = (4 - (line_width % 4)) & 0x03;

    data_size = (line_width + pad)*image->getHeight();
  }
  else if (depth == 8) {
    int bytes_per_line = (image->getWidth() + 3)/4;

    int pad = bytes_per_line*4 - image->getWidth();

    data_size = (image->getWidth() + pad)*image->getHeight();
  }
  else if (depth == 4) {
    int bytes_per_line = (image->getWidth() + 7)/8;

    data_size = 4*bytes_per_line*image->getHeight();
  }
  else if (depth == 1) {
    int width1 = ((image->getWidth() + 31)/32)*32;
    int width2 = width1/8;

    data_size = width2*image->getHeight();
  }
  else
    return false;

  int data_size1 = 0;

  {
    int width1 = ((image->getWidth() + 31)/32)*32;
    int width2 = width1/8;

    data_size1 = width2*image->getHeight();
  }

  //------

  int num_colors = 0;

  if (image->hasColormap())
    num_colors = 1 << depth;

  //------

  int bitmap_header_size = 40;

  int color_data_size = 0;

  if (depth == 1 || depth == 4 || depth == 8)
    color_data_size = 4*num_colors;

  int file_size = bitmap_header_size + color_data_size +
                  data_size + data_size1;

  //------

  int planes         = 1;
  int bits_per_pixel = depth;

  //-------

  // ICONDIR

  writeShort(file, 0); // idReserved (0)
  writeShort(file, 1); // idType     (1)
  writeShort(file, 1); // idCount    (1 image)

  // ICONDIRENTRY

  writeByte(file, image->getWidth ()); // bWidth
  writeByte(file, image->getHeight()); // bHeight

  writeByte(file, num_colors); // bColorCount

  writeByte(file, 0); // bReserved (0)

  writeShort(file, planes); // wPlanes (1)

  writeShort(file, bits_per_pixel); // wBitCount

  writeInteger(file, file_size); // dwBytesInRes
  writeInteger(file, 22       ); // dwImageOffset

  //---------

  // BITMAPINFOHEADER

  writeInteger(file, bitmap_header_size    ); // biSize
  writeInteger(file,   image->getWidth ()  ); // biWidth
  writeInteger(file, 2*image->getHeight()  ); // biHeight
  writeShort  (file, planes                ); // biPlanes
  writeShort  (file, bits_per_pixel        ); // biBitCount
  writeInteger(file, 0                     ); // biCompression
  writeInteger(file, data_size + data_size1); // biSizeImage
  writeInteger(file, 0                     ); // biXPelsPerMeter
  writeInteger(file, 0                     ); // biYPelsPerMeter
  writeInteger(file, 0                     ); // biClrUsed
  writeInteger(file, 0                     ); // biClrImportant

  //---------

  // RGBQUAD

  if (image->hasColormap()) {
    int i = 0;

    uint r, g, b, a;

    for ( ; i < image->getNumColors(); ++i) {
      image->getColorRGBAI(i, &r, &g, &b, &a);

      if (CImage::isConvertTransparent(a/255.0))
        CImage::getConvertBg().getRGBAI(&r, &g, &b, &a);

      writeByte(file, b);
      writeByte(file, g);
      writeByte(file, r);
      writeByte(file, 0);
    }

    for ( ; i < num_colors; ++i) {
      writeByte(file, 0);
      writeByte(file, 0);
      writeByte(file, 0);
      writeByte(file, 0);
    }
  }

  //---------

  // icXOR

  if      (depth == 24) {
    int line_width  = 3*image->getWidth();
    int line_width1 = image->getWidth();

    int num_data = line_width1*image->getHeight();

    std::vector<uchar> buffer;

    buffer.resize(line_width);

    int pad = (4 - (line_width % 4)) & 0x03;

    int j = num_data;

    uint r, g, b, a;

    for (uint i = 0; i < image->getHeight(); ++i) {
      j -= line_width1;

      uchar *p = &buffer[0];

      int ind = j;

      for (uint k = 0; k < image->getWidth(); ++k, ++ind) {
        image->getRGBAPixelI(ind, &r, &g, &b, &a);

        *p++ = b;
        *p++ = g;
        *p++ = r;
      }

      file->write(&buffer[0], line_width);

      int pad1 = pad;

      while (pad1--)
        file->write('\0');
    }
  }
  else if (depth == 8) {
    std::vector<uchar> buffer;

    buffer.resize(image->getWidth());

    int bytes_per_line = (image->getWidth() + 3)/4;

    int pad = bytes_per_line*4 - image->getWidth();

    int j = image->getWidth()*image->getHeight();

    for (uint i = 0; i < image->getHeight(); ++i) {
      j -= image->getWidth();

      uchar *p = &buffer[0];

      int ind = j;

      for (uint k = 0; k < image->getWidth(); ++k, ++ind) {
        int pixel = image->getColorIndexPixel(ind);

        *p++ = (uchar) pixel;
      }

      file->write(&buffer[0], image->getWidth());

      int pad1 = pad;

      while (pad1--)
        file->write('\0');
    }
  }
  else if (depth == 4) {
    int bytes_per_line = (image->getWidth() + 7)/8;

    int width1 = 4*bytes_per_line;

    std::vector<uchar> buffer;

    buffer.resize(width1);

    int j = image->getWidth()*image->getHeight();

    for (uint i = 0; i < image->getHeight(); ++i) {
      j -= image->getWidth();

      uint k1 = 0;
      uint k2 = j;

      for ( ; k1 < image->getWidth()/2; k1++) {
        int pixel1 = image->getColorIndexPixel(k2    );
        int pixel2 = image->getColorIndexPixel(k2 + 1);

        buffer[k1] = (uchar) ((pixel1 << 4) | pixel2);

        k2 += 2;
      }

      if (k1 < (image->getWidth() + 1)/2) {
        int pixel1 = image->getColorIndexPixel(k2);

        buffer[k1] = (uchar) (pixel1 << 4);

        k2++;
      }

      file->write(&buffer[0], width1);
    }
  }
  else {
    int width1 = ((image->getWidth() + 31)/32)*32;

    int width2 = width1/8;

    std::vector<uchar> buffer;

    buffer.resize(width2);

    int j = image->getWidth()*image->getHeight();

    for (uint i = 0; i < image->getHeight(); ++i) {
      j -= image->getWidth();

      uint k1 = 0;
      uint k2 = j;

      for ( ; k1 < image->getWidth()/8; k1++) {
        int pixel1 = image->getColorIndexPixel(k2    );
        int pixel2 = image->getColorIndexPixel(k2 + 1);
        int pixel3 = image->getColorIndexPixel(k2 + 2);
        int pixel4 = image->getColorIndexPixel(k2 + 3);
        int pixel5 = image->getColorIndexPixel(k2 + 4);
        int pixel6 = image->getColorIndexPixel(k2 + 5);
        int pixel7 = image->getColorIndexPixel(k2 + 6);
        int pixel8 = image->getColorIndexPixel(k2 + 7);

        buffer[k1] = (uchar)
         (((pixel1 << 7) & 0x80) | ((pixel2 << 6) & 0x40) |
          ((pixel3 << 5) & 0x20) | ((pixel4 << 4) & 0x10) |
          ((pixel5 << 3) & 0x08) | ((pixel6 << 2) & 0x04) |
          ((pixel7 << 1) & 0x02) | ((pixel8 << 0) & 0x01));

        k2 += 8;
      }

      if (k1 < (image->getWidth() + 7)/8) {
        int k3 = image->getWidth() % 8;

        int pixel1 = k3 > 0 ? image->getColorIndexPixel(k2    ) : 0;
        int pixel2 = k3 > 1 ? image->getColorIndexPixel(k2 + 1) : 0;
        int pixel3 = k3 > 2 ? image->getColorIndexPixel(k2 + 2) : 0;
        int pixel4 = k3 > 3 ? image->getColorIndexPixel(k2 + 3) : 0;
        int pixel5 = k3 > 4 ? image->getColorIndexPixel(k2 + 4) : 0;
        int pixel6 = k3 > 5 ? image->getColorIndexPixel(k2 + 5) : 0;
        int pixel7 = k3 > 6 ? image->getColorIndexPixel(k2 + 6) : 0;
        int pixel8 = k3 > 7 ? image->getColorIndexPixel(k2 + 7) : 0;

        buffer[k1] = (uchar)
         (((pixel1 << 7) & 0x80) | ((pixel2 << 6) & 0x40) |
          ((pixel3 << 5) & 0x20) | ((pixel4 << 4) & 0x10) |
          ((pixel5 << 3) & 0x08) | ((pixel6 << 2) & 0x04) |
          ((pixel7 << 1) & 0x02) | ((pixel8 << 0) & 0x01));
      }

      file->write(&buffer[0], width2);
    }
  }

  //---------

  // icAnd

  {
    int width1 = ((image->getWidth() + 31)/32)*32;

    int width2 = width1/8;

    std::vector<uchar> buffer;

    buffer.resize(width2);

    int j = image->getWidth()*image->getHeight();

    for (uint i = 0; i < image->getHeight(); ++i) {
      j -= image->getWidth();

      uint k1 = 0;
      uint k2 = j;

      for ( ; k1 < image->getWidth()/8; k1++) {
        int value1 = (image->isTransparent(k2    ) ? 1 : 0);
        int value2 = (image->isTransparent(k2 + 1) ? 1 : 0);
        int value3 = (image->isTransparent(k2 + 2) ? 1 : 0);
        int value4 = (image->isTransparent(k2 + 3) ? 1 : 0);
        int value5 = (image->isTransparent(k2 + 4) ? 1 : 0);
        int value6 = (image->isTransparent(k2 + 5) ? 1 : 0);
        int value7 = (image->isTransparent(k2 + 6) ? 1 : 0);
        int value8 = (image->isTransparent(k2 + 7) ? 1 : 0);

        buffer[k1] = (uchar)
         (((value1 << 7) & 0x80) | ((value2 << 6) & 0x40) |
          ((value3 << 5) & 0x20) | ((value4 << 4) & 0x10) |
          ((value5 << 3) & 0x08) | ((value6 << 2) & 0x04) |
          ((value7 << 1) & 0x02) | ((value8 << 0) & 0x01));

        k2 += 8;
      }

      if (k1 < (image->getWidth() + 7)/8) {
        int k3 = image->getWidth() % 8;

        int value1 = k3 > 0 ? (image->isTransparent(k2    ) ? 1 : 0) : 0;
        int value2 = k3 > 1 ? (image->isTransparent(k2 + 1) ? 1 : 0) : 0;
        int value3 = k3 > 2 ? (image->isTransparent(k2 + 2) ? 1 : 0) : 0;
        int value4 = k3 > 3 ? (image->isTransparent(k2 + 3) ? 1 : 0) : 0;
        int value5 = k3 > 4 ? (image->isTransparent(k2 + 4) ? 1 : 0) : 0;
        int value6 = k3 > 5 ? (image->isTransparent(k2 + 5) ? 1 : 0) : 0;
        int value7 = k3 > 6 ? (image->isTransparent(k2 + 6) ? 1 : 0) : 0;
        int value8 = k3 > 7 ? (image->isTransparent(k2 + 7) ? 1 : 0) : 0;

        buffer[k1] = (uchar)
         (((value1 << 7) & 0x80) | ((value2 << 6) & 0x40) |
          ((value3 << 5) & 0x20) | ((value4 << 4) & 0x10) |
          ((value5 << 3) & 0x08) | ((value6 << 2) & 0x04) |
          ((value7 << 1) & 0x02) | ((value8 << 0) & 0x01));
      }

      file->write(&buffer[0], width2);
    }
  }

  //---------

  return true;
}

void
CImageICO::
writeInteger(CFile *file, int data)
{
  uint i = data;

  file->write((char) ( i        & 0xff));
  file->write((char) ((i >>  8) & 0xff));
  file->write((char) ((i >> 16) & 0xff));
  file->write((char) ((i >> 24) & 0xff));
}

void
CImageICO::
writeShort(CFile *file, int data)
{
  ushort s = data;

  file->write((char) ( s       & 0xff));
  file->write((char) ((s >> 8) & 0xff));
}

void
CImageICO::
writeByte(CFile *file, int data)
{
  uchar c = data;

  file->write((char) (c & 0xff));
}
