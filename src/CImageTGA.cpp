#include <CImageLib.h>
#include <CImageTGA.h>

struct TGAHeader {
  uchar  id_len;
  uchar  has_cmap;
  uchar  image_type;
  ushort cmap_start;
  ushort cmap_len;
  uchar  cmap_byte_size;
  ushort x, y;
  ushort width, height;
  uchar  depth;
  uchar  image_alpha;
  uchar  image_origin;
  uchar  image_pad;
};

enum TGAType {
  COLOR_MAPPED,
  TRUE_COLOR,
  BLACK_AND_WHITE
};

bool
CImageTGA::
read(CFile *file, CImagePtr &image)
{
  TGAHeader header;

  file->rewind();

  if (! readHeader(file, image, &header))
    return false;

  std::string id;

  if (header.id_len) {
    char buffer[256];

    if (! file->read((uchar *) buffer, header.id_len))
      return false;

    buffer[header.id_len] = '\0';

    id = buffer;
  }

  if (header.has_cmap) {
    uint color_bytes = header.cmap_byte_size/8;
    uint cmap_len    = header.cmap_len*color_bytes;

    uchar *cmap = new uchar [cmap_len];

    if (! file->read(cmap, cmap_len))
      return false;

    // add colors

    delete [] cmap;
  }

  //------

  TGAType type = COLOR_MAPPED;

  bool compressed = (header.image_type & 0x08);

  switch (header.image_type & 0x07) {
    case 0: // no image data
      return false;
    case 1: // uncompressed, color mapped
      type = COLOR_MAPPED;
      break;
    case 2: // uncompressed, true color
      type = TRUE_COLOR;
      break;
    case 3: // uncompressed, black and white
      type = BLACK_AND_WHITE;
      break;
    default:
      return false;
  }

  assert(type != -1);

  uint pixel_size = (header.depth >> 3);

  int   num_data = header.width*header.height;
  uint *data     = new uint [num_data];

  int x   = 0;
  int y   = header.height - 1;
  int pos = header.width*y;

  int r, g, b, a;

  while (y >= 0) {
    if (! compressed) {
      a = 255;

      if      (pixel_size == 1) {
        b = file->getC();
        g = b;
        r = b;
      }
      else if (pixel_size == 3) {
        b = file->getC();
        g = file->getC();
        r = file->getC();
      }
      else if (pixel_size == 4) {
        b = file->getC();
        g = file->getC();
        r = file->getC();
            file->getC();
      }

      if (r == EOF || g == EOF || b == EOF || a == EOF)
        return false;

      data[pos++] = image->rgbaToPixelI(r, g, b, a);

      ++x;

      if (x >= header.width) {
        x = 0;

        --y;

        pos = header.width*y;
      }
    }
    else {
      int c = file->getC();

      int size = 1 + (c & 0x7f);

      if (c & 0x80) { // Run-length packet
        a = 255;

        if      (pixel_size == 1) {
          b = file->getC();
          g = b;
          r = b;
        }
        else if (pixel_size == 3) {
          b = file->getC();
          g = file->getC();
          r = file->getC();
        }
        else if (pixel_size == 4) {
          b = file->getC();
          g = file->getC();
          r = file->getC();
              file->getC();
        }

        if (r == EOF || g == EOF || b == EOF || a == EOF)
          return false;

        for (int i = 0; i < size; ++i) {
          data[pos++] = image->rgbaToPixelI(r, g, b, a);

          ++x;

          if (x >= header.width) {
            x = 0;

            --y;

            pos = header.width*y;
          }
        }
      }
      else { // Non run-length packet
        for (int i = 0; i < size; ++i) {
          a = 255;

          if      (pixel_size == 1) {
            b = file->getC();
            g = b;
            r = b;
          }
          else if (pixel_size == 3) {
            b = file->getC();
            g = file->getC();
            r = file->getC();
          }
          else if (pixel_size == 4) {
            b = file->getC();
            g = file->getC();
            r = file->getC();
                file->getC();
          }

          if (r == EOF || g == EOF || b == EOF || a == EOF)
            return false;

          data[pos++] = image->rgbaToPixelI(r, g, b, a);

          ++x;

          if (x >= header.width) {
            x = 0;

            --y;

            pos = header.width*y;
          }
        }
      }
    }
  }

  //------

  image->setType(CFILE_TYPE_IMAGE_TGA);

  image->setDataSize(header.width, header.height);

  image->setRGBAData(data);

  delete [] data;

  return true;
}

bool
CImageTGA::
readHeader(CFile *file, CImagePtr &image)
{
  TGAHeader header;

  file->rewind();

  if (! readHeader(file, image, &header))
    return false;

  //------

  image->setType(CFILE_TYPE_IMAGE_TGA);

  image->setSize(header.width, header.height);

  //------

  return true;
}

bool
CImageTGA::
readHeader(CFile *file, CImagePtr &, TGAHeader *header)
{
  uchar buffer[18];

  if (! file->read(buffer, 18))
    return false;

  header->id_len         = buffer[ 0];
  header->has_cmap       = buffer[ 1];
  header->image_type     = buffer[ 2];
  header->cmap_start     = buffer[ 3] + (buffer[ 4] << 8);
  header->cmap_len       = buffer[ 5] + (buffer[ 6] << 8);
  header->cmap_byte_size = buffer[ 7];
  header->x              = buffer[ 8] + (buffer[ 9] << 8);
  header->y              = buffer[10] + (buffer[11] << 8);
  header->width          = buffer[12] + (buffer[13] << 8);
  header->height         = buffer[14] + (buffer[15] << 8);
  header->depth          = buffer[16];
  header->image_alpha    =  buffer[17] & 0x0F;
  header->image_origin   = (buffer[17] & 0x30) >> 4;
  header->image_pad      = (buffer[17] & 0xC0) >> 6;

  return true;
}

bool
CImageTGA::
write(CFile *, CImagePtr)
{
  return false;
}
