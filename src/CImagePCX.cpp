#include <CImageLib.h>
#include <CImagePCX.h>

#include <cstring>

#define ENCODE_SHORT(a) (((a)[0]) + ((a)[1] << 8))

struct PCXHeader {
  uchar  identifier;
  uchar  version;
  uchar  encoding;
  uchar  bits_per_pixel;
  ushort x_start;
  ushort y_start;
  ushort x_end;
  ushort y_end;
  ushort horiz_res;
  ushort vert_res;
  uchar  palette[48];
  uchar  reserved1;
  uchar  num_bit_planes;
  ushort bytes_per_line;
  ushort palette_type;
  ushort horiz_screen_size;
  ushort vert_screen_size;
  uchar  reserved2[54];
};

bool
CImagePCX::
read(CFile *file, CImagePtr &image)
{
  PCXHeader header;

  //int file_size = file->getSize();

  file->rewind();

  if (! readHeader(file, image, &header))
    return false;

  //------

  int xsize = header.x_end - header.x_start + 1;
  int ysize = header.y_end - header.y_start + 1;

  //------

  int num_data = ysize*header.num_bit_planes*header.bytes_per_line;

  //if (num_data < 0 || num_data > file_size)
  //  return false;

  //------

  uint *data = new uint [num_data];

  int icount = sizeof(PCXHeader);
  int ocount = 0;

  while (true) {
    int c = file->getC();

    if (c == EOF)
      break;

    ++icount;

    int len = 1;

    if ((c & 0xC0) == 0xC0) {
      len = c & 0x3F;

      c = file->getC();

      if (c == EOF)
        break;

      ++icount;
    }

    for (int i = 0; i < len && ocount < num_data; ++i)
      data[ocount++] = c;

    if (ocount == num_data)
      break;
  }

  //------

  int num_colors = 1 << (header.num_bit_planes*header.bits_per_pixel);

  CRGBA *colors = new CRGBA [num_colors];

  if (num_colors <= 16) {
    for (int i = 0; i < num_colors; ++i)
      colors[i].setRGBAI(header.palette[3*i + 0],
                         header.palette[3*i + 1],
                         header.palette[3*i + 2]);
  }
  else {
    int c = file->getC();

    while (c != EOF && c != 0x0c)
      c = file->getC();

    int r, g, b;

    for (int i = 0; i < num_colors; ++i) {
      r = file->getC();
      g = file->getC();
      b = file->getC();

      colors[i].setRGBAI(r, g, b);
    }

    if (file->eof()) {
      // Gray scale

      double iscale = 1.0/num_colors;

      double g;

      for (int i = 0; i < num_colors; ++i) {
        g = i*iscale;

        colors[i].setRGBA(g, g, g, 1.0);
      }
    }
  }

  //------

  image->setType(CFILE_TYPE_IMAGE_PCX);

  image->setDataSize(xsize, ysize);

  if (header.bits_per_pixel <= 8) {
    image->setColorIndexData(data);

    for (int i = 0; i < num_colors; ++i)
      image->addColor(colors[i]);

    delete [] colors;
  }
  else
    image->setRGBAData(data);

  delete [] data;

  //------

  return true;
}

bool
CImagePCX::
readHeader(CFile *file, CImagePtr &image)
{
  PCXHeader header;

  file->rewind();

  if (! readHeader(file, image, &header))
    return false;

  //------

  int xsize = header.x_end - header.x_start + 1;
  int ysize = header.y_end - header.y_start + 1;

  //------

  image->setType(CFILE_TYPE_IMAGE_PCX);

  image->setSize(xsize, ysize);

  //------

  return true;
}

bool
CImagePCX::
readHeader(CFile *file, CImagePtr &, PCXHeader *header)
{
  uchar header_buffer[sizeof(PCXHeader)];

  memset(header_buffer, 0, sizeof(PCXHeader));

  if (! file->read(header_buffer, sizeof(PCXHeader)))
    return false;

  header->identifier     = header_buffer[0];
  header->version        = header_buffer[1];
  header->encoding       = header_buffer[2];
  header->bits_per_pixel = header_buffer[3];

  header->x_start   = ENCODE_SHORT(&header_buffer[ 4]);
  header->y_start   = ENCODE_SHORT(&header_buffer[ 6]);
  header->x_end     = ENCODE_SHORT(&header_buffer[ 8]);
  header->y_end     = ENCODE_SHORT(&header_buffer[10]);
  header->horiz_res = ENCODE_SHORT(&header_buffer[12]);
  header->vert_res  = ENCODE_SHORT(&header_buffer[14]);

  for (int i = 0; i < 48; ++i)
    header->palette[i] = header_buffer[16 + i];

  header->num_bit_planes = header_buffer[65];

  header->bytes_per_line    = ENCODE_SHORT(&header_buffer[66]);
  header->palette_type      = ENCODE_SHORT(&header_buffer[68]);
  header->horiz_screen_size = ENCODE_SHORT(&header_buffer[70]);
  header->vert_screen_size  = ENCODE_SHORT(&header_buffer[72]);

  if (header->version > 10 ||
      header->bits_per_pixel > 32 ||
      header->x_start > header->x_end ||
      header->y_start > header->y_end ||
      header->num_bit_planes > 32)
    return false;

  if (CImageState::getDebug()) {
    CImage::infoMsg("identifier        = " + std::to_string(header->identifier));
    CImage::infoMsg("version           = " + std::to_string(header->version));
    CImage::infoMsg("encoding          = " + std::to_string(header->encoding));
    CImage::infoMsg("bits_per_pixel    = " + std::to_string(header->bits_per_pixel));
    CImage::infoMsg("x_start           = " + std::to_string(header->x_start));
    CImage::infoMsg("y_start           = " + std::to_string(header->y_start));
    CImage::infoMsg("x_end             = " + std::to_string(header->x_end));
    CImage::infoMsg("y_end             = " + std::to_string(header->y_end));
    CImage::infoMsg("horiz_res         = " + std::to_string(header->horiz_res));
    CImage::infoMsg("vert_res          = " + std::to_string(header->vert_res));
    CImage::infoMsg("num_bit_planes    = " + std::to_string(header->num_bit_planes));
    CImage::infoMsg("bytes_per_line    = " + std::to_string(header->bytes_per_line));
    CImage::infoMsg("palette_type      = " + std::to_string(header->palette_type));
    CImage::infoMsg("horiz_screen_size = " + std::to_string(header->horiz_screen_size));
    CImage::infoMsg("vert_screen_size  = " + std::to_string(header->vert_screen_size));
  }

  return true;
}

bool
CImagePCX::
write(CFile *, CImagePtr)
{
  return false;
}
