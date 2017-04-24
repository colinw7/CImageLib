#include <CImageLib.h>
#include <CImageJPG.h>

#include <cstring>
#include <setjmp.h>

#ifdef IMAGE_JPEG
extern "C" {
#include <jpeglib.h>
}
#endif

static jmp_buf jpeg_setjmp_buffer;

std::vector<char> CImageJPG::errorBuffer_;

bool
CImageJPG::
read(CFile *file, CImagePtr &image)
{
#ifdef IMAGE_JPEG
  struct jpeg_error_mgr         jerr;
  struct jpeg_decompress_struct cinfo;

  //------

  cinfo.err = jpeg_std_error(&jerr);

  jerr.error_exit     = jpgErrorProc;
  jerr.output_message = jpgMessageProc;

  if (setjmp(jpeg_setjmp_buffer) == 1) {
    jpeg_destroy_decompress(&cinfo);
    return false;
  }

  //------

  jpeg_create_decompress(&cinfo);

/*
  jpeg_set_marker_processor(&cinfo, JPEG_COM, jpgProcessMarker);
*/

  //------

  file->open(CFile::Mode::READ);

  file->rewind();

  jpeg_stdio_src(&cinfo, file->getFP());

  //------

  jpeg_read_header(&cinfo, TRUE);

  //------

  int depth = 24;

  if (depth != 24) {
    cinfo.quantize_colors = TRUE;

    if (cinfo.desired_number_of_colors > 256)
      cinfo.desired_number_of_colors = 256;
  }
  else
    cinfo.quantize_colors = FALSE;

  if (cinfo.jpeg_color_space == JCS_GRAYSCALE)
    cinfo.out_color_space = JCS_GRAYSCALE;
  else
    cinfo.out_color_space = JCS_RGB;

  //------

  jpeg_start_decompress(&cinfo);

  //------

  uint *data = new uint [cinfo.output_width*cinfo.output_height];

  JSAMPROW buffer;

  if (depth == 24) {
    if (cinfo.jpeg_color_space == JCS_GRAYSCALE)
      buffer = new JSAMPLE [cinfo.output_width];
    else
      buffer = new JSAMPLE [3*cinfo.output_width];
  }
  else
    buffer = new JSAMPLE [cinfo.output_width];

  JSAMPROW rowptr[1];

  rowptr[0] = buffer;

  int j = 0;

  while (cinfo.output_scanline < cinfo.output_height) {
    jpeg_read_scanlines(&cinfo, rowptr, 1);

    if (depth == 24) {
      if (cinfo.jpeg_color_space == JCS_GRAYSCALE) {
        for (uint k = 0; k < cinfo.output_width; ++k)
          data[j++] = image->rgbaToPixelI(buffer[k],
                                          buffer[k],
                                          buffer[k]);
      }
      else {
        for (uint k = 0; k < 3*cinfo.output_width; k += 3)
          data[j++] = image->rgbaToPixelI(buffer[k + 0],
                                          buffer[k + 1],
                                          buffer[k + 2]);
      }
    }
    else {
      for (uint k = 0; k < cinfo.output_width; ++k)
        data[j++] = buffer[k];
    }
  }

  delete [] buffer;

  //------

  image->setType(CFILE_TYPE_IMAGE_JPG);

  image->setDataSize(cinfo.output_width, cinfo.output_height);

  if (depth != 24) {
    if (cinfo.out_color_space == JCS_RGB) {
      int r, g, b;

      for (int i = 0; i < cinfo.actual_number_of_colors; ++i) {
        b = cinfo.colormap[0][i];
        g = cinfo.colormap[1][i];
        r = cinfo.colormap[2][i];

        image->addColorI(r, g, b);

        if (CImageState::getDebug())
          printf("%d) R %d G %d B %d\n", i + 1, r, g, b);
      }
    }
    else {
      int g;

      for (int i = 0; i < cinfo.actual_number_of_colors; ++i) {
        g = cinfo.colormap[0][i];

        image->addColorI(g, g, g);

        if (CImageState::getDebug())
          printf("%d) R %d G %d B %d\n", i + 1, g, g, g);
      }
    }

    image->setColorIndexData(data);
  }
  else
    image->setRGBAData(data);

  delete [] data;

  //------

  jpeg_finish_decompress(&cinfo);

  //------

  jpeg_destroy_decompress(&cinfo);

  //------

  return true;
#else
  return false;
#endif
}

bool
CImageJPG::
readHeader(CFile *file, CImagePtr &image)
{
#ifdef IMAGE_JPEG
  struct jpeg_error_mgr         jerr;
  struct jpeg_decompress_struct cinfo;

  //------

  cinfo.err = jpeg_std_error(&jerr);

  jerr.error_exit     = jpgErrorProc;
  jerr.output_message = jpgMessageProc;

  if (setjmp(jpeg_setjmp_buffer) == 1) {
    jpeg_destroy_decompress(&cinfo);
    return false;
  }

  //------

  jpeg_create_decompress(&cinfo);

/*
  jpeg_set_marker_processor(&cinfo, JPEG_COM, jpgProcessMarker);
*/

  //------

  file->open(CFile::Mode::READ);

  file->rewind();

  jpeg_stdio_src(&cinfo, file->getFP());

  //------

  jpeg_read_header(&cinfo, TRUE);

  //------

  image->setType(CFILE_TYPE_IMAGE_JPG);

  image->setSize(cinfo.image_width, cinfo.image_height);

  //------

  jpeg_destroy_decompress(&cinfo);

  //------

  return true;
#else
  return false;
#endif
}

#ifdef IMAGE_JPEG
int
CImageJPG::
jpgProcessMarker(struct jpeg_decompress_struct *cinfo)
{
  if (CImageState::getDebug())
    CImage::infoMsg("In jpgProcessMarker");

  int length = 0;

  length += jpgGetC(cinfo) << 8;
  length += jpgGetC(cinfo) - 2;

  for (int i = 0; i < length; ++i)
    jpgGetC(cinfo);

  return 1;
}

int
CImageJPG::
jpgGetC(struct jpeg_decompress_struct *cinfo)
{
  struct jpeg_source_mgr *datasrc = cinfo->src;

  if (datasrc->bytes_in_buffer == 0) {
    if (! (*datasrc->fill_input_buffer)(cinfo))
      longjmp(jpeg_setjmp_buffer, 1);
  }

  --datasrc->bytes_in_buffer;

  return GETJOCTET(*datasrc->next_input_byte++);
}

void
CImageJPG::
jpgErrorProc(struct jpeg_common_struct *cinfo)
{
  (*cinfo->err->output_message)(cinfo);

  CImage::errorMsg("JPEG Error '" + std::string(&errorBuffer_[0]) + "'");

  longjmp(jpeg_setjmp_buffer, 1);
}

void
CImageJPG::
jpgMessageProc(struct jpeg_common_struct *cinfo)
{
  errorBuffer_.resize(JMSG_LENGTH_MAX + 1);

  (*cinfo->err->format_message)(cinfo, &errorBuffer_[0]);
}
#endif

bool
CImageJPG::
write(CFile *file, CImagePtr image)
{
#ifdef IMAGE_JPEG
  struct jpeg_error_mgr       jerr;
  struct jpeg_compress_struct cinfo;

  cinfo.err = jpeg_std_error(&jerr);

  jpeg_create_compress(&cinfo);

  file->open(CFile::Mode::WRITE);

  jpeg_stdio_dest(&cinfo, file->getFP());

  cinfo.image_width      = image->getWidth();
  cinfo.image_height     = image->getHeight();
  cinfo.input_components = 3;
  cinfo.in_color_space   = JCS_RGB;

  jpeg_set_defaults(&cinfo);

  jpeg_start_compress(&cinfo, TRUE);

  uchar *data = new uchar [3*image->getWidth()];

  JSAMPROW row_pointer[1];

  row_pointer[0] = data;

  int k = 0;

  uint r, g, b, a;

  while (cinfo.next_scanline < cinfo.image_height) {
    int j = 0;

    for (uint i = 0; i < image->getWidth(); ++i, j += 3, ++k) {
      image->getRGBAPixelI(k, &r, &g, &b, &a);

      data[j + 2] = b;
      data[j + 1] = g;
      data[j + 0] = r;
    }

    jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  jpeg_finish_compress(&cinfo);

  jpeg_destroy_compress(&cinfo);

  delete [] data;

  return true;
#else
  return false;
#endif
}
