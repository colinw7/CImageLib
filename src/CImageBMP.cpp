#include <CImageLib.h>
#include <CImageBMP.h>
#include <CThrow.h>

struct CImageBMPHeader {
  int file_size;
  int start_offset;
  int header_size;
  int width;
  int height;
  int depth;
  int compression;
  int num_colors;
};

bool
CImageBMP::
read(CFile *file, CImagePtr &image)
{
  bool rc = true;

  CRGBA *colors = 0;
  uint  *data   = 0;

  file->rewind();

  try {
    CImageBMPHeader header;

    if (! readHeader(file, &header))
      return false;

    //------

    readColors(file, &header, &colors);

    //------

    if (! readData(file, image, &header, &data))
      return false;

    //------

    image->setType(CFILE_TYPE_IMAGE_BMP);

    image->setDataSize(header.width, header.height);

    if (header.depth <= 8) {
      for (int i = 0; i < header.num_colors; ++i)
        image->addColor(colors[i]);

      image->setColorIndexData(data);
    }
    else
      image->setRGBAData(data);
  }
  catch (...) {
    CImage::errorMsg("Failed to read BMP file");

    delete [] colors;

    rc = false;
  }

  delete [] data;

  return rc;
}

bool
CImageBMP::
readHeader(CFile *file, CImagePtr &image)
{
  file->rewind();

  try {
    CImageBMPHeader header;

    if (! readHeader(file, &header))
      return false;

    //------

    image->setType(CFILE_TYPE_IMAGE_BMP);

    image->setSize(header.width, header.height);
  }
  catch (...) {
    CImage::errorMsg("Failed to read BMP file");
    return false;
  }

  return true;
}

bool
CImageBMP::
readHeader(CFile *file, CImageBMPHeader *header)
{
  uchar buffer[40];

  file->read(buffer, 14);

  if (buffer[0] != 'B' || buffer[1] != 'M') {
    CImage::errorMsg("Missing 'BM' at start of File");
    return false;
  }

  readInteger(&buffer[ 2], &header->file_size   );
  readInteger(&buffer[10], &header->start_offset);

  if (CImageState::getDebug()) {
    CImage::infoMsg("File Size    " + std::to_string(header->file_size));
    CImage::infoMsg("Start Offset " + std::to_string(header->start_offset));
  }

  //------

  file->read(buffer, 4);

  readInteger(&buffer[0], &header->header_size);

  if (CImageState::getDebug())
    CImage::infoMsg("Header Size " + std::to_string(header->header_size));

  if (header->header_size != 40) {
    CImage::errorMsg("Invalid Header Size");
    return false;
  }

  //------

  file->read(buffer, 36);

  readInteger(&buffer[ 0], &header->width      );
  readInteger(&buffer[ 4], &header->height     );
  readShort  (&buffer[10], &header->depth      );
  readInteger(&buffer[12], &header->compression);
  readInteger(&buffer[28], &header->num_colors );

  if (CImageState::getDebug()) {
    CImage::infoMsg("Width       " + std::to_string(header->width      ));
    CImage::infoMsg("Height      " + std::to_string(header->height     ));
    CImage::infoMsg("Depth       " + std::to_string(header->depth      ));
    CImage::infoMsg("Compression " + std::to_string(header->compression));
    CImage::infoMsg("Num Colors  " + std::to_string(header->num_colors ));
  }

  //------

  if (header->depth != 1 && header->depth != 4  &&
      header->depth != 8 && header->depth != 24) {
    CImage::errorMsg("BMP Depth " + std::to_string(header->depth) + " not supported");
    return false;
  }

  return true;
}

void
CImageBMP::
readColors(CFile *file, CImageBMPHeader *header, CRGBA **colors)
{
  if (header->depth == 1 || header->depth == 4 || header->depth == 8) {
    uchar buffer[1024];

    if (header->num_colors == 0)
      header->num_colors = 1 << header->depth;

    file->read(buffer, 4*header->num_colors);

    *colors = new CRGBA [header->num_colors];

    for (int i = 0; i < header->num_colors; ++i) {
      (*colors)[i].setRGBAI(buffer[i*4 + 2], buffer[i*4 + 1], buffer[i*4 + 0]);

      if (CImageState::getDebug())
        CImage::infoMsg("Color " + std::to_string(i + 1) + ") " +
                        std::string((char *) &buffer[i*4 + 2], 1) + "," +
                        std::string((char *) &buffer[i*4 + 1], 1) + "," +
                        std::string((char *) &buffer[i*4 + 0], 1) + "," +
                        std::string((char *) &buffer[i*4 + 3], 1));
    }
  }
  else
    header->num_colors = 0;
}

bool
CImageBMP::
readData(CFile *file, CImagePtr &image, CImageBMPHeader *header, uint **data)
{
  if     (header->depth == 24) {
    if (header->compression != 0) {
      CImage::errorMsg("Compression not supported for Depth " + std::to_string(header->depth));
      return false;
    }

    int line_width  = 3*header->width;
    int line_width1 = header->width;

    int num_data = line_width1*header->height;

    *data = new uint [num_data];

    std::vector<uchar> line_buffer;

    line_buffer.resize(line_width);

    int pad = (4 - (line_width % 4)) & 0x03;

    int j = num_data;

    for (int i = 0; i < header->height; ++i) {
      j -= line_width1;

      file->read(&line_buffer[0], line_width);

      int pad1 = pad;

      while (pad1--)
        file->read(&line_buffer[0], 1);

      uchar *p  = &line_buffer[0];
      uint  *p1 = &(*data)[j];

      for (int k = 0; k < header->width; ++k) {
        *p1 = image->rgbaToPixelI(p[2], p[1], p[0]);

        p += 3;

        p1++;
      }
    }
  }
  else if (header->depth == 8) {
    int num_data;

    if      (header->compression == 0)
          readCmp0Data8(file, header->width, header->height, data, &num_data);
    else if (header->compression == 1)
          readCmp1Data8(file, header->width, header->height, data, &num_data);
    else {
      CImage::errorMsg("Compression " + std::to_string(header->compression) +
                       " not supported for Depth " + std::to_string(header->depth));
      return false;
    }
  }
  else if (header->depth == 4) {
    if (header->compression != 0) {
      CImage::errorMsg("Compression not supported for Depth " + std::to_string(header->depth));
      return false;
    }

    int num_data = header->width*header->height;

    *data = new uint [num_data];

    int bytes_per_line = (header->width + 7)/8;

    int width1 = 4*bytes_per_line;

    std::vector<uchar> buffer1;

    buffer1.resize(width1);

    int j = header->width*header->height;

    for (int i = 0; i < header->height; ++i) {
      j -= header->width;

      file->read(&buffer1[0], width1);

      int k1 = 0;
      int k2 = j;

      for ( ; k1 < header->width/2; k1++) {
        (*data)[k2++] = (buffer1[k1] & 0xF0) >> 4;
        (*data)[k2++] = (buffer1[k1] & 0x0F) >> 0;
      }

      if (k1 < (header->width + 1)/2)
        (*data)[k2++] = (buffer1[k1] & 0xF0) >> 4;
    }

    header->depth = 8;
  }
  else {
    *data = new uint [header->width*header->height];

    int width1 = ((header->width + 31)/32)*32;

    int width2 = width1/8;

    std::vector<uchar> buffer1;

    buffer1.resize(width2);

    int j = header->width*header->height;

    for (int i = 0; i < header->height; ++i) {
      j -= header->width;

      file->read(&buffer1[0], width2);

      int k1 = 0;
      int k2 = j;

      for ( ; k1 < header->width/8; k1++) {
        (*data)[k2++] = buffer1[k1] & 0x80;
        (*data)[k2++] = buffer1[k1] & 0x40;
        (*data)[k2++] = buffer1[k1] & 0x20;
        (*data)[k2++] = buffer1[k1] & 0x10;
        (*data)[k2++] = buffer1[k1] & 0x08;
        (*data)[k2++] = buffer1[k1] & 0x04;
        (*data)[k2++] = buffer1[k1] & 0x02;
        (*data)[k2++] = buffer1[k1] & 0x01;
      }

      if (k1 < (header->width + 7)/8) {
        (*data)[k2++] = buffer1[k1] & 0x80;
        (*data)[k2++] = buffer1[k1] & 0x40;
        (*data)[k2++] = buffer1[k1] & 0x20;
        (*data)[k2++] = buffer1[k1] & 0x10;
        (*data)[k2++] = buffer1[k1] & 0x08;
        (*data)[k2++] = buffer1[k1] & 0x04;
        (*data)[k2++] = buffer1[k1] & 0x02;
        (*data)[k2++] = buffer1[k1] & 0x01;
      }
    }

    header->depth = 8;
  }

  return true;
}

void
CImageBMP::
readCmp0Data8(CFile *file, int width, int height, uint **data, int *num_data)
{
  *num_data = width*height;

  *data = new uint [*num_data];

  int bytes_per_line = (width + 3)/4;

  int pad = bytes_per_line*4 - width;

  std::vector<uchar> buffer;

  buffer.resize(width);

  int j = width*height;

  for (int i = 0; i < height; ++i) {
    j -= width;

    file->read(&buffer[0], width);

    uint  *p1 = &(*data)[j];
    uchar *p2 = &buffer[0];

    for (int k = 0; k < width; ++k)
      *p1++ = *p2++;

    int pad1 = pad;

    while (pad1--)
      file->read(&buffer[0], 1);
  }
}

void
CImageBMP::
readCmp1Data8(CFile *file, int width, int height, uint **data, int *num_data)
{
  *num_data = width*height;

  *data = new uint [*num_data];

  int line_num = height - 1;
  int char_num = 0;

  while (line_num > 0) {
    int pos = line_num*width + char_num;

    uchar c1, c2;

    file->read(&c1, 1);
    file->read(&c2, 1);

    if      (c1 != 0) {
      for (int i = 0; i < c1; ++i)
        (*data)[pos++] = c2;

      char_num += c1;
    }
    else if (c2 == 0) {
      if (char_num > 0) {
        for (int i = char_num; i < width; ++i)
          (*data)[pos++] = 0;

        char_num = 0;

        --line_num;
      }
    }
    else if (c2 == 1) {
      if (char_num > 0) {
        for (int i = char_num; i < width; ++i)
          (*data)[pos++] = 0;

        char_num = 0;

        --line_num;
      }

      for (int j = line_num; j >= 0; --j) {
        pos = line_num*width + char_num;

        for (int i = 0; i < width; ++i)
          (*data)[pos++] = 0;

        ++line_num;
      }
    }
    else if (c2 == 2) {
      uchar c3, c4;

      file->read(&c3, 1);
      file->read(&c4, 1);

      char_num += c3;
      line_num -= c4;
    }
    else {
      uchar c;

      for (int i = 0; i < c2; ++i) {
        file->read(&c, 1);

        (*data)[pos++] = c;
      }

      if (c2 & 1)
        file->read(&c, 1);

      char_num += c2;
    }

    while (char_num >= width) {
      char_num -= width;

      --line_num;
    }
  }
}

void
CImageBMP::
readInteger(uchar *buffer, int *integer)
{
  *integer = ((buffer[0] & 0xFF)      ) |
             ((buffer[1] & 0xFF) <<  8) |
             ((buffer[2] & 0xFF) << 16) |
             ((buffer[3] & 0xFF) << 24);
}

void
CImageBMP::
readShort(uchar *buffer, int *integer)
{
  *integer = ((buffer[0] & 0xFF)     ) |
             ((buffer[1] & 0xFF) << 8);
}

bool
CImageBMP::
write(CFile *file, CImagePtr image)
{
  int num_data = 0;

  int depth;

  if (image->hasColormap())
    depth = 8;
  else
    depth = 24;

  if      (depth == 24) {
    int line_width = 3*image->getWidth();

    int pad = (4 - (line_width % 4)) & 0x03;

    num_data = (line_width + pad)*image->getHeight();
  }
  else if (depth == 8) {
    num_data = image->getWidth()*image->getHeight();
  }
  else if (depth == 4) {
    int bytes_per_line = (image->getWidth() + 7)/8;

    num_data = 4*bytes_per_line*image->getHeight();
  }
  else if (depth == 1) {
    int width1 = ((image->getWidth() + 31)/32)*32;

    int width2 = width1/8;

    num_data = width2*image->getHeight();
  }
  else
    return false;

  int num_color_data = 0;

  if (depth == 1 || depth == 4 || depth == 8)
    num_color_data = 4*image->getNumColors();

  file->write('B');
  file->write('M');

  int file_size = 40 + num_color_data + num_data;

  writeInteger(file, file_size);

  writeShort(file, 0);
  writeShort(file, 0);

  int offset = 40 + num_color_data;

  writeInteger(file, offset);

  writeInteger(file, 40);

  int planes         = 1;
  int bits_per_pixel = depth;
  int compression    = 0;
  int bitmap_size    = image->getWidth()*image->getHeight();
  int horz_res       = 0;
  int vert_res       = 0;

  writeInteger(file, image->getWidth());
  writeInteger(file, image->getHeight());
  writeShort  (file, planes);
  writeShort  (file, bits_per_pixel);
  writeInteger(file, compression);
  writeInteger(file, bitmap_size);
  writeInteger(file, horz_res);
  writeInteger(file, vert_res);

  if (depth == 1 || depth == 4 || depth == 8) {
    writeInteger(file, image->getNumColors());
    writeInteger(file, image->getNumColors());

    uint r, g, b, a;

    for (int i = 0; i < image->getNumColors(); ++i) {
      image->getColorRGBAI(i, &r, &g, &b, &a);

      if (CImage::isConvertTransparent(a/255.0))
        CImage::getConvertBg().getRGBAI(&r, &g, &b, &a);

      writeByte(file, b);
      writeByte(file, g);
      writeByte(file, r);
      writeByte(file, 0);
    }
  }
  else {
    writeInteger(file, 0);
    writeInteger(file, 0);
  }

  uint width  = image->getWidth ();
  uint height = image->getHeight();

  if      (depth == 24) {
    int line_width  = 3*width;
    int line_width1 = width;

    int num_data = line_width1*height;

    std::vector<uchar> buffer;

    buffer.resize(line_width);

    int pad = (4 - (line_width % 4)) & 0x03;

    int j = num_data;

    for (uint i = 0; i < height; ++i) {
      j -= line_width1;

      uchar *p = &buffer[0];

      int ind = j;

      uint r, g, b, a;

      for (uint k = 0; k < width; ++k, ++ind) {
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

     buffer.resize(width);

    int bytes_per_line = (width + 3)/4;

    int pad = bytes_per_line*4 - width;

    int j = width*height;

    for (uint i = 0; i < height; ++i) {
      j -= width;

      uchar *p = &buffer[0];

      int ind = j;

      for (uint k = 0; k < width; ++k, ++ind) {
        int pixel = image->getColorIndexPixel(ind);

        *p++ = (uchar) pixel;
      }

      file->write(&buffer[0], width);

      int pad1 = pad;

      while (pad1--)
        file->write('\0');
    }
  }
  else if (depth == 4) {
    int bytes_per_line = (width + 7)/8;

    int width1 = 4*bytes_per_line;

    std::vector<uchar> buffer;

    buffer.resize(width1);

    int j = width*height;

    for (uint i = 0; i < height; ++i) {
      j -= width;

      uint k1 = 0;
      uint k2 = j;

      for ( ; k1 < width/2; k1++) {
        int pixel1 = image->getColorIndexPixel(k2    );
        int pixel2 = image->getColorIndexPixel(k2 + 1);

        buffer[k1] = (uchar) ((pixel1 << 4) | pixel2);

        k2 += 2;
      }

      if (k1 < (width + 1)/2) {
        int pixel1 = image->getColorIndexPixel(k2);

        buffer[k1] = (uchar) (pixel1 << 4);

        k2++;
      }

      file->write(&buffer[0], width1);
    }
  }
  else {
    int width1 = ((width + 31)/32)*32;

    int width2 = width1/8;

    std::vector<uchar> buffer;

    buffer.resize(width2);

    int j = width*height;

    for (uint i = 0; i < height; ++i) {
      j -= width;

      uint k1 = 0;
      uint k2 = j;

      for ( ; k1 < width/8; k1++) {
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

      if (k1 < (width + 7)/8) {
        int k3 = width % 8;

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

  return true;
}

void
CImageBMP::
writeInteger(CFile *file, int data)
{
  uint i = data;

  file->write((char) ( i        & 0xff));
  file->write((char) ((i >>  8) & 0xff));
  file->write((char) ((i >> 16) & 0xff));
  file->write((char) ((i >> 24) & 0xff));
}

void
CImageBMP::
writeShort(CFile *file, int data)
{
  ushort s = data;

  file->write((char) ( s       & 0xff));
  file->write((char) ((s >> 8) & 0xff));
}

void
CImageBMP::
writeByte(CFile *file, int data)
{
  uchar c = data;

  file->write((char) (c & 0xff));
}
