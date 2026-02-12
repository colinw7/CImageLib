#include <CImageLib.h>
#include <CImagePPM.h>
#include <CStrUtil.h>

bool
CImagePPM::
read(CFile *file, CImagePtr &image)
{
  file->rewind();

  std::string line;

  file->readLine(line);

  auto len = line.size();

  if (len < 2)
    return false;

  if      (line[0] == 'P' && line[1] == '3')
    return readV3(file, image);
  else if (line[0] == 'P' && line[1] == '6')
    return readV6(file, image);
  else
    return false;
}

bool
CImagePPM::
readV3(CFile *file, CImagePtr &image)
{
  // skip comments
  std::string line;

  file->readLine(line);

  auto len = line.size();

  while (len > 0 && line[0] == '#') {
    file->readLine(line);

    len = line.size();
  }

  //---

  // read dimension
  int width, height;

  uint i = 0;
  CStrUtil::skipSpace(line, &i);

  if (! CStrUtil::readInteger(line, &i, &width))
    return false;

  CStrUtil::skipSpace(line, &i);

  if (! CStrUtil::readInteger(line, &i, &height))
    return false;

  //---

  // read max value size
  file->readLine(line);

  i = 0;
  CStrUtil::skipSpace(line, &i);

  int max_size;

  if (! CStrUtil::readInteger(line, &i, &max_size))
    return false;

  //---

  image->setType(CFILE_TYPE_IMAGE_PPM);

  image->setDataSize(width, height);

  //---

  // read pixels
  int num_pixels = width*height;

  int ip = 0;

  while (ip < num_pixels) {
    file->readLine(line);

    i = 0;
    CStrUtil::skipSpace(line, &i);

    while (i < line.size() && ip < num_pixels) {
      int ri;
      if (! CStrUtil::readInteger(line, &i, &ri))
        return false;

      CStrUtil::skipSpace(line, &i);

      int gi;
      if (! CStrUtil::readInteger(line, &i, &gi))
        return false;

      CStrUtil::skipSpace(line, &i);

      int bi;
      if (! CStrUtil::readInteger(line, &i, &bi))
        return false;

      auto r = ri/(1.0 * max_size);
      auto g = gi/(1.0 * max_size);
      auto b = bi/(1.0 * max_size);

      image->setRGBAPixel(ip++, r, g, b);

      CStrUtil::skipSpace(line, &i);
    }
  }

  return true;
}

bool
CImagePPM::
readV6(CFile *file, CImagePtr &image)
{
  // skip comments
  std::string line;

  file->readLine(line);

  auto len = line.size();

  while (len > 0 && line[0] == '#') {
    file->readLine(line);

    len = line.size();
  }

  //---

  // read dimension
  int width, height;

  uint i = 0;
  CStrUtil::skipSpace(line, &i);

  if (! CStrUtil::readInteger(line, &i, &width))
    return false;

  CStrUtil::skipSpace(line, &i);

  if (! CStrUtil::readInteger(line, &i, &height))
    return false;

  //---

  // read max value size
  file->readLine(line);

  i = 0;
  CStrUtil::skipSpace(line, &i);

  int max_size;

  if (! CStrUtil::readInteger(line, &i, &max_size))
    return false;

  //---

  // read data

  int num_pixels = width*height;

  auto buffer_size = size_t(3*num_pixels);

  auto *buffer = new uchar [buffer_size];

  if (! file->read(buffer, buffer_size))
    return false;

  //------

  image->setType(CFILE_TYPE_IMAGE_PPM);

  image->setDataSize(width, height);

  double r, g, b;

  int j = 0;

  for (i = 0; i < uint(buffer_size); i += 3, ++j) {
    r = buffer[i + 0]/(1.0 * max_size);
    g = buffer[i + 1]/(1.0 * max_size);
    b = buffer[i + 2]/(1.0 * max_size);

    image->setRGBAPixel(j, r, g, b);
  }

  delete [] buffer;

  return true;
}

bool
CImagePPM::
readHeader(CFile *, CImagePtr &)
{
  return false;
}

bool
CImagePPM::
write(CFile *, CImagePtr)
{
  return false;
}
