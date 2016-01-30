#include <CImageUtil.h>

union IntData {
  CIMAGE_INT32 l;
  CIMAGE_INT16 s;
  CIMAGE_INT8  c[sizeof(CIMAGE_INT32)];
};

CIMAGE_INT32
CImage::
swapBytes32(CIMAGE_INT32 bytes)
{
  IntData bytes1;

  bytes1.l = bytes;

  std::swap(bytes1.c[0], bytes1.c[3]);
  std::swap(bytes1.c[1], bytes1.c[2]);

  return bytes1.l;
}

CIMAGE_INT16
CImage::
swapBytes16(CIMAGE_INT16 bytes)
{
  IntData bytes1;

  bytes1.s = bytes;

  std::swap(bytes1.c[0], bytes1.c[1]);

  return bytes1.s;
}
