#include <CImageLib.h>
#include <CImageWebP.h>

#ifdef IMAGE_WEBP
# include <webp/decode.h>
#endif

bool
CImageWebP::
read(CFile *file, CImagePtr &image)
{
#ifdef IMAGE_WEBP
  file->rewind();

  auto file_size = file->getSize();

  auto *data = new uchar [size_t(file_size)];

  if (! file->read(data, file_size))
    return false;

  int width, height;
  auto rc = WebPGetInfo(data, file_size, &width, &height);
  if (! rc) return false;

  image->setType(CFILE_TYPE_IMAGE_WEBP);

  image->setDataSize(width, height);

  auto *d = WebPDecodeRGBA(data, file_size, &width, &height);

  auto *imageData = new uint [size_t(width*height)];

  int i = 0, j = 0;

  for (int iy = 0; iy < height; ++iy) {
    for (int ix = 0; ix < width; ++ix) {
      imageData[j++] = image->rgbaToPixelI(d[i + 0], d[i + 1], d[i + 2], d[i + 3]);

      i += 4;
    }
  }

  image->setRGBAData(imageData);

  WebPFree(d);

  return true;
#else
  return false;
#endif
}

bool
CImageWebP::
readHeader(CFile *file, CImagePtr &image)
{
#ifdef IMAGE_WEBP
  file->rewind();

  auto file_size = file->getSize();

  auto *data = new uchar [size_t(file_size)];

  if (! file->read(data, file_size))
    return false;

  int width, height;
  auto rc = WebPGetInfo(data, file_size, &width, &height);
  if (! rc) return false;

  image->setType(CFILE_TYPE_IMAGE_WEBP);

  image->setDataSize(width, height);

  return true;
#else
  return false;
#endif
}

bool
CImageWebP::
write(CFile *, CImagePtr)
{
#ifdef IMAGE_WEBP
  return false;
#else
  return false;
#endif
}
