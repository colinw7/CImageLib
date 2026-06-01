#include <CImageLib.h>
#include <CImageDDS.h>

namespace {

using DWORD = uint;

struct DDS_PIXELFORMAT {
  DWORD dwSize;
  DWORD dwFlags;
  DWORD dwFourCC;
  DWORD dwRGBBitCount;
  DWORD dwRBitMask;
  DWORD dwGBitMask;
  DWORD dwBBitMask;
  DWORD dwABitMask;
};

struct DDS_HEADER {
  DWORD           dwSize;
  DWORD           dwFlags;
  DWORD           dwHeight;
  DWORD           dwWidth;
  DWORD           dwPitchOrLinearSize;
  DWORD           dwDepth;
  DWORD           dwMipMapCount;
  DWORD           dwReserved1[11];
  DDS_PIXELFORMAT ddspf;
  DWORD           dwCaps;
  DWORD           dwCaps2;
  DWORD           dwCaps3;
  DWORD           dwCaps4;
  DWORD           dwReserved2;
};

}

bool
CImageDDS::
read(CFile *file, CImagePtr &image)
{
  file->rewind();

  // check magic
  uchar buffer[5];

  if (! file->read(buffer, 4))
    return false;

  // 'DDS '
  if (buffer[0] != 'D' || buffer[1] != 'D' || buffer[2] != 'S' || buffer[3] != ' ')
    return false;

  //------

  DDS_HEADER header;

  if (! file->read(reinterpret_cast<uchar *>(&header), sizeof(DDS_HEADER)))
    return false;

  if (header.dwSize != 124)
    return false;

  int   num_data = header.dwWidth*header.dwHeight;
  auto *data     = new uint [size_t(num_data)];

  if (! file->read(reinterpret_cast<uchar *>(data), sizeof(uint)*num_data))
    return false;

  //------

  image->setType(CFILE_TYPE_IMAGE_DDS);

  image->setDataSize(header.dwWidth, header.dwHeight);

  image->setRGBAData(data);

  //------

  delete [] data;

  return true;
}

bool
CImageDDS::
readHeader(CFile *file, CImagePtr &)
{
  file->rewind();

  // check magic
  uchar buffer[5];

  if (! file->read(buffer, 4))
    return false;

  // 'DDS '
  if (buffer[0] != 'D' || buffer[1] != 'D' || buffer[2] != 'S' || buffer[3] != ' ')
    return false;

  //------

  return false;
}

bool
CImageDDS::
write(CFile *, CImagePtr)
{
  return false;
}
