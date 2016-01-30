#include <CImageLib.h>
#include <CImageIFF.h>
#include <CThrow.h>

#include <cstring>

#define MAX_NUMBER_OF_COLORS 256
#define MAX_NUMBER_OF_COLOR_RANGES 24

struct IFFBitMapHeader {
  IFF_UWORD w;
  IFF_UWORD h;
  IFF_WORD  x;
  IFF_WORD  y;
  IFF_UBYTE BitPlanes;
  IFF_UBYTE Masking;
  IFF_UBYTE Compression;
  IFF_UBYTE PadByte;
  IFF_UWORD TransCol;
  IFF_UBYTE XAspect;
  IFF_UBYTE YAspect;
  IFF_WORD  Width;
  IFF_WORD  Height;
};

struct IFFColorRegister {
  IFF_UBYTE r;
  IFF_UBYTE g;
  IFF_UBYTE b;
};

struct IFFCommodoreAmiga {
  IFF_UWORD PadWord;
  IFF_UWORD ViewModes;
};

struct IFFColorRange {
  IFF_WORD  pad1;
  IFF_WORD  rate;
  IFF_WORD  active;
  IFF_UBYTE low;
  IFF_UBYTE high;
};

bool
CImageIFF::
read(CFile *file, CImagePtr &image)
{
  IFF_UBYTE *screen_memory = 0;

  try {
    IFFBitMapHeader    bitmap_header;
    char               header_name[5];
    uint              *screen_memory2;
    IFFCommodoreAmiga  commodore_amiga;
    IFF_ULONG          screen_memory_size;
    IFFColorRegister   cregs[MAX_NUMBER_OF_COLORS];
    IFFColorRange      color_ranges[MAX_NUMBER_OF_COLOR_RANGES];

    bool FORM_flag = false;
    bool ILBM_flag = false;
    bool BMHD_flag = false;
    bool CMAP_flag = false;
    bool CAMG_flag = false;
    bool CRNG_flag = false;
    bool BODY_flag = false;

    int num_colors       = 0;
    int num_color_ranges = 0;

    /*-----------*/

    file->rewind();

    while (readHeaderName(file, header_name)) {
      if      (strcmp(header_name, "FORM") == 0) {
        FORM_flag = readFORM(file);
      }
      else if (strcmp(header_name, "ILBM") == 0) {
        if (FORM_flag)
          ILBM_flag = readILBM(file);
      }
      else if (strcmp(header_name, "BMHD") == 0) {
        if (FORM_flag && ILBM_flag)
          BMHD_flag = readBMHD(file, &bitmap_header);
      }
      else if (strcmp(header_name, "CMAP") == 0) {
        if (FORM_flag && ILBM_flag && BMHD_flag)
          CMAP_flag = readCMAP(file, cregs, &num_colors);
      }
      else if (strcmp(header_name, "CAMG") == 0) {
        if (FORM_flag && ILBM_flag && BMHD_flag)
          CAMG_flag = readCAMG(file, &commodore_amiga);
      }
      else if (strcmp(header_name, "CRNG") == 0) {
        if (FORM_flag && ILBM_flag && BMHD_flag)
          CRNG_flag = readCRNG(file, &color_ranges[++num_color_ranges]);
      }
      else if (strcmp(header_name, "BODY") == 0) {
        if (FORM_flag && ILBM_flag && BMHD_flag)
          BODY_flag = readBody(file, &screen_memory, &screen_memory_size);
      }
      else {
        if (CImageState::getDebug())
          CImage::infoMsg("Unknown " + std::string(header_name));

        readUnknown(file);
      }
    }

    if (! FORM_flag || ! ILBM_flag || ! BMHD_flag || ! BODY_flag) {
      CImage::errorMsg("No FORM, ILBM, BMHD or BODY Section");
      return false;
    }

    if (! CAMG_flag) commodore_amiga.ViewModes = 0;

    if (! CRNG_flag) num_color_ranges = 0;

    /*-----------*/

    if (bitmap_header.Compression == 1) {
      IFF_UBYTE *screen_memory1 =
        decompressScreenMemory(screen_memory, screen_memory_size,
                               bitmap_header.w, bitmap_header.h,
                               bitmap_header.BitPlanes);

      delete [] screen_memory;

      screen_memory = screen_memory1;
    }

    /*-----------*/

    if (bitmap_header.BitPlanes <= 8) {
      IFF_UBYTE *screen_memory1 =
        convertScreenMemory8(screen_memory, bitmap_header.w, bitmap_header.h,
                             bitmap_header.BitPlanes);

      delete [] screen_memory;

      screen_memory = screen_memory1;
    }
    else {
      IFF_UBYTE *screen_memory1 =
        convertScreenMemory24(screen_memory, bitmap_header.w, bitmap_header.h);

      delete [] screen_memory;

      screen_memory = screen_memory1;
    }

    /*-----------*/

    CRGBA *colors;

    if      ((commodore_amiga.ViewModes & 0x0800) &&
             bitmap_header.BitPlanes == 6) {
      IFF_UBYTE *screen_memory1 =
        convertHAM(screen_memory, bitmap_header.w, bitmap_header.h,
                   cregs, &colors, &num_colors);

      delete [] screen_memory;

      screen_memory = screen_memory1;
    }
    else if ((commodore_amiga.ViewModes & 0x0800) &&
             bitmap_header.BitPlanes == 8) {
      IFF_UBYTE *screen_memory1 =
        convertHAM8(screen_memory, bitmap_header.w, bitmap_header.h,
                    cregs, &colors, &num_colors);

      delete [] screen_memory;

      screen_memory = screen_memory1;
    }
    else if (bitmap_header.BitPlanes == 24) {
      IFF_UBYTE *screen_memory1 =
        convert24Bit(screen_memory, bitmap_header.w, bitmap_header.h,
                     &colors, &num_colors);

      delete [] screen_memory;

      screen_memory = screen_memory1;
    }
    else {
      if (commodore_amiga.ViewModes & 0x0080) {
        for (int i = 0; i < 32; ++i) {
          cregs[i + 32].r = cregs[i].r/2;
          cregs[i + 32].g = cregs[i].g/2;
          cregs[i + 32].b = cregs[i].b/2;
        }
      }

      assert(CMAP_flag);

      colors = convertColors(cregs, num_colors);
    }

    screen_memory2 = new uint [bitmap_header.w*bitmap_header.h];

    for (int i = 0; i < bitmap_header.w*bitmap_header.h; ++i)
      screen_memory2[i] = screen_memory[i];

    delete [] screen_memory;

    /*-----------*/

    image->setType(CFILE_TYPE_IMAGE_IFF);

    image->setDataSize(bitmap_header.w, bitmap_header.h);

    image->setColorIndexData(screen_memory2);

    for (int i = 0; i < num_colors; ++i)
      image->addColor(colors[i]);

    delete [] screen_memory2;

    delete [] colors;
  }
  catch (...) {
    delete [] screen_memory;

    CImage::errorMsg("Failed to read IFF file");
    return false;
  }

  return true;
}

bool
CImageIFF::
readHeader(CFile *file, CImagePtr &image)
{
  try {
    IFFBitMapHeader bitmap_header;
    char            header_name[5];

    /*-----------*/

    file->rewind();

    bitmap_header.w = 0;
    bitmap_header.h = 0;

    while (readHeaderName(file, header_name)) {
      if      (strcmp(header_name, "FORM") == 0) {
        readFORM(file);
      }
      else if (strcmp(header_name, "ILBM") == 0) {
        readILBM(file);
      }
      else if (strcmp(header_name, "BMHD") == 0) {
        readBMHD(file, &bitmap_header);
      }
      else
        readUnknown(file);
    }

    /*-----------*/

    image->setType(CFILE_TYPE_IMAGE_IFF);

    image->setSize(bitmap_header.w, bitmap_header.h);
  }
  catch (...) {
    CImage::errorMsg("Failed to read IFF file");
    return false;
  }

  return true;
}

bool
CImageIFF::
readFORM(CFile *file)
{
  IFF_ULONG  header_length;

  if (CImageState::getDebug())
    CImage::infoMsg("FORM");

  if (! readHeaderLength(file, &header_length))
    return false;

  if (CImageState::getDebug())
    CImage::infoMsg("  " + std::to_string(header_length) + " bytes");

  return true;
}

bool
CImageIFF::
readILBM(CFile *)
{
  if (CImageState::getDebug())
    CImage::infoMsg("ILBM");

  return true;
}

bool
CImageIFF::
readBMHD(CFile *file, IFFBitMapHeader *bitmap_header)
{
  if (CImageState::getDebug())
    CImage::infoMsg("BMHD");

  IFF_ULONG header_length;

  if (! readHeaderLength(file, &header_length))
    return false;

  if (CImageState::getDebug())
    CImage::infoMsg("  " + std::to_string(header_length) + " bytes");

  if (sizeof(IFFBitMapHeader) != header_length)
    return false;

  if (! readStorage(file, header_length, (IFF_UBYTE *) bitmap_header))
    return false;

  convertWord(&bitmap_header->w);
  convertWord(&bitmap_header->h);
  convertWord((IFF_UWORD *) &bitmap_header->x);
  convertWord((IFF_UWORD *) &bitmap_header->y);
  convertWord(&bitmap_header->TransCol);
  convertWord((IFF_UWORD *) &bitmap_header->Width);
  convertWord((IFF_UWORD *) &bitmap_header->Height);

  if (CImageState::getDebug()) {
    CImage::infoMsg("  w           = " + std::to_string(bitmap_header->w));
    CImage::infoMsg("  h           = " + std::to_string(bitmap_header->h));
    CImage::infoMsg("  x           = " + std::to_string(bitmap_header->x));
    CImage::infoMsg("  y           = " + std::to_string(bitmap_header->y));
    CImage::infoMsg("  BitPlanes   = " + std::to_string(bitmap_header->BitPlanes));
    CImage::infoMsg("  Masking     = " + std::to_string(bitmap_header->Masking));
    CImage::infoMsg("  Compression = " + std::to_string(bitmap_header->Compression));
    CImage::infoMsg("  TransCol    = " + std::to_string(bitmap_header->TransCol));
    CImage::infoMsg("  XAspect     = " + std::to_string(bitmap_header->XAspect));
    CImage::infoMsg("  YAspect     = " + std::to_string(bitmap_header->YAspect));
    CImage::infoMsg("  Width       = " + std::to_string(bitmap_header->Width));
    CImage::infoMsg("  Height      = " + std::to_string(bitmap_header->Height));
  }

  return true;
}

bool
CImageIFF::
readCMAP(CFile *file, IFFColorRegister *cregs, int *num_colors)
{
  if (CImageState::getDebug())
    CImage::infoMsg("CMAP");

  IFF_ULONG header_length;

  if (! readHeaderLength(file, &header_length))
    return false;

  if (CImageState::getDebug())
    CImage::infoMsg("  " + std::to_string(header_length) + " bytes");

  std::vector<IFF_UBYTE> buffer;

  buffer.resize(header_length + 1);

  if (! readStorage(file, header_length, &buffer[0]))
    return false;

  *num_colors = header_length/3;

  for (int i = 0; i < *num_colors; ++i) {
    cregs[i].r = buffer[(3 * i) + 0];
    cregs[i].g = buffer[(3 * i) + 1];
    cregs[i].b = buffer[(3 * i) + 2];

    if (CImageState::getDebug())
      CImage::infoMsg("  "     + std::to_string(i) + ") " +
                      "Red "   + std::to_string(cregs[i].r >> 4) + ", " +
                      "Green " + std::to_string(cregs[i].g >> 4) + ", " +
                      "Blue "  + std::to_string(cregs[i].b >> 4));
  }

  return true;
}

bool
CImageIFF::
readCAMG(CFile *file, IFFCommodoreAmiga *commodore_amiga)
{
  if (CImageState::getDebug())
    CImage::infoMsg("CAMG");

  IFF_ULONG header_length;

  if (! readHeaderLength(file, &header_length))
    return false;

  if (CImageState::getDebug())
    CImage::infoMsg("  " + std::to_string(header_length) + " bytes");

  if (sizeof(IFFCommodoreAmiga) != header_length)
    return false;

  if (! readStorage(file, header_length, (IFF_UBYTE *) commodore_amiga))
    return false;

  convertWord(&commodore_amiga->ViewModes);

  if (CImageState::getDebug()) {
    CImage::infoMsg("  ViewModes = " + std::to_string(commodore_amiga->ViewModes));

    if (commodore_amiga->ViewModes & 0x8000)
      CImage::infoMsg("  HIRES");
    if (commodore_amiga->ViewModes & 0x0800)
      CImage::infoMsg("  HAM");
    if (commodore_amiga->ViewModes & 0x0080)
      CImage::infoMsg("  HALFBRIT");
    if (commodore_amiga->ViewModes & 0x0004)
      CImage::infoMsg("  LACE");
  }

  return true;
}

bool
CImageIFF::
readCRNG(CFile *file, IFFColorRange *color_range)
{
  if (CImageState::getDebug())
    CImage::infoMsg("CRNG");

  IFF_ULONG header_length;

  if (! readHeaderLength(file, &header_length))
    return false;

  if (CImageState::getDebug())
    CImage::infoMsg("  " + std::to_string(header_length) + " bytes");

  if (sizeof(IFFColorRange) != header_length)
    return false;

  if (! readStorage(file, header_length, (IFF_UBYTE *) color_range))
    return false;

  convertWord((IFF_UWORD *) &color_range->rate);
  convertWord((IFF_UWORD *) &color_range->active);

  if (CImageState::getDebug()) {
    CImage::infoMsg("  Rate   = " + std::to_string(color_range->rate));
    CImage::infoMsg("  Active = " + std::to_string(color_range->active));
    CImage::infoMsg("  Low    = " + std::to_string(color_range->low));
    CImage::infoMsg("  High   = " + std::to_string(color_range->high));
  }

  return true;
}

bool
CImageIFF::
readBody(CFile *file, IFF_UBYTE **screen_memory, IFF_ULONG *screen_memory_size)
{
  if (CImageState::getDebug())
    CImage::infoMsg("BODY");

  IFF_ULONG header_length;

  if (! readHeaderLength(file, &header_length))
    return false;

  if (CImageState::getDebug())
    CImage::infoMsg("  " + std::to_string(header_length) + " bytes");

  *screen_memory_size = header_length;
  *screen_memory      = new IFF_UBYTE [header_length + 1];

  if (*screen_memory == 0)
    return false;

  if (! readStorage(file, header_length, *screen_memory)) {
    delete [] *screen_memory;

    return false;
  }

  return true;
}

bool
CImageIFF::
readUnknown(CFile *file)
{
  IFF_ULONG header_length;

  if (! readHeaderLength(file, &header_length))
    return false;

  if (CImageState::getDebug())
    CImage::infoMsg("  " + std::to_string(header_length) + " bytes");

  if (! readBytes(file, header_length))
    return false;

  return true;
}

bool
CImageIFF::
readHeaderName(CFile *file, char *name)
{
  size_t size;
  uchar  buffer[4];

  if (! file->read(buffer, 4, &size))
    return false;

  strncpy(&name[0], (char *) &buffer[0], 4);
  name[4] = '\0';

  for (int i = 0; i < 4; ++i)
    if (! isalnum(name[i]) && ! isspace(name[i]))
      return false;

  return true;
}

bool
CImageIFF::
readHeaderLength(CFile *file, IFF_ULONG *length)
{
  size_t size;
  uchar  buffer[4];

  *length = 0;

  if (! file->read(buffer, 4, &size))
    return false;

  *length = ((buffer[0] << 24) & 0xFF000000U) |
            ((buffer[1] << 16) & 0x00FF0000U) |
            ((buffer[2] << 8 ) & 0x0000FF00U) |
            ((buffer[3]      ) & 0x000000FFU);

  return true;
}

bool
CImageIFF::
readBytes(CFile *file, int length)
{
  size_t size;

  if (length & 1)
    length += 1;

  IFF_UBYTE *buffer = new IFF_UBYTE [length];

  if (! file->read(buffer, length, &size)) {
    delete [] buffer;

    return false;
  }

  delete [] buffer;

  return true;
}

bool
CImageIFF::
readStorage(CFile *file, int length, IFF_UBYTE *buffer)
{
  size_t size;

  if (length & 1)
    length += 1;

  if (! file->read(buffer, length, &size))
    return false;

  return true;
}

IFF_UBYTE *
CImageIFF::
decompressScreenMemory(IFF_UBYTE *screen_memory, IFF_ULONG screen_memory_size, int width,
                       int height, int depth)
{
  IFF_UBYTE bytes_per_row = ((width + 15)/16) * 2;

  IFF_ULONG screen_memory_size1 = height*depth*bytes_per_row;

  IFF_UBYTE *screen_memory1 = new IFF_UBYTE [screen_memory_size1];

  IFF_ULONG screen_memory_position  = 0;
  IFF_ULONG screen_memory_position1 = 0;

  while (screen_memory_position < screen_memory_size) {
    IFF_UBYTE key = screen_memory[screen_memory_position++];

    if      (key < 128) {
      ++key;

      if (screen_memory_position  + key > screen_memory_size ||
          screen_memory_position1 + key > screen_memory_size1) {
        CImage::errorMsg("Decompress Failed 1");
        break;
      }

      for (int i = 0; i < key; ++i)
        screen_memory1[screen_memory_position1++] =
         screen_memory[screen_memory_position++];
    }
    else if (key > 128) {
      int count = 257 - key;

      if (screen_memory_position >= screen_memory_size) {
        CImage::errorMsg("Decompress Failed 2");
        break;
      }

      IFF_UBYTE data_byte = screen_memory[screen_memory_position++];

      if (screen_memory_position1 + count > screen_memory_size1) {
        CImage::errorMsg("Decompress Failed 3");
        CImage::errorMsg("Count = " + std::to_string(count) + ", Left = " +
                         std::to_string(screen_memory_size1 - screen_memory_position1));
        break;
      }

      for (int i = 0; i < count; ++i)
        screen_memory1[screen_memory_position1++] = data_byte;
    }
    else
      break;
  }

  return screen_memory1;
}

IFF_UBYTE *
CImageIFF::
convertScreenMemory8(IFF_UBYTE *screen_memory, int width, int height, int depth)
{
  IFF_UBYTE bytes_per_row = ((width + 15)/16) * 2;

  int screen_memory_size1 = 8*height*bytes_per_row;

  IFF_UBYTE *screen_memory1 = new IFF_UBYTE [screen_memory_size1];

  memset(screen_memory1, 0, screen_memory_size1*sizeof(IFF_UBYTE));

  int screen_memory_position = 0;

  int l = 0;

  for (int i = 0; i < height; ++i, l += width) {
    for (int j = 0; j < depth; ++j) {
      int m = l;

      for (int k = 0; k < bytes_per_row; ++k) {
        IFF_UBYTE byte = screen_memory[screen_memory_position++];

        IFF_UBYTE byte1 = ((byte >> 0) & 0x01) << j;
        IFF_UBYTE byte2 = ((byte >> 1) & 0x01) << j;
        IFF_UBYTE byte3 = ((byte >> 2) & 0x01) << j;
        IFF_UBYTE byte4 = ((byte >> 3) & 0x01) << j;
        IFF_UBYTE byte5 = ((byte >> 4) & 0x01) << j;
        IFF_UBYTE byte6 = ((byte >> 5) & 0x01) << j;
        IFF_UBYTE byte7 = ((byte >> 6) & 0x01) << j;
        IFF_UBYTE byte8 = ((byte >> 7) & 0x01) << j;

        screen_memory1[m++] |= byte8;
        screen_memory1[m++] |= byte7;
        screen_memory1[m++] |= byte6;
        screen_memory1[m++] |= byte5;
        screen_memory1[m++] |= byte4;
        screen_memory1[m++] |= byte3;
        screen_memory1[m++] |= byte2;
        screen_memory1[m++] |= byte1;
      }
    }
  }

  return screen_memory1;
}

IFF_UBYTE *
CImageIFF::
convertScreenMemory24(IFF_UBYTE *screen_memory, int width, int height)
{
  IFF_UBYTE bytes_per_row = ((width + 15)/16) * 2;

  int screen_memory_size1 = 24*height*bytes_per_row;

  IFF_UBYTE *screen_memory1 = new IFF_UBYTE [screen_memory_size1];

  memset(screen_memory1, 0, screen_memory_size1*sizeof(IFF_UBYTE));

  int screen_memory_position = 0;

  int l = 0;

  for (int i = 0; i < height; ++i, l += 3*width) {
    int j;

    for (j = 0; j < 8; ++j) {
      int m = l;

      for (int k = 0; k < bytes_per_row; ++k) {
        IFF_UBYTE byte = screen_memory[screen_memory_position++];

        IFF_UBYTE byte1 = ((byte >> 0) & 0x01) << j;
        IFF_UBYTE byte2 = ((byte >> 1) & 0x01) << j;
        IFF_UBYTE byte3 = ((byte >> 2) & 0x01) << j;
        IFF_UBYTE byte4 = ((byte >> 3) & 0x01) << j;
        IFF_UBYTE byte5 = ((byte >> 4) & 0x01) << j;
        IFF_UBYTE byte6 = ((byte >> 5) & 0x01) << j;
        IFF_UBYTE byte7 = ((byte >> 6) & 0x01) << j;
        IFF_UBYTE byte8 = ((byte >> 7) & 0x01) << j;

        screen_memory1[m] |= byte8; m += 3;
        screen_memory1[m] |= byte7; m += 3;
        screen_memory1[m] |= byte6; m += 3;
        screen_memory1[m] |= byte5; m += 3;
        screen_memory1[m] |= byte4; m += 3;
        screen_memory1[m] |= byte3; m += 3;
        screen_memory1[m] |= byte2; m += 3;
        screen_memory1[m] |= byte1; m += 3;
      }
    }

    for (j = 0; j < 8; ++j) {
      int m = l + 1;

      for (int k = 0; k < bytes_per_row; ++k) {
        IFF_UBYTE byte = screen_memory[screen_memory_position++];

        IFF_UBYTE byte1 = ((byte >> 0) & 0x01) << j;
        IFF_UBYTE byte2 = ((byte >> 1) & 0x01) << j;
        IFF_UBYTE byte3 = ((byte >> 2) & 0x01) << j;
        IFF_UBYTE byte4 = ((byte >> 3) & 0x01) << j;
        IFF_UBYTE byte5 = ((byte >> 4) & 0x01) << j;
        IFF_UBYTE byte6 = ((byte >> 5) & 0x01) << j;
        IFF_UBYTE byte7 = ((byte >> 6) & 0x01) << j;
        IFF_UBYTE byte8 = ((byte >> 7) & 0x01) << j;

        screen_memory1[m] |= byte8; m += 3;
        screen_memory1[m] |= byte7; m += 3;
        screen_memory1[m] |= byte6; m += 3;
        screen_memory1[m] |= byte5; m += 3;
        screen_memory1[m] |= byte4; m += 3;
        screen_memory1[m] |= byte3; m += 3;
        screen_memory1[m] |= byte2; m += 3;
        screen_memory1[m] |= byte1; m += 3;
      }
    }

    for (j = 0; j < 8; ++j) {
      int m = l + 2;

      for (int k = 0; k < bytes_per_row; ++k) {
        IFF_UBYTE byte = screen_memory[screen_memory_position++];

        IFF_UBYTE byte1 = ((byte >> 0) & 0x01) << j;
        IFF_UBYTE byte2 = ((byte >> 1) & 0x01) << j;
        IFF_UBYTE byte3 = ((byte >> 2) & 0x01) << j;
        IFF_UBYTE byte4 = ((byte >> 3) & 0x01) << j;
        IFF_UBYTE byte5 = ((byte >> 4) & 0x01) << j;
        IFF_UBYTE byte6 = ((byte >> 5) & 0x01) << j;
        IFF_UBYTE byte7 = ((byte >> 6) & 0x01) << j;
        IFF_UBYTE byte8 = ((byte >> 7) & 0x01) << j;

        screen_memory1[m] |= byte8; m += 3;
        screen_memory1[m] |= byte7; m += 3;
        screen_memory1[m] |= byte6; m += 3;
        screen_memory1[m] |= byte5; m += 3;
        screen_memory1[m] |= byte4; m += 3;
        screen_memory1[m] |= byte3; m += 3;
        screen_memory1[m] |= byte2; m += 3;
        screen_memory1[m] |= byte1; m += 3;
      }
    }
  }

  return screen_memory1;
}

IFF_UBYTE *
CImageIFF::
convertHAM(IFF_UBYTE *screen_memory, int width, int height,
           IFFColorRegister *cregs, CRGBA **colors, int *num_colors)
{
  int i;

  for (i = 0; i < 16; ++i) {
    cregs[i].r = (cregs[i].r >> 4) & 0x0F;
    cregs[i].g = (cregs[i].g >> 4) & 0x0F;
    cregs[i].b = (cregs[i].b >> 4) & 0x0F;
  }

  IFF_ULONG size = width*height;

  IFF_UBYTE *screen_memory1 = new IFF_UBYTE [size*3];

  IFF_UBYTE *p  = screen_memory;
  IFF_UBYTE *p1 = screen_memory1;

  for (i = 0; i < height; ++i) {
    IFF_UBYTE r = cregs[0].r;
    IFF_UBYTE g = cregs[0].g;
    IFF_UBYTE b = cregs[0].b;

    for (int j = 0; j < width; ++j) {
      IFF_ULONG c = *p & 0x0F;

      switch (*p & 0x30) {
        case 0x00:
          r = cregs[c].r;
          g = cregs[c].g;
          b = cregs[c].b;

          break;
        case 0x10:
          b = c;
          break;
        case 0x20:
          r = c;
          break;
        case 0x30:
          g = c;
          break;
      }

      *p1++ = r;
      *p1++ = g;
      *p1++ = b;

      ++p;
    }
  }

  int max_c = 16*16*16;

  IFF_UBYTE *c_flags  = new IFF_UBYTE [max_c];
  IFF_ULONG *c_flags1 = new IFF_ULONG [256  ];

  int tol     = 0;
  int tol_inc = 2;

  int num_c;

  while (true) {
    num_c = 0;

    memset(c_flags , '\0', max_c*sizeof(IFF_UBYTE));
    memset(c_flags1, '\0', 256  *sizeof(IFF_ULONG));

    p1 = screen_memory1;

    for (uint i = 0; i < size; ++i) {
      IFF_UBYTE r = *p1++;
      IFF_UBYTE g = *p1++;
      IFF_UBYTE b = *p1++;

      IFF_ULONG c = (r << 8) + (g << 4) + b;

      if (c_flags[c] == 1)
        continue;

      int r1 = r - tol;
      int r2 = r + tol;
      int g1 = g - tol;
      int g2 = g + tol;
      int b1 = b - tol;
      int b2 = b + tol;

      if (r1 <  0) r1 = 0;
      if (r2 > 15) r2 = 15;
      if (g1 <  0) g1 = 0;
      if (g2 > 15) g2 = 15;
      if (b1 <  0) b1 = 0;
      if (b2 > 15) b2 = 15;

      for (int rr = r1; rr <= r2; ++rr)
        for (int gg = g1; gg <= g2; ++gg)
          for (int bb = b1; bb <= b2; ++bb) {
            IFF_ULONG c1 = ((rr & 0x0F) << 8) +
                           ((gg & 0x0F) << 4) +
                           ((bb & 0x0F) << 0);

            c_flags[c1] = 1;
          }

      ++num_c;

      if (num_c > 256)
        continue;

      c_flags1[num_c - 1] = c;
    }

    if (num_c >= 256) {
      if (tol > 0 && tol_inc > 1) {
        if (CImageState::getDebug())
          CImage::infoMsg(std::to_string(num_c) + " Colors at Tolerance of " +
                          std::to_string(tol) + " (" + std::to_string(tol_inc) + ")");

        tol -= tol_inc;

        tol_inc /= 2;

        tol += tol_inc;

        continue;
      }

      break;
    }

    if (CImageState::getDebug())
      CImage::infoMsg(std::to_string(num_c) + " Colors at Tolerance of " +
                      std::to_string(tol) + " (" + std::to_string(tol_inc) + ")");

    tol += tol_inc;
  }

  if (CImageState::getDebug())
    CImage::infoMsg(std::to_string(num_c) + " Colors at Tolerance of " + std::to_string(tol));

  *num_colors = num_c;
  *colors     = new CRGBA [num_c];

  memset(c_flags, '\0', max_c*sizeof(IFF_UBYTE));

  for (i = 0; i < num_c; ++i) {
    IFF_UBYTE r = (c_flags1[i] >> 8) & 0x0F;
    IFF_UBYTE g = (c_flags1[i] >> 4) & 0x0F;
    IFF_UBYTE b = (c_flags1[i]     ) & 0x0F;

    (*colors)[i].setRGBA(r/15.0, g/15.0, b/15.0);

    if (CImageState::getDebug())
      CImage::infoMsg("Color : R " + std::to_string(r) +
                             " G " + std::to_string(g) +
                             " B " + std::to_string(b));

    int r1 = r - tol;
    int r2 = r + tol;
    int g1 = g - tol;
    int g2 = g + tol;
    int b1 = b - tol;
    int b2 = b + tol;

    if (r1 <  0) r1 = 0;
    if (r2 > 15) r2 = 15;
    if (g1 <  0) g1 = 0;
    if (g2 > 15) g2 = 15;
    if (b1 <  0) b1 = 0;
    if (b2 > 15) b2 = 15;

    for (int rr = r1; rr <= r2; ++rr)
      for (int gg = g1; gg <= g2; ++gg)
        for (int bb = b1; bb <= b2; ++bb) {
          IFF_ULONG c1 = ((rr & 0x0F) << 8) +
                         ((gg & 0x0F) << 4) +
                         ((bb & 0x0F) << 0);

          c_flags[c1] = i;
        }
  }

  IFF_UBYTE *screen_memory2 = new IFF_UBYTE [size];

  p  = screen_memory1;
  p1 = screen_memory2;

  for (uint i = 0; i < size; ++i) {
    IFF_UBYTE r = *p++;
    IFF_UBYTE g = *p++;
    IFF_UBYTE b = *p++;

    IFF_ULONG c1 = (r << 8) + (g << 4) + b;

    *p1++ = c_flags[c1];
  }

  delete [] c_flags;
  delete [] c_flags1;
  delete [] screen_memory1;

  return screen_memory2;
}

IFF_UBYTE *
CImageIFF::
convertHAM8(IFF_UBYTE *screen_memory, int width, int height,
            IFFColorRegister *cregs, CRGBA **colors, int *num_colors)
{
  if (CImageState::getDebug()) {
    for (int i = 0; i < 64; ++i)
      CImage::infoMsg("  "       + std::to_string(i) +
                      ") Red "   + std::to_string(cregs[i].r) +
                      ", Green " + std::to_string(cregs[i].g) +
                      ", Blue "  + std::to_string(cregs[i].b));
  }

  for (int i = 0; i < 64; ++i) {
    cregs[i].r = (cregs[i].r >> 2) & 0x3F;
    cregs[i].g = (cregs[i].g >> 2) & 0x3F;
    cregs[i].b = (cregs[i].b >> 2) & 0x3F;

    if (CImageState::getDebug()) {
      CImage::infoMsg("  "       + std::to_string(i) +
                      ") Red "   + std::to_string(cregs[i].r) +
                      ", Green " + std::to_string(cregs[i].g) +
                      ", Blue "  + std::to_string(cregs[i].b));
    }
  }

  IFF_ULONG size = width*height;

  IFF_UBYTE *screen_memory1 = new IFF_UBYTE [size*3];

  IFF_UBYTE *p  = screen_memory;
  IFF_UBYTE *p1 = screen_memory1;

  for (int i = 0; i < height; ++i) {
    IFF_UBYTE r = cregs[0].r;
    IFF_UBYTE g = cregs[0].g;
    IFF_UBYTE b = cregs[0].b;

    for (int j = 0; j < width; ++j) {
      IFF_ULONG c = *p & 0x3F;

      switch (*p & 0xc0) {
        case 0x00:
          r = cregs[c].r;
          g = cregs[c].g;
          b = cregs[c].b;

          break;
        case 0x40:
          b = c;
          break;
        case 0x80:
          r = c;
          break;
        case 0xc0:
          g = c;
          break;
      }

      *p1++ = r;
      *p1++ = g;
      *p1++ = b;

      ++p;
    }
  }

  int max_c = 64*64*64;

  IFF_UBYTE *c_flags  = new IFF_UBYTE [max_c];
  IFF_ULONG *c_flags1 = new IFF_ULONG [256  ];

  int tol     = 0;
  int tol_inc = 2;

  int num_c;

  while (true) {
    num_c = 0;

    memset(c_flags , '\0', max_c*sizeof(IFF_UBYTE));
    memset(c_flags1, '\0', 256  *sizeof(IFF_ULONG));

    p1 = screen_memory1;

    for (uint i = 0; i < size; ++i) {
      IFF_UBYTE r = *p1++;
      IFF_UBYTE g = *p1++;
      IFF_UBYTE b = *p1++;

      IFF_ULONG c = (r << 12) + (g << 6) + b;

      if (c_flags[c] == 1)
        continue;

      int r1 = r - tol;
      int r2 = r + tol;
      int g1 = g - tol;
      int g2 = g + tol;
      int b1 = b - tol;
      int b2 = b + tol;

      if (r1 <  0) r1 = 0;
      if (r2 > 63) r2 = 63;
      if (g1 <  0) g1 = 0;
      if (g2 > 63) g2 = 63;
      if (b1 <  0) b1 = 0;
      if (b2 > 63) b2 = 63;

      for (int rr = r1; rr <= r2; ++rr)
        for (int gg = g1; gg <= g2; ++gg)
          for (int bb = b1; bb <= b2; ++bb) {
            IFF_ULONG c1 = ((rr & 0x3F) << 12) +
                           ((gg & 0x3F) <<  6) +
                           ((bb & 0x3F) <<  0);

            c_flags[c1] = 1;
          }

      ++num_c;

      if (num_c > 256)
        continue;

      c_flags1[num_c - 1] = c;
    }

    if (num_c <= 256) {
      if (tol > 0 && tol_inc > 1) {
        if (CImageState::getDebug())
          CImage::infoMsg(std::to_string(num_c) + " Colors at Tolerance of " +
                          std::to_string(tol) + " (" + std::to_string(tol_inc) + ")");

        tol -= tol_inc;

        tol_inc /= 2;

        tol += tol_inc;

        continue;
      }

      break;
    }

    if (CImageState::getDebug())
      CImage::infoMsg(std::to_string(num_c) + " Colors at Tolerance of " +
                      std::to_string(tol) + " (" + std::to_string(tol_inc) + ")");

    tol += tol_inc;
  }

  if (CImageState::getDebug())
    CImage::infoMsg(std::to_string(num_c) + " Colors at Tolerance of " +
                    std::to_string(tol));

  *num_colors = num_c;
  *colors     = new CRGBA [num_c];

  memset(c_flags, '\0', max_c*sizeof(IFF_UBYTE));

  for (int i = 0; i < num_c; ++i) {
    IFF_UBYTE r = (c_flags1[i] >> 12) & 0x3F;
    IFF_UBYTE g = (c_flags1[i] >>  6) & 0x3F;
    IFF_UBYTE b = (c_flags1[i]      ) & 0x3F;

    (*colors)[i].setRGBA(r/63.0, g/63.0, b/63.0);

    if (CImageState::getDebug())
      CImage::infoMsg("Color : R " + std::to_string(r) +
                             " G " + std::to_string(g) +
                             " B " + std::to_string(b));

    int r1 = r - tol;
    int r2 = r + tol;
    int g1 = g - tol;
    int g2 = g + tol;
    int b1 = b - tol;
    int b2 = b + tol;

    if (r1 <  0) r1 = 0;
    if (r2 > 63) r2 = 63;
    if (g1 <  0) g1 = 0;
    if (g2 > 63) g2 = 63;
    if (b1 <  0) b1 = 0;
    if (b2 > 63) b2 = 63;

    for (int rr = r1; rr <= r2; ++rr)
      for (int gg = g1; gg <= g2; ++gg)
        for (int bb = b1; bb <= b2; ++bb) {
          IFF_ULONG c1 = ((rr & 0x3F) << 12) +
                         ((gg & 0x3F) <<  6) +
                         ((bb & 0x3F) <<  0);

          c_flags[c1] = i;
        }
  }

  IFF_UBYTE *screen_memory2 = new IFF_UBYTE [size];

  p  = screen_memory1;
  p1 = screen_memory2;

  for (uint i = 0; i < size; ++i) {
    IFF_UBYTE r = *p++;
    IFF_UBYTE g = *p++;
    IFF_UBYTE b = *p++;

    IFF_ULONG c1 = (r << 12) + (g << 6) + b;

    *p1++ = c_flags[c1];
  }

  delete [] c_flags;
  delete [] c_flags1;
  delete [] screen_memory1;

  return screen_memory2;
}

IFF_UBYTE *
CImageIFF::
convert24Bit(IFF_UBYTE *screen_memory, int width, int height, CRGBA **colors, int *num_colors)
{
  int max_c = 256*256*256;

  IFF_UBYTE *c_flags  = new IFF_UBYTE [max_c];
  IFF_ULONG *c_flags1 = new IFF_ULONG [256  ];

  int tol     = 0;
  int tol_inc = 2;

  int num_c;

  while (true) {
    num_c = 0;

    memset(c_flags , '\0', max_c*sizeof(IFF_UBYTE));
    memset(c_flags1, '\0', 256  *sizeof(IFF_ULONG));

    IFF_UBYTE *p = screen_memory;

    for (int i = 0; i < height; ++i) {
      for (int j = 0; j < width; ++j) {
        IFF_UBYTE r = *p++;
        IFF_UBYTE g = *p++;
        IFF_UBYTE b = *p++;

        IFF_ULONG c = (r << 16) + (g << 8) + b;

        if (c_flags[c] == 1)
          continue;

        int r1 = std::max(r - tol, 0);
        int r2 = std::min(r + tol, 255);
        int g1 = std::max(g - tol, 0);
        int g2 = std::min(g + tol, 255);
        int b1 = std::max(b - tol, 0);
        int b2 = std::min(b + tol, 255);

        for (int rr = r1; rr <= r2; ++rr)
          for (int gg = g1; gg <= g2; ++gg)
            for (int bb = b1; bb <= b2; ++bb) {
              IFF_ULONG c1 = ((rr & 0xFF) << 16) +
                             ((gg & 0xFF) <<  8) +
                             ((bb & 0xFF) <<  0);

              c_flags[c1] = 1;
            }

        ++num_c;

        if (num_c > 256)
          continue;

        c_flags1[num_c - 1] = c;
      }
    }

    if (num_c <= 256) {
      if (tol > 0 && tol_inc > 1) {
        if (CImageState::getDebug())
          CImage::infoMsg(std::to_string(num_c) + " Colors at Tolerance of " +
                          std::to_string(tol) + " (" + std::to_string(tol_inc) + ")");

        tol -= tol_inc;

        tol_inc /= 2;

        tol += tol_inc;

        continue;
      }

      break;
    }

    if (CImageState::getDebug())
      CImage::infoMsg(std::to_string(num_c) + " Colors at Tolerance of " +
                      std::to_string(tol) + " (" + std::to_string(tol_inc) + ")");

    tol += tol_inc;
  }

  if (CImageState::getDebug())
    CImage::infoMsg(std::to_string(num_c) + " Colors at Tolerance of " +
                    std::to_string(tol));

  *num_colors = num_c;
  *colors     = new CRGBA [num_c];

  memset(c_flags, '\0', max_c*sizeof(IFF_UBYTE));

  int i;

  for (i = 0; i < num_c; ++i) {
    IFF_UBYTE r = (c_flags1[i] >> 16) & 0xFF;
    IFF_UBYTE g = (c_flags1[i] >>  8) & 0xFF;
    IFF_UBYTE b = (c_flags1[i]      ) & 0xFF;

    (*colors)[i].setRGBAI(r, g, b);

    if (CImageState::getDebug())
      CImage::infoMsg("Color : R " + std::to_string(r) +
                             " G " + std::to_string(g) +
                             " B " + std::to_string(b));

    int r1 = std::min(r - tol, 0);
    int r2 = std::max(r + tol, 255);
    int g1 = std::min(g - tol, 0);
    int g2 = std::max(g + tol, 255);
    int b1 = std::min(b - tol, 0);
    int b2 = std::max(b + tol, 255);

    for (int rr = r1; rr <= r2; ++rr)
      for (int gg = g1; gg <= g2; ++gg)
        for (int bb = b1; bb <= b2; ++bb) {
          IFF_ULONG c1 = ((rr & 0xFF) << 16) +
                         ((gg & 0xFF) <<  8) +
                         ((bb & 0xFF) <<  0);

          c_flags[c1] = i;
        }
  }

  IFF_UBYTE *screen_memory1 = new IFF_UBYTE [width*height];

  IFF_UBYTE *p  = screen_memory;
  IFF_UBYTE *p1 = screen_memory1;

  for (i = 0; i < height; ++i) {
    for (int j = 0; j < width; ++j) {
      IFF_UBYTE r = *p++;
      IFF_UBYTE g = *p++;
      IFF_UBYTE b = *p++;

      IFF_ULONG c1 = (r << 16) + (g << 8) + b;

      *p1++ = c_flags[c1];
    }
  }

  delete [] c_flags;
  delete [] c_flags1;

  return screen_memory1;
}

CRGBA *
CImageIFF::
convertColors(IFFColorRegister *cregs, int num_colors)
{
  CRGBA *colors = new CRGBA [num_colors];

  for (int i = 0; i < num_colors; ++i) {
    IFF_UBYTE r = (cregs[i].r >> 4) & 0x0F;
    IFF_UBYTE g = (cregs[i].g >> 4) & 0x0F;
    IFF_UBYTE b = (cregs[i].b >> 4) & 0x0F;

    colors[i].setRGBA(r/15.0, g/15.0, b/15.0);

    if (CImageState::getDebug())
      CImage::infoMsg(std::to_string(i) +
                      ") Red "   + std::to_string(r) +
                      ", Green " + std::to_string(g) +
                      ", Blue "  + std::to_string(b));
  }

  return colors;
}

void
CImageIFF::
convertWord(IFF_UWORD *integer)
{
  IFF_UBYTE *p = (IFF_UBYTE *) integer;

  IFF_UWORD integer1 = (p[0] << 8) | p[1];

  *integer = integer1;
}

bool
CImageIFF::
write(CFile *, CImagePtr)
{
  return false;
}
