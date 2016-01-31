#ifndef CIMAGE_XPM_H
#define CIMAGE_XPM_H

#include <CRGBA.h>
#include <CImageFmt.h>

#define CImageXPMInst CImageXPM::getInstance()

struct CImageXPMData;
struct CImageXPMColor;

class CImageXPM : public CImageFmt {
 public:
  static CImageXPM *getInstance() {
    static CImageXPM *instance;

    if (! instance)
      instance = new CImageXPM;

    return instance;
  }

  bool read(CFile *file, CImagePtr &image);

  bool read(const char **strings, uint num_strings, CImagePtr &image);

  bool readHeader(CFile *file, CImagePtr &image);

  bool write(CFile *file, CImagePtr image);

  void setHotSpot(int x, int y);

 private:
  CImageXPM() :
   CImageFmt(CFILE_TYPE_IMAGE_XPM) {
  }

 ~CImageXPM() { }

  CImageXPM(const CImageXPM &xpm);

  const CImageXPM &operator=(const CImageXPM &xpm);

 private:
  bool readHeader(const char *data, int *i);

  bool skipDcl(const char *data, int *i);

  bool readValues(const char *data, int *i, CImageXPMData *xpm_data);
  bool readValuesString(CImageXPMData *xpm_data, const char *str);
  bool readColors(const char *data, int *i, CImageXPMData *xpm_data);
  bool readColorString(CImageXPMData *xpm_data, const char *str,
                       CImageXPMColor *color);

  void skipToColorKey(const char *str, int *i);

  bool isColorKey(const char *str);

  bool readData(const char *data, int *i, CImageXPMData *xpm_data);

  bool readDataString(CImageXPMData *xpm_data, const char *str,
                      uint *data, int *pos);
  bool readData24String(CImageXPMData *xpm_data, CRGBA *colors, const char *str,
                        uint *data, int *pos);

  CRGBA *createImageColors(CImageXPMData *xpm_data);

  bool skipComment(const char *data, int *i);

  char *readString(const char *data, int *i);

  bool lookupColor(const std::string &name, CRGBA &color);

  void getColorUsage(CImagePtr image, char **used, int *num_used);

  std::string pixelToSymbol(int pixel);
  std::string colorToString(CImagePtr image, int color);
  std::string colorToMonoString(CImagePtr image, int color);
};

#endif
