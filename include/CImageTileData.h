#ifndef CImageTileData_H
#define CImageTileData_H

#include <CAlignType.h>

struct CImageTileData {
  CImageTileData() { }

  CImageTileData(CHAlignType halign1, CVAlignType valign1) :
   halign(halign1), valign(valign1) {
  }

  CHAlignType halign = CHALIGN_TYPE_CENTER;
  CVAlignType valign = CVALIGN_TYPE_CENTER;
};

#endif
