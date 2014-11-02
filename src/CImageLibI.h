#include <CFile.h>
#include <CTempFile.h>
#include <CStrUtil.h>
#include <CRGBA.h>
#include <CRGB.h>
#include <CThrow.h>
#include <CFileUtil.h>
#include <CImageLib.h>
#include <CImageFmt.h>
#include <CImageColorDef.h>

class CAssert {
 public:
  static bool exec(bool e, const char *m) {
    if (! e) {
      std::cerr << m << std::endl;

      assert(false);
    }

    return e;
  }
};

#define CASSERT(e, m) CAssert::exec(e, m)
