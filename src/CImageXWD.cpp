#include <CImageLib.h>
#include <CImageXWD.h>
#include <CThrow.h>
#include <CStrUtil.h>

#include <XWDFile.h>

static int xwd_bits_used  = -1;
static int xwd_pixel_mask = -1;

bool
CImageXWD::
read(CFile *file, CImagePtr &image)
{
  bool swap_flag = false;

  file->rewind();

  XWDFileHeader hdr;

  readHeader(file, image, &hdr, &swap_flag);

  //------

  // Read Color Map

  CRGBA *colors = 0;

  if (hdr.ncolors > 0) {
    XWDColor *xcolors = new XWDColor [hdr.ncolors];

    file->read((uchar *) xcolors, sizeof(XWDColor)*hdr.ncolors);

    if (swap_flag) {
      for (int i = 0; i < (int) hdr.ncolors; ++i) {
        xcolors[i].r = image->swapBytes16(xcolors[i].r);
        xcolors[i].g = image->swapBytes16(xcolors[i].g);
        xcolors[i].b = image->swapBytes16(xcolors[i].b);
      }
    }

    if (hdr.pixmap_depth <= 8) {
      colors = new CRGBA [hdr.ncolors];

      for (int i = 0; i < (int) hdr.ncolors; ++i)
        colors[i].setRGBA(xcolors[i].r/65535.0,
                          xcolors[i].g/65535.0,
                          xcolors[i].b/65535.0);
    }
    else
      hdr.ncolors = 0;

    delete [] xcolors;
  }

  //------

  /* Read Image Data */

  uint data_length = hdr.pixmap_height*hdr.bytes_per_line;

  uchar *data = new uchar [data_length];

  file->read(data, data_length);

  //------

  xwd_bits_used  = -1;
  xwd_pixel_mask = -1;

  data_length = hdr.pixmap_width*hdr.pixmap_height;

  int line_pad = hdr.bitmap_pad*
                 ((8*hdr.bytes_per_line + hdr.bitmap_pad - 1)/
                  hdr.bitmap_pad);

  line_pad /= hdr.bits_per_pixel;
  line_pad -= hdr.pixmap_width;

  uint *data1 = 0;

  if      (hdr.pixmap_depth == 1) {
    hdr.pixmap_depth = 8;

    data1 = new uint [data_length];

    uint *p   = data1;
    uint  pos = 0;

    for (uint y = 0; y < hdr.pixmap_height; ++y) {
      for (uint x = 0; x < hdr.pixmap_width; ++x, ++p) {
        uint pixel = getPixel(&hdr, data, &pos);

        *p = (pixel ? 0x01 : 0x00);
      }

      for (int x = 0; x < line_pad; ++x)
        getPixel(&hdr, data, &pos);
    }
  }
  else if (hdr.pixmap_depth == 8) {
    data1 = new uint [data_length];

    uint *p   = data1;
    uint  pos = 0;

    for (uint y = 0; y < hdr.pixmap_height; ++y) {
      for (uint x = 0; x < hdr.pixmap_width; ++x, ++p) {
        uint pixel = getPixel(&hdr, data, &pos);

        *p = pixel & 0xFF;
      }

      for (int x = 0; x < line_pad; ++x)
        getPixel(&hdr, data, &pos);
    }
  }
  else if (hdr.pixmap_depth == 16) {
    data1 = new uint [data_length];

    uint *p   = data1;
    uint  pos = 0;

    int r, g, b;

    for (uint y = 0; y < hdr.pixmap_height; ++y) {
      for (uint x = 0; x < hdr.pixmap_width; ++x, ++p) {
        uint pixel = getPixel(&hdr, data, &pos);

        r = (pixel >> 11) & 0x1F;
        g = (pixel >>  5) & 0x3F;
        b = (pixel >>  0) & 0x1F;

        *p = image->rgbaToPixel(r/31.0, g/63.0, b/31.0);
      }

      for (int x = 0; x < line_pad; ++x)
        getPixel(&hdr, data, &pos);
    }
  }
  else if (hdr.pixmap_depth == 24) {
    data1 = new uint [data_length];

    uint *p   = data1;
    uint  pos = 0;

    int r, g, b;

    for (uint y = 0; y < hdr.pixmap_height; ++y) {
      for (uint x = 0; x < hdr.pixmap_width; ++x, ++p) {
        uint pixel = getPixel(&hdr, data, &pos);

        r = (pixel >> 24) & 0xFF;
        g = (pixel >> 16) & 0xFF;
        b = (pixel >>  8) & 0xFF;

        *p = image->rgbaToPixelI(r, g, b);
      }

      for (int x = 0; x < line_pad; ++x)
        getPixel(&hdr, data, &pos);
    }
  }
  else {
    CImage::errorMsg(std::string("Invalid pixmap depth ") +
                     CStrUtil::toString((int) hdr.pixmap_depth));
    return false;
  }

  delete [] data;

  //------

  if (hdr.xoffset != 0) {
    CImage::errorMsg(std::string("Invalid x offset ") + CStrUtil::toString((int) hdr.xoffset));
    return false;
  }

  //------

  image->setType(CFILE_TYPE_IMAGE_XWD);

  image->setDataSize(hdr.pixmap_width, hdr.pixmap_height);

  if (hdr.pixmap_depth <= 8) {
    image->setColorIndexData(data1);

    for (uint i = 0; i < hdr.ncolors; ++i)
      image->addColor(colors[i]);
  }
  else
    image->setRGBAData(data1);

  delete [] data1;

  //------

  delete [] colors;

  return true;
}

bool
CImageXWD::
readHeader(CFile *file, CImagePtr &image)
{
  bool swap_flag = false;

  file->rewind();

  XWDFileHeader hdr;

  readHeader(file, image, &hdr, &swap_flag);

  //------

  image->setType(CFILE_TYPE_IMAGE_XWD);

  image->setSize(hdr.pixmap_width, hdr.pixmap_height);

  //------

  return true;
}

bool
CImageXWD::
readHeader(CFile *file, CImagePtr &image, XWDFileHeader *hdr, bool *swap_flag)
{
  file->read((uchar *) hdr, sz_XWDheader);

  *swap_flag = false;

  if (hdr->file_version != XWD_FILE_VERSION) {
    *swap_flag = true;

    hdr->header_size      = image->swapBytes32(hdr->header_size);
    hdr->file_version     = image->swapBytes32(hdr->file_version);
    hdr->pixmap_format    = image->swapBytes32(hdr->pixmap_format);
    hdr->pixmap_depth     = image->swapBytes32(hdr->pixmap_depth);
    hdr->pixmap_width     = image->swapBytes32(hdr->pixmap_width);
    hdr->pixmap_height    = image->swapBytes32(hdr->pixmap_height);
    hdr->xoffset          = image->swapBytes32(hdr->xoffset);
    hdr->byte_order       = image->swapBytes32(hdr->byte_order);
    hdr->bitmap_unit      = image->swapBytes32(hdr->bitmap_unit);
    hdr->bitmap_bit_order = image->swapBytes32(hdr->bitmap_bit_order);
    hdr->bitmap_pad       = image->swapBytes32(hdr->bitmap_pad);
    hdr->bits_per_pixel   = image->swapBytes32(hdr->bits_per_pixel);
    hdr->bytes_per_line   = image->swapBytes32(hdr->bytes_per_line);
    hdr->visual_class     = image->swapBytes32(hdr->visual_class);
    hdr->r_mask           = image->swapBytes32(hdr->r_mask);
    hdr->g_mask           = image->swapBytes32(hdr->g_mask);
    hdr->b_mask           = image->swapBytes32(hdr->b_mask);
    hdr->bits_per_rgb     = image->swapBytes32(hdr->bits_per_rgb);
    hdr->colormap_entries = image->swapBytes32(hdr->colormap_entries);
    hdr->ncolors          = image->swapBytes32(hdr->ncolors);
    hdr->window_width     = image->swapBytes32(hdr->window_width);
    hdr->window_height    = image->swapBytes32(hdr->window_height);
    hdr->window_x         = image->swapBytes32(hdr->window_x);
    hdr->window_y         = image->swapBytes32(hdr->window_y);
    hdr->window_bdrwidth  = image->swapBytes32(hdr->window_bdrwidth);

    if (hdr->file_version != XWD_FILE_VERSION) {
      CImage::errorMsg("Invalid X Window Dump File");
      return false;
    }
  }

  // Skip Extra Header Bytes

  if (hdr->header_size > sz_XWDheader)
    file->setPos(hdr->header_size);

  return true;
}

uint
CImageXWD::
getPixel(XWDFileHeader *hdr, uchar *data, uint *pos)
{
  static uchar  c;
  static ushort s;
  static uint   l;
  static uint   bit_shift;

  if (xwd_bits_used == -1) {
    xwd_bits_used = hdr->bitmap_unit;

    if (hdr->bits_per_pixel == 32)
      xwd_pixel_mask = 0xFFFFFFFF;
    else
      xwd_pixel_mask = (1 << hdr->bits_per_pixel) - 1;
  }

  if (xwd_bits_used == (int) hdr->bitmap_unit) {
    switch (hdr->bitmap_unit) {
      case 8:
        c = data[(*pos)++];

        break;
      case 16:
        if (hdr->byte_order == MSBFirst)
          s = ((data[(*pos) + 0] & 0xFF) << 8) |
              ((data[(*pos) + 1] & 0xFF)     );
        else
          s = ((data[(*pos) + 0] & 0xFF)     ) |
              ((data[(*pos) + 1] & 0xFF) << 8);

        *pos += 2;

        break;
      case 24:
      case 32:
        if (hdr->byte_order == MSBFirst)
          l = ((data[(*pos) + 0] & 0xFF) << 24) |
              ((data[(*pos) + 1] & 0xFF) << 16) |
              ((data[(*pos) + 2] & 0xFF) <<  8) |
              ((data[(*pos) + 3] & 0xFF)      );
        else
          l = ((data[(*pos) + 0] & 0xFF)      ) |
              ((data[(*pos) + 1] & 0xFF) <<  8) |
              ((data[(*pos) + 2] & 0xFF) << 16) |
              ((data[(*pos) + 3] & 0xFF) << 24);

        *pos += 4;

        break;
      default:
        break;
    }

    xwd_bits_used = 0;

    if (hdr->bitmap_bit_order == MSBFirst)
      bit_shift = hdr->bitmap_unit - hdr->bits_per_pixel;
    else
      bit_shift = 0;
  }

  uint pixel = 0;

  switch (hdr->bitmap_unit) {
    case 8:
      pixel = (c >> bit_shift) & xwd_pixel_mask;

      break;
    case 16:
      pixel = (s >> bit_shift) & xwd_pixel_mask;

      break;
    case 32:
      pixel = (l >> bit_shift) & xwd_pixel_mask;

      break;
    default:
      break;
  }

  if (hdr->bitmap_bit_order == MSBFirst)
    bit_shift -= hdr->bits_per_pixel;
  else
    bit_shift += hdr->bits_per_pixel;

  xwd_bits_used += hdr->bits_per_pixel;

  return pixel;
}

uchar *
CImageXWD::
expandData(uchar *data, int width, int height)
{
  int width1 = (width + 7)/8;

  int num_bytes = width*height;

  uchar *data1 = new uchar [num_bytes + 7];

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

bool
CImageXWD::
write(CFile *file, CImagePtr image)
{
  int bytes_per_line = 0;
  int visual_class   = 0;

  uint r_mask = 0;
  uint g_mask = 0;
  uint b_mask = 0;

  int bits_per_pixel = 8;
  int bits_per_rgb   = 8;
  int bitmap_unit    = 8;
  int bitmap_pad     = 8;

  int depth;

  if (image->hasColormap())
    depth = 8;
  else
    depth = 24;

  if      (depth == 24) {
    bytes_per_line = 4*image->getWidth();
    visual_class   = TrueColor;
    r_mask         = 0x0000ff;
    g_mask         = 0x00ff00;
    b_mask         = 0xff0000;
    bits_per_pixel = 32;
    bits_per_rgb   = 8;
    bitmap_unit    = 32;
    bitmap_pad     = 8;
  }
  else if (depth == 8) {
    bytes_per_line = image->getWidth();
    visual_class   = PseudoColor;
    bits_per_pixel = 8;
    bits_per_rgb   = 8;
    bitmap_unit    = 8;
    bitmap_pad     = 8;
  }
  else {
    bytes_per_line = (image->getWidth() + 7)/8;
    visual_class   = PseudoColor;
    bits_per_pixel = 8;
    bits_per_rgb   = 8;
    bitmap_unit    = 8;
    bitmap_pad     = 8;
  }

  XWDFileHeader *header = new XWDFileHeader;

  int format = ZPixmap;

  if (depth == 1)
    format = XYBitmap;

  header->header_size      = sizeof(XWDFileHeader);
  header->file_version     = XWD_FILE_VERSION;
  header->pixmap_format    = format;
  header->pixmap_depth     = depth;
  header->pixmap_width     = image->getWidth();
  header->pixmap_height    = image->getHeight();
  header->xoffset          = 0;
  header->byte_order       = LSBFirst;
  header->bitmap_unit      = bitmap_unit;
  header->bitmap_bit_order = LSBFirst;
  header->bitmap_pad       = bitmap_pad;
  header->bits_per_pixel   = bits_per_pixel;
  header->bytes_per_line   = bytes_per_line;
  header->visual_class     = visual_class;
  header->r_mask           = r_mask;
  header->g_mask           = g_mask;
  header->b_mask           = b_mask;
  header->bits_per_rgb     = bits_per_rgb;

  if (image->hasColormap()) {
    header->colormap_entries = 256;
    header->ncolors          = image->getNumColors();
  }
  else {
    header->colormap_entries = 0;
    header->ncolors          = 0;
  }

  header->window_width     = image->getWidth();
  header->window_height    = image->getHeight();
  header->window_x         = 0;
  header->window_y         = 0;
  header->window_bdrwidth  = 0;

  file->write((uchar *) header, sizeof(XWDFileHeader));

  delete [] header;

  if (header->ncolors > 0) {
    XWDColor *xcolors = new XWDColor [header->ncolors];

    uint r, g, b, a;

    for (uint i = 0; i < header->ncolors; ++i) {
      image->getColorRGBAI(i, &r, &g, &b, &a);

      xcolors[i].r = (r << 8) | r;
      xcolors[i].g = (g << 8) | g;
      xcolors[i].b = (b << 8) | b;
    }

    file->write((uchar *) xcolors,
                sizeof(XWDColor)*header->ncolors);

    delete [] xcolors;
  }

  int len = image->getHeight()*bytes_per_line;

  uchar *data = new uchar [len];

  uchar *p = data;

  if      (depth == 24) {
    int k = 0;

    uint r, g, b, a;

    for (uint i = 0; i < image->getHeight(); ++i)
      for (uint j = 0; j < image->getWidth(); ++j, p += 4, ++k) {
        image->getRGBAPixelI(k, &r, &g, &b, &a);

        p[0] = a;
        p[1] = b;
        p[2] = g;
        p[3] = r;
      }
  }
  else {
    int k = 0;

    for (uint i = 0; i < image->getHeight(); ++i)
      for (uint j = 0; j < image->getWidth(); ++j, ++p, ++k)
        *p = (uchar) image->getColorIndexPixel(k);
  }

  file->write(data, len);

  delete [] data;

  return true;
}
