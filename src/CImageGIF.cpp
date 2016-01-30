#include <CImageLib.h>
#include <CImageGIF.h>
#include <CThrow.h>
#include <CStrUtil.h>
#include <cstring>

#define CIMAGE_GIF_DICT_SIZE 5021

#define GIF87a 0
#define GIF89a 0

#define IMAGE_ID   0x2C
#define CONTROL_ID 0x21
#define TRAILER_ID 0x3B

#define CONTROL_APPEXT_ID  0xFF
#define CONTROL_COMMENT_ID 0xFE
#define CONTROL_LABEL_ID   0xF9

#define UNUSED_CODE ((uint) -1)

struct CImageGIFHeader;
struct CImageGIFColorTable;

struct CImageGIFData {
  CImageGIFHeader     *header;
  CImageGIFColorTable *global_colors;
  int                  num_global_colors;
  CImageGIFColorTable *local_colors;
  int                  num_local_colors;
};

struct CImageGIFHeader {
  char   signature[3];
  char   version[3];
  ushort width;
  ushort height;
  uchar  flags;
  uchar  background;
  uchar  aspect;

  uchar  color_bits;
  uchar  colors_sorted;
  uchar  max_color_bits;
  uchar  global_color_table;
};

struct CImageGIFColorTable {
  uchar r;
  uchar g;
  uchar b;
};

struct CImageGIFImageHeader {
  ushort left;
  ushort top;
  ushort width;
  ushort height;
  uchar  flags;

  uchar  local_color_table;
  uchar  interlaced;
  uchar  colors_sorted;
  uchar  color_bits;
};

struct CImageGIFCompressData {
  uchar         bit_mask;
  uchar         code_size;
  uchar         init_code_size;
  uint          code_mask;
  uint          clear_code;
  uint          eof_code;
  uint          free_code;
  uint          max_code;
  uchar         max_code_size;
  uint          current_bit;
  uchar         current_byte;

  CImageGIFDict dictionary[CIMAGE_GIF_DICT_SIZE];
  uchar         buffer[256];

  int           num_code_bytes;
  uchar         code_bytes[256];
  uint          color_table_bits;

  uchar         out_bytes[1024];
};

static CImageGIFCompressData compress_data;

bool
CImageGIF::
read(CFile *file, CImagePtr &image)
{
  CImageAnim *image_anim = createAnim(file);

  if (image_anim->begin() != image_anim->end()) {
    CImageFrame *frame = *(image_anim->begin());

    CImagePtr ptr = frame->getImage();

    image->replace(ptr);
  }

  delete image_anim;

  return true;
}

bool
CImageGIF::
readHeader(CFile *file, CImagePtr &image)
{
  file->rewind();

  try {
    CImageGIFHeader header;

    readHeader(file, image, &header);

    //------

    image->setType(CFILE_TYPE_IMAGE_GIF);

    image->setSize(header.width, header.height);
  }
  catch (...) {
    CImage::errorMsg("Failed to read GIF file");
    return false;
  }

  return true;
}

CImageAnim *
CImageGIF::
createAnim(CFile *file)
{
  CImageAnim *image_anim = new CImageAnim();

  //------

  file->rewind();

  //------

  CImageGIFData *gif_data = new CImageGIFData;

  gif_data->header = new CImageGIFHeader;

  gif_data->global_colors     = 0;
  gif_data->num_global_colors = 0;
  gif_data->local_colors      = 0;
  gif_data->num_local_colors  = 0;

  //------

  try {
    memset(&compress_data, 0, sizeof(CImageGIFCompressData));

    CImagePtr image;

    readHeader(file, image, gif_data->header);

    //------

    readGlobalColors(file, gif_data);

    //------

    readAnimData(file, image_anim, gif_data);
  }
  catch (...) {
    CImage::errorMsg("Failed to read GIF file");
    return 0;
  }

  //------

  if (gif_data != 0) {
    delete gif_data->header;

    delete [] gif_data->global_colors;
    delete [] gif_data->local_colors;
  }

  delete gif_data;

  //------

  return image_anim;
}

bool
CImageGIF::
readHeader(CFile *file, CImagePtr &, CImageGIFHeader *header)
{
  uchar byte1;
  uchar byte2;

  file->read((uchar *) header->signature, 3);
  file->read((uchar *) header->version  , 3);

  file->read(&byte1, 1);
  file->read(&byte2, 1);

  header->width = (byte2 << 8) | byte1;

  file->read(&byte1, 1);
  file->read(&byte2, 1);

  header->height = (byte2 << 8) | byte1;

  file->read(&header->flags     , 1);
  file->read(&header->background, 1);
  file->read(&header->aspect    , 1);

  header->color_bits         = (header->flags     ) & 0x07;
  header->colors_sorted      = (header->flags >> 3) & 0x01;
  header->max_color_bits     = (header->flags >> 4) & 0x07;
  header->global_color_table = (header->flags >> 7) & 0x01;

  if (CImageState::getDebug()) {
    CImage::infoMsg("Signature     " + std::to_string(header->signature[0]) +
                                       std::to_string(header->signature[1]) +
                                       std::to_string(header->signature[2]));
    CImage::infoMsg("Version       " + std::to_string(header->version[0]) +
                                       std::to_string(header->version[1]) +
                                       std::to_string(header->version[2]));
    CImage::infoMsg("Width         " + std::to_string(header->width));
    CImage::infoMsg("Height        " + std::to_string(header->height));
    CImage::infoMsg("Num Colors    " + std::to_string((1 << (header->color_bits + 1))));
    CImage::infoMsg("Colors Sorted " + std::to_string((int) header->colors_sorted));
    CImage::infoMsg("Max Colors    " + std::to_string((1 << (header->max_color_bits + 1))));
    CImage::infoMsg("Global Colors " + std::to_string((int) header->global_color_table));
    CImage::infoMsg("Background    " + std::to_string((int) header->background));
    CImage::infoMsg("Aspect        " + std::to_string((int) header->aspect));
  }

  if (strncmp(header->signature, "GIF", 3) != 0) {
    CImage::errorMsg("Not a GIF File");
    return false;
  }

#if 0
  int type;

  if      (strncmp(header->version, "87a", 3) == 0)
    type = GIF87a;
  else if (strncmp(header->version, "89a", 3) == 0)
    type = GIF89a;
  else {
    CImage::errorMsg("Invalid GIF Version");
    return false;
  }
#endif

  return true;
}

void
CImageGIF::
readGlobalColors(CFile *file, CImageGIFData *gif_data)
{
  if (gif_data->header->global_color_table) {
    gif_data->num_global_colors = 1 << (gif_data->header->color_bits + 1);

    gif_data->global_colors = new CImageGIFColorTable [gif_data->num_global_colors];

    for (int i = 0; i < gif_data->num_global_colors; ++i)
      file->read((uchar *) &gif_data->global_colors[i], 3);
  }

  compress_data.bit_mask = gif_data->num_global_colors - 1;

  if (CImageState::getDebug()) {
    for (int i = 0; i < gif_data->num_global_colors; ++i)
      CImage::infoMsg("Color: " + std::to_string(i) + " " +
                      std::to_string((int) gif_data->global_colors[i].r) + " " +
                      std::to_string((int) gif_data->global_colors[i].g) + " " +
                      std::to_string((int) gif_data->global_colors[i].b));
  }
}

bool
CImageGIF::
readAnimData(CFile *file, CImageAnim *image_anim, CImageGIFData *gif_data)
{
  int  inum              = 0;
  int  delay             = 0;
  bool transparent       = false;
  uint transparent_color = 0;
  int  dispose           = 0;
  int  user_input        = 0;

  uint file_size = file->getSize();

  while (true) {
    uchar id;

    try {
      if (! file->read(&id, 1))
        break;
    }
    catch (...) {
      break;
    }

    if      (id == IMAGE_ID) {
      ++inum;

      if (CImageState::getDebug())
        CImage::infoMsg("Image Id");

      CImageGIFImageHeader *image_header = new CImageGIFImageHeader;

      try {
        uchar byte1;
        uchar byte2;

        file->read(&byte1, 1);
        file->read(&byte2, 1);

        image_header->left = (byte2 << 8) | byte1;

        file->read(&byte1, 1);
        file->read(&byte2, 1);

        image_header->top = (byte2 << 8) | byte1;

        file->read(&byte1, 1);
        file->read(&byte2, 1);

        image_header->width = (byte2 << 8) | byte1;

        file->read(&byte1, 1);
        file->read(&byte2, 1);

        image_header->height = (byte2 << 8) | byte1;

        file->read(&image_header->flags, 1);

        image_header->local_color_table = (image_header->flags >> 7) & 0x01;
        image_header->interlaced        = (image_header->flags >> 6) & 0x01;
        image_header->colors_sorted     = (image_header->flags >> 5) & 0x01;
        image_header->color_bits        = (image_header->flags     ) & 0x07;

        if (CImageState::getDebug()) {
          CImage::infoMsg("Left          " + std::to_string(image_header->left));
          CImage::infoMsg("Top           " + std::to_string(image_header->top));
          CImage::infoMsg("Width         " + std::to_string(image_header->width));
          CImage::infoMsg("Height        " + std::to_string(image_header->height));
          CImage::infoMsg("Local Colors  " + std::to_string(image_header->local_color_table));
          CImage::infoMsg("Interlaced    " + std::to_string(image_header->interlaced));
          CImage::infoMsg("Colors Sorted " + std::to_string(image_header->colors_sorted));
          CImage::infoMsg("Num Colors    " + std::to_string(1 << (image_header->color_bits + 1)));
        }

        if (image_header->local_color_table &&
            image_header->color_bits > 0) {
          gif_data->num_local_colors = 1 << (image_header->color_bits + 1);

          gif_data->local_colors = new CImageGIFColorTable [gif_data->num_local_colors];

          for (int i = 0; i < gif_data->num_local_colors; ++i)
            file->read((uchar *) &gif_data->local_colors[i], 3);

          if (CImageState::getDebug()) {
            for (int i = 0; i < gif_data->num_local_colors; ++i)
              CImage::infoMsg(std::to_string(gif_data->local_colors[i].r) + " " +
                              std::to_string(gif_data->local_colors[i].g) + " " +
                              std::to_string(gif_data->local_colors[i].b));
          }
        }

        file->read(&compress_data.code_size, 1);

        compress_data.clear_code = 1 << compress_data.code_size;
        compress_data.eof_code   = compress_data.clear_code + 1;
        compress_data.free_code  = compress_data.clear_code + 2;

        ++compress_data.code_size;

        compress_data.init_code_size = compress_data.code_size;

        compress_data.max_code  = 1 << compress_data.code_size;
        compress_data.code_mask = compress_data.max_code - 1;

        uint num_image_bytes = image_header->width*image_header->height;

        uchar *data = new uchar [file_size];

        uchar size;

        file->read(&size, 1);

        uint num_bytes_read = 0;

        while (size > 0) {
          while (size--) {
            file->read(&data[num_bytes_read], 1);

            ++num_bytes_read;
          }

          file->read(&size, 1);
        }

        if (num_bytes_read < file_size)
          memset(&data[num_bytes_read], 0, file_size - num_bytes_read);

        //------

        uchar *raw_data = new uchar [num_image_bytes];

        decompressData(data, num_bytes_read, raw_data, num_image_bytes);

        delete [] data;

        if (image_header->interlaced)
          deInterlace(raw_data, image_header);

        //------

        CImageNameSrc src("CImageGIF:" + file->getPath() + "/" + CStrUtil::toString(inum));

        CImagePtr image = CImageMgrInst->createImage(src);

        image->setType(CFILE_TYPE_IMAGE_GIF);

        image->setDataSize(image_header->width, image_header->height);

        int bottom = gif_data->header->height - image_header->height - image_header->top;
        int right  = gif_data->header->width  - image_header->width  - image_header->left;

        image->setBorder(image_header->left, bottom, right, image_header->top);

        if (gif_data->num_local_colors > 0) {
          for (int i = 0; i < gif_data->num_local_colors; ++i)
            image->addColorI(gif_data->local_colors[i].r,
                             gif_data->local_colors[i].g,
                             gif_data->local_colors[i].b);
        }
        else {
          for (int i = 0; i < gif_data->num_global_colors; ++i)
            image->addColorI(gif_data->global_colors[i].r,
                             gif_data->global_colors[i].g,
                             gif_data->global_colors[i].b);

          image->setBackground(image->getColor(gif_data->header->background));
        }

        //------

        if (transparent)
          image->setTransparentColor(transparent_color);

        //------

        image->setColorIndexData(raw_data);

        delete [] raw_data;

        //------

        CImageFrame *frame = new CImageFrame(image);

        frame->setDelay(delay);
        frame->setDispose(dispose);
        frame->setUserInput(user_input);

        image_anim->add(frame);

        //------

        delay             = 0;
        transparent       = false;
        transparent_color = 0;
        dispose           = 0;
        user_input        = 0;

        //------

        delete image_header;

        image_header = 0;
      }
      catch (...) {
        delete image_header;

        CImage::errorMsg("Failed to read GIF file");

        return false;
      }
    }
    else if (id == CONTROL_ID) {
      if (CImageState::getDebug())
        CImage::infoMsg("Control Id");

      try {
        uchar id1;

        file->read(&id1, 1);

        if (CImageState::getDebug())
          CImage::infoMsg("Id = " + CStrUtil::toHexString(id1));

        uchar size;

        file->read(&size, 1);

        if (CImageState::getDebug())
          CImage::infoMsg("Size = " + CStrUtil::toHexString(size));

        //if (id1 == CONTROL_APPEXT_ID) size = 11;

        if (size == 0)
          continue;

        file->read(compress_data.buffer, size);

        if (id1 == CONTROL_LABEL_ID) {
          if (CImageState::getDebug())
            CImage::infoMsg("Graphics Control Extension");

          delay             = (compress_data.buffer[2] << 8) |
                              compress_data.buffer[1];
          transparent       = compress_data.buffer[0] & 0x01;
          transparent_color = compress_data.buffer[3];
          user_input        = ((compress_data.buffer[0] & 0x02) >> 1);
          dispose           = ((compress_data.buffer[0] & 0x1C) >> 2);

          if (CImageState::getDebug()) {
            CImage::infoMsg("Delay       " + std::to_string(delay));
            CImage::infoMsg("Transparent " + std::to_string(transparent) + " " +
                                             std::to_string(transparent_color));
            CImage::infoMsg("User Input  " + std::to_string(user_input));
            CImage::infoMsg("Dispose     " + std::to_string(dispose));
          }
        }
        else if (id1 == CONTROL_APPEXT_ID) {
          if (CImageState::getDebug())
            CImage::infoMsg("Application Extension");
        }
        else if (id1 == CONTROL_COMMENT_ID) {
          if (CImageState::getDebug())
            CImage::infoMsg("Comment");

          uchar len = 0;

          file->read(&len, 1);

          while (len > 0) {
            uchar c;

            for (uint i = 0; i < len; ++i)
              file->read(&c, 1);

            file->read(&len, 1);
          }
        }
        else {
          CImage::errorMsg("Unknown control block " + std::to_string((int) id1));
        }

        // skip to block terminator
        while (true) {
          if (! file->read(&size, 1))
            break;

          if (size == 0)
            break;

          file->read(compress_data.buffer, size);
        }

        if (CImageState::getDebug())
          CImage::infoMsg("@ " + std::to_string(file->getPos()));
      }
      catch (...) {
        CImage::errorMsg("Failed to read GIF file");
        return false;
      }
    }
    else if (id == TRAILER_ID) {
      if (CImageState::getDebug())
        CImage::errorMsg("Trailer Id");

      break;
    }
    else if (id == 0) {
      uchar pad;

      file->read(&pad, 1);
    }
    else {
      CImage::errorMsg("Invalid Id " + std::to_string(int(id)) +
                       " @ " + std::to_string(file->getPos()));

      uchar c;

      file->read(&c, 1);

      while (c) {
        if (! file->read(&c, 1))
          break;
      }
    }
  }

  return true;
}

bool
CImageGIF::
decompressData(uchar *in_data, int in_data_size,
               uchar *out_data, int out_data_size)
{
  int num_out_data = 0;

  uint bit_offset = 0;

  int byte_no = bit_offset/8;

  uint  last_code = 0;
  uchar last_byte = 0;

  while (byte_no < in_data_size) {
    uint code = readCode(&bit_offset, in_data);

    if (code == compress_data.eof_code)
      break;

    if (code == compress_data.clear_code) {
      compress_data.code_size = compress_data.init_code_size;

      compress_data.max_code  = 1 << compress_data.code_size;
      compress_data.code_mask = compress_data.max_code - 1;

      compress_data.free_code = compress_data.clear_code + 2;

      code = readCode(&bit_offset, in_data);

      last_code = code;

      last_byte = code & compress_data.bit_mask;

      if (num_out_data + 1 > out_data_size) {
        if (CImageState::getDebug())
          CImage::infoMsg("Output Data Overflow !!!");

        break;
      }

      out_data[num_out_data++] = last_byte;
    }
    else {
      int num_out_bytes = 0;

      uint code1 = code;

      if (code >= compress_data.free_code) {
        code1 = last_code;

        compress_data.out_bytes[num_out_bytes++] = last_byte;
      }

      while (code1 > compress_data.bit_mask) {
        compress_data.out_bytes[num_out_bytes++] = compress_data.dictionary[code1].character;

        code1 = compress_data.dictionary[code1].parent_code;
      }

      last_byte = code1 & compress_data.bit_mask;

      compress_data.out_bytes[num_out_bytes++] = last_byte;

      if (num_out_data + num_out_bytes > out_data_size) {
        if (CImageState::getDebug())
          CImage::infoMsg("Output Data Overflow !!!");

        break;
      }

      for (int i = num_out_bytes - 1; i >= 0; --i)
        out_data[num_out_data++] = compress_data.out_bytes[i] & compress_data.bit_mask;

      compress_data.dictionary[compress_data.free_code].parent_code = last_code;
      compress_data.dictionary[compress_data.free_code].character   = last_byte;

      last_code = code;

      ++compress_data.free_code;

      if (compress_data.free_code >= compress_data.max_code && compress_data.code_size < 12) {
        ++compress_data.code_size;

        compress_data.max_code  = 1 << compress_data.code_size;
        compress_data.code_mask = compress_data.max_code - 1;
      }
    }

    byte_no = bit_offset/8;
  }

  return true;
}

uint
CImageGIF::
readCode(uint *bit_offset, uchar *data)
{
  int byte_no = (*bit_offset)/8;

  int code = data[byte_no] & 0xFF;

  code |= (data[byte_no + 1] << 8) & 0xFF00;

  if (compress_data.code_size > 8)
    code |= (data[byte_no + 2] << 16) & 0xFF0000;

  code >>= (*bit_offset) % 8;

  code &= compress_data.code_mask;

  *bit_offset += compress_data.code_size;

  return code;
}

void
CImageGIF::
deInterlace(uchar *image, CImageGIFImageHeader *image_header)
{
  int image_size = image_header->width*image_header->height;

  uchar *image1 = new uchar [image_size];

  int i = 0;

  int j;

  for (j = 0; j < image_header->height; j += 8, ++i) {
    int i1 = i*image_header->width;
    int i2 = j*image_header->width;

    memcpy(&image1[i2], &image[i1], image_header->width);
  }

  for (j = 4; j < image_header->height; j += 8, ++i) {
    int i1 = i*image_header->width;
    int i2 = j*image_header->width;

    memcpy(&image1[i2], &image[i1], image_header->width);
  }

  for (j = 2; j < image_header->height; j += 4, ++i) {
    int i1 = i*image_header->width;
    int i2 = j*image_header->width;

    memcpy(&image1[i2], &image[i1], image_header->width);
  }

  for (j = 1; j < image_header->height; j += 2, ++i) {
    int i1 = i*image_header->width;
    int i2 = j*image_header->width;

    memcpy(&image1[i2], &image[i1], image_header->width);
  }

  memcpy(image, image1, image_size*sizeof(uchar));

  delete [] image1;
}

bool
CImageGIF::
write(CFile *file, CImagePtr image)
{
  memset(&compress_data, 0, sizeof(CImageGIFCompressData));

  CImagePtr image1 = image;

  if (! image->hasColormap()) {
    CImage::warnMsg("GIF Image Depth greater than 8 not supported - "
                    "Converting image to 256 colors");

    image1 = image1->dup();

    image1->convertToColorIndex();
  }

  writeChars(file, "GIF", 3);

  if (image1->isTransparent())
    writeChars(file, "89a", 3);
  else
    writeChars(file, "87a", 3);

  writeHeader(file, image1);

  writeGraphicsBlock(file, image1, 0);

  writeByte(file, IMAGE_ID);

  int left, right, top, bottom;

  image1->getBorder(&left, &bottom, &right, &top);

  writeShort(file, left);
  writeShort(file, top );

  writeShort(file, image1->getWidth ());
  writeShort(file, image1->getHeight());

  uint color_table   = 0;
  uint color_bits    = 0;
  uint colors_sorted = 0;
  uint interlace     = 0;

  uint packed = 0;

  packed |= color_bits > 0 ? (color_bits - 1) << 5 : 0;
  packed |= colors_sorted << 2;
  packed |= interlace     << 1;
  packed |= color_table   << 0;

  writeByte(file, packed);

  compress_data.code_size = compress_data.color_table_bits;

  if (compress_data.code_size < 2)
    compress_data.code_size = 2;

  writeByte(file, compress_data.code_size);

  writeData(file, image1);
  writeByte(file, 0);

  writeByte(file, TRAILER_ID);

  return true;
}

bool
CImageGIF::
writeAnim(CFile *file, const std::vector<CImagePtr> &images, int delay)
{
  if (images.empty())
    return false;

  writeChars(file, "GIF", 3);
  writeChars(file, "89a", 3);

  uint num_images = images.size();

  std::vector<CImagePtr> images1;

  for (uint i = 0; i < num_images; ++i) {
    CImagePtr image1 = images[i];

    if (! image1->hasColormap()) {
      CImage::warnMsg("GIF Image Depth greater than 8 not supported - "
                      "Converting image to 256 colors");

      image1 = image1->dup();

      image1->convertToColorIndex();
    }

    images1.push_back(image1);
  }

  CImagePtr image1 = images1[0];

  writeHeader(file, image1);

  int color_table_bits = compress_data.color_table_bits;

  for (uint i = 0; i < num_images; ++i) {
    memset(&compress_data, 0, sizeof(CImageGIFCompressData));

    compress_data.color_table_bits = color_table_bits;

    CImagePtr image1 = images1[i];

    writeGraphicsBlock(file, image1, delay);

    writeByte(file, IMAGE_ID);

    int left, right, top, bottom;

    image1->getBorder(&left, &bottom, &right, &top);

    writeShort(file, left);
    writeShort(file, top );

    writeShort(file, image1->getWidth ());
    writeShort(file, image1->getHeight());

    uint color_table   = 0;
    uint color_bits    = 0;
    uint colors_sorted = 0;
    uint interlace     = 0;

    uint packed = 0;

    packed |= color_bits > 0 ? (color_bits - 1) << 5 : 0;
    packed |= colors_sorted << 2;
    packed |= interlace     << 1;
    packed |= color_table   << 0;

    writeByte(file, packed);

    compress_data.code_size = compress_data.color_table_bits;

    if (compress_data.code_size < 2)
      compress_data.code_size = 2;

    writeByte(file, compress_data.code_size);

    writeData(file, image1);
    writeByte(file, 0);
  }

  writeByte(file, TRAILER_ID);

  return true;
}

void
CImageGIF::
writeHeader(CFile *file, CImagePtr image)
{
  writeShort(file, image->getWidth() );
  writeShort(file, image->getHeight());

  uint color_table   = 1;
  uint color_bits    = 8;
  uint colors_sorted = 0;

  int i;

  for (i = 1; i < 8; ++i)
    if ((1<<i) >= image->getNumColors())
      break;

  compress_data.color_table_bits = i;

  uint color_map_size = 1 << compress_data.color_table_bits;

  uint packed = 0;

  packed |=  color_table                         << 7;
  packed |= (color_bits - 1)                     << 4;
  packed |=  colors_sorted                       << 3;
  packed |= (compress_data.color_table_bits - 1) << 0;

  writeByte(file, packed);

  uint background = 0;

  writeByte(file, background);

  uint aspect_ratio = 0;

  writeByte(file, aspect_ratio);

  uint r, g, b, a;

  for (i = 0; i < (int) color_map_size; ++i) {
    if (i < image->getNumColors()) {
      image->getColorRGBAI(i, &r, &g, &b, &a);

      writeByte(file, r);
      writeByte(file, g);
      writeByte(file, b);
    }
    else {
      writeByte(file, 0);
      writeByte(file, 0);
      writeByte(file, 0);
    }
  }
}

void
CImageGIF::
writeGraphicsBlock(CFile *file, CImagePtr image, int delay)
{
  int transparent_index = -1;

  if (image->isTransparent())
    transparent_index = image->getTransparentColor();

  writeByte (file, CONTROL_ID      );
  writeByte (file, CONTROL_LABEL_ID); // graphics control extension
  writeByte (file, 0x04            ); // length

  if (transparent_index >= 0) {
    writeByte (file, 0x01             ); // transparent flag
    writeShort(file, delay            ); // delay
    writeByte (file, transparent_index); // transparent index
  }
  else {
    writeByte (file, 0                ); // transparent flag
    writeShort(file, delay            ); // delay
    writeByte (file, 0                ); // transparent index
  }

  writeByte(file, 0); // EOF
}

void
CImageGIF::
writeData(CFile *file, CImagePtr image)
{
  compress_data.num_code_bytes = 0;

  compress_data.current_bit = 0;

  compress_data.max_code_size = 12;

  compress_data.init_code_size = compress_data.code_size + 1;

  compress_data.clear_code = 1 << compress_data.code_size;
  compress_data.eof_code   = compress_data.clear_code + 1;
  compress_data.free_code  = compress_data.clear_code + 2;

  ++compress_data.code_size;

  compress_data.max_code  = 1 << compress_data.code_size;
  compress_data.code_mask = compress_data.max_code - 1;

  clearDictionary();

  outputCode(file, compress_data.clear_code);

  int i = 0;

  uint code_value = (uint) image->getColorIndexPixel(i);

  ++i;

  int num_data = image->getWidth()*image->getHeight();

  while (i < num_data) {
    uint character = (uint) image->getColorIndexPixel(i);

    ++i;

    uint ind = findChildCode(code_value, character);

    if (compress_data.dictionary[ind].code_value != UNUSED_CODE) {
      code_value = compress_data.dictionary[ind].code_value;

      continue;
    }

    if (compress_data.free_code <= compress_data.max_code) {
      compress_data.dictionary[ind].code_value  = compress_data.free_code++;
      compress_data.dictionary[ind].parent_code = code_value;
      compress_data.dictionary[ind].character   = character;
    }

    outputCode(file, code_value);

    code_value = character;

    if (compress_data.free_code > compress_data.max_code) {
      if (compress_data.code_size >= compress_data.max_code_size) {
        outputCode(file, compress_data.clear_code);

        clearDictionary();

        compress_data.code_size = compress_data.init_code_size - 1;

        compress_data.free_code = compress_data.clear_code + 2;
      }

      ++compress_data.code_size;

      compress_data.max_code  = 1 << compress_data.code_size;
      compress_data.code_mask = compress_data.max_code - 1;
    }
  }

  outputCode(file, code_value);
  outputCode(file, compress_data.eof_code);
}

uint
CImageGIF::
findChildCode(uint parent_code, uint character)
{
  int ind = (character << (compress_data.max_code_size - 8)) ^ parent_code;

  int offset;

  if (ind == 0)
    offset = 1;
  else
    offset = CIMAGE_GIF_DICT_SIZE - ind;

  while (true) {
    if (compress_data.dictionary[ind].code_value == UNUSED_CODE)
      return ind;

    if (compress_data.dictionary[ind].parent_code == parent_code &&
        compress_data.dictionary[ind].character   == character)
      return ind;

    ind -= offset;

    if (ind < 0)
      ind += CIMAGE_GIF_DICT_SIZE;
  }
}

void
CImageGIF::
clearDictionary()
{
  for (int i = 0; i < CIMAGE_GIF_DICT_SIZE; ++i)
    compress_data.dictionary[i].code_value = UNUSED_CODE;
}

void
CImageGIF::
outputCode(CFile *file, uint code)
{
  uint code1 = code & compress_data.code_mask;

  if     (compress_data.current_bit + compress_data.code_size > 16) {
    uchar byte1 = code1 << compress_data.current_bit;
    uchar byte2 = code1 >> (8  - compress_data.current_bit);
    uchar byte3 = code1 >> (16 - compress_data.current_bit);

    compress_data.current_byte |= byte1;

    writeCodeByte(file, compress_data.current_byte);

    compress_data.current_byte = byte2;

    writeCodeByte(file, compress_data.current_byte);

    compress_data.current_byte = byte3;

    compress_data.current_bit += compress_data.code_size - 16;
  }
  else if (compress_data.current_bit + compress_data.code_size > 8) {
    uchar byte1 = code1 << compress_data.current_bit;
    uchar byte2 = code1 >> (8 - compress_data.current_bit);

    compress_data.current_byte |= byte1;

    writeCodeByte(file, compress_data.current_byte);

    compress_data.current_byte = byte2;

    compress_data.current_bit += compress_data.code_size - 8;
  }
  else {
    uchar byte1 = code1 << compress_data.current_bit;

    compress_data.current_byte |= byte1;

    compress_data.current_bit += compress_data.code_size;
  }

  if (compress_data.current_bit == 8) {
    writeCodeByte(file, compress_data.current_byte);

    compress_data.current_bit  = 0;
    compress_data.current_byte = 0;
  }

  if (code == compress_data.eof_code) {
    if (compress_data.current_bit != 0)
      writeCodeByte(file, compress_data.current_byte);

    flushCodeBytes(file);
  }
}

void
CImageGIF::
writeCodeByte(CFile *file, int data)
{
  compress_data.code_bytes[compress_data.num_code_bytes++] = data;

  if (compress_data.num_code_bytes >= 254)
    flushCodeBytes(file);
}

void
CImageGIF::
flushCodeBytes(CFile *file)
{
  if (compress_data.num_code_bytes == 0)
    return;

  uchar code_byte = compress_data.num_code_bytes;

  file->write(&code_byte, 1);

  file->write(compress_data.code_bytes, compress_data.num_code_bytes);

  compress_data.num_code_bytes = 0;
}

void
CImageGIF::
writeChars(CFile *file, const char *chars, int len)
{
  file->write((uchar *) chars, len);
}

void
CImageGIF::
writeShort(CFile *file, int data)
{
  ushort s = data;

  uchar c[2];

  c[0] =  s       & 0xff;
  c[1] = (s >> 8) & 0xff;

  file->write(c, 2);
}

void
CImageGIF::
writeByte(CFile *file, int data)
{
  uchar b = data;

  file->write(&b, 1);
}
