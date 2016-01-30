#include <CImageLib.h>
#include <CImageXBM.h>
#include <CThrow.h>
#include <CStrUtil.h>

#include <cstring>

static char hex_chars[] = "0123456789abcdefABCDEF";

bool
CImageXBM::
read(CFile *file, CImagePtr &image)
{
  uint *data;
  int   x_hot;
  int   y_hot;
  uint  width;
  uint  height;

  file->rewind();

  bool flag = readBitmap(file, &width, &height, &data, &x_hot, &y_hot);

  if (! flag)
    return false;

  //------

  image->setType(CFILE_TYPE_IMAGE_XBM);

  image->setDataSize(width, height);

  image->setColorIndexData(data);

  //------

  image->addColor(1, 1, 1);
  image->addColor(0, 0, 0);

  //------

  return true;
}

bool
CImageXBM::
read(const uchar *data, CImagePtr &image, int width, int height)
{
  uint *data1 = expandBitmapData(data, width, height);

  //------

  image->setType(CFILE_TYPE_IMAGE_XBM);

  image->setDataSize(width, height);

  image->setColorIndexData(data1);

  //------

  image->addColor(1, 1, 1);
  image->addColor(0, 0, 0);

  //------

  return true;
}

bool
CImageXBM::
readHeader(CFile *file, CImagePtr &image)
{
  int  x_hot;
  int  y_hot;
  uint width;
  uint height;

  file->rewind();

  bool flag = readBitmap(file, &width, &height, 0, &x_hot, &y_hot);

  if (! flag)
    return false;

  //------

  image->setType(CFILE_TYPE_IMAGE_XBM);

  image->setSize(width, height);

  //------

  return true;
}

bool
CImageXBM::
readBitmap(CFile *file, uint *width, uint *height, uint **data, int *x_hot, int *y_hot)
{
  try {
    *width  = 0;
    *height = 0;
    *x_hot  = 0;
    *y_hot  = 0;

    if (data != 0)
      *data = 0;

    //------

    int text_len = file->getSize();

    //------

    std::vector<char> text;

    text.resize(text_len + 1);

    file->read((uchar *) &text[0], text_len);

    text[text_len] = '\0';

    //------

    uint i = 0;

    //------

    char *ptext = &text[0];

    skipSpace(ptext, &i);

    if (strncmp(&ptext[i], "#define", 7) != 0)
      return false;

    i += 7;

    skipSpace(ptext, &i);

    CStrUtil::skipNonSpace(ptext, &i);

    skipSpace(ptext, &i);

    int j = i;

    CStrUtil::skipNonSpace(ptext, &i);

    std::string str = std::string(&ptext[j], i - j);

    *width = CStrUtil::toInteger(str);

    //------

    skipSpace(ptext, &i);

    if (strncmp(&ptext[i], "#define", 7) != 0)
      return false;

    i += 7;

    skipSpace(ptext, &i);

    CStrUtil::skipNonSpace(ptext, &i);

    skipSpace(ptext, &i);

    j = i;

    CStrUtil::skipNonSpace(ptext, &i);

    str = std::string(&ptext[j], i - j);

    *height = CStrUtil::toInteger(str);

    //------

    if (data != 0) {
      int width1 = ((*width) + 7)/8;

      int num_bytes = (*width)*(*height);

      *data = new uint [num_bytes + 7];

      char hex_string[3];

      hex_string[2] = '\0';

      for (uint j = 0; j < *height; ++j) {
        int l = j*(*width);

        for (int k = 0; k < width1; ++k) {
          while (ptext[i] != '\0' &&
                 (ptext[i] != '0' || ptext[i + 1] != 'x' ||
                  strchr(hex_chars, ptext[i + 2]) == 0 ||
                  strchr(hex_chars, ptext[i + 3]) == 0))
            ++i;

          if (ptext[i] == '\0') {
            CImage::errorMsg("Invalid Hex Number in Data");
            return false;
          }

          hex_string[0] = ptext[i + 2];
          hex_string[1] = ptext[i + 3];

          uint hex_value;

          int no = sscanf(hex_string, "%x", &hex_value);

          if (no != 1) {
            CImage::errorMsg("Invalid Hex Number in Data");
            return false;
          }

          i += 4;

          (*data)[l++] = (hex_value & 0x01) != 0;
          (*data)[l++] = (hex_value & 0x02) != 0;
          (*data)[l++] = (hex_value & 0x04) != 0;
          (*data)[l++] = (hex_value & 0x08) != 0;
          (*data)[l++] = (hex_value & 0x10) != 0;
          (*data)[l++] = (hex_value & 0x20) != 0;
          (*data)[l++] = (hex_value & 0x40) != 0;
          (*data)[l++] = (hex_value & 0x80) != 0;
        }
      }
    }

    //------

    return true;
  }
  catch (...) {
    if (data != 0)
      delete [] *data;

    return false;
  }
}

uint *
CImageXBM::
expandBitmapData(const uchar *data, int width, int height)
{
  int width1 = (width + 7)/8;

  int num_bytes = width*height;

  uint *data1 = new uint [num_bytes + 7];

  int i = 0;

  for (int j = 0; j < height; ++j) {
    int l = j*width;

    for (int k = 0; k < width1; ++k, ++i) {
      data1[l++] = (data[i] & 0x01) >> 0;
      data1[l++] = (data[i] & 0x02) >> 1;
      data1[l++] = (data[i] & 0x04) >> 2;
      data1[l++] = (data[i] & 0x08) >> 3;
      data1[l++] = (data[i] & 0x10) >> 4;
      data1[l++] = (data[i] & 0x20) >> 5;
      data1[l++] = (data[i] & 0x40) >> 6;
      data1[l++] = (data[i] & 0x80) >> 7;
    }
  }

  return data1;
}

void
CImageXBM::
skipSpace(char *data, uint *i)
{
  CStrUtil::skipSpace(data, i);

  while (data[*i] != '\0' && (data[*i] == '/' && data[*i + 1] == '*')) {
    *i += 2;

    while (data[*i] != '\0' && ! (data[*i] == '*' && data[*i + 1] == '/'))
      (*i)++;

    if (data[*i] == '\0')
      return;

    *i += 2;

    CStrUtil::skipSpace(data, i);
  }
}

bool
CImageXBM::
write(CFile *file, CImagePtr image)
{
  std::string base = file->getBase();

  file->write("#define ");
  file->write(base.c_str());
  file->write("_width  ");
  file->write(CStrUtil::toString(image->getWidth()));
  file->write("\n");

  file->write("#define ");
  file->write(base.c_str());
  file->write("_height ");
  file->write(CStrUtil::toString(image->getHeight()));
  file->write("\n");

  file->write("\n");

  file->write("static const unsigned char\n");
  file->write(base.c_str());
  file->write("_bits[] = {\n");

  //------

  uint byte    = 0;
  int  bit_no  = 0;
  int  byte_no = 0;

  for (uint i = 0, k = 0; i < image->getHeight(); ++i) {
    for (uint j = 0; j < image->getWidth(); ++j, ++k) {
      double gray;

      image->getGrayPixel(k, &gray);

      int pixel = (gray > 0.5 ? 0 : 1);

      byte |= (pixel << bit_no);

      ++bit_no;

      if (bit_no == 8) {
        if (byte_no == 0)
          file->write("  0x");
        else
          file->write(" 0x");

        file->write(CStrUtil::toHexString(byte & 0xFF, 2));

        file->write(",");

        ++byte_no;

        if (byte_no >= 8) {
          file->write("\n");

          byte_no = 0;
        }

        byte   = 0;
        bit_no = 0;
      }
    }

    if (bit_no > 0) {
      if (byte_no == 0)
        file->write("  0x");
      else
        file->write(" 0x");

      file->write(CStrUtil::toHexString(byte & 0xFF, 2));

      file->write(",");

      ++byte_no;

      if (byte_no >= 8) {
        file->write("\n");

        byte_no = 0;
      }

      byte   = 0;
      bit_no = 0;
    }
  }

  if (byte_no > 0)
    file->write("\n");

  //------

  file->write("};\n");

  //------

  return true;
}
