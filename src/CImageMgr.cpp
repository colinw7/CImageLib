// TODO: Two Pass Loading. Just Size and then Data using Thread
//       Unload data. Default Image when no Data (Loading)

#include <CImageLibI.h>
#include <CImageBMP.h>
#include <CImageGIF.h>
#include <CImageICO.h>
#include <CImageIFF.h>
#include <CImageJPG.h>
#include <CImagePCX.h>
#include <CImagePNG.h>
#include <CImagePPM.h>
#include <CImagePSP.h>
#include <CImageSGI.h>
#include <CImageSIX.h>
#include <CImageTGA.h>
#include <CImageTIF.h>
#include <CImageWebP.h>
#include <CImageXBM.h>
#include <CImageXPM.h>
#include <CImageXWD.h>

CImageThumbnailMgr::
CImageThumbnailMgr(uint width, uint height) :
 size_(int(width), int(height))
{
}

CImageThumbnailMgr::
~CImageThumbnailMgr()
{
}

CImagePtr
CImageThumbnailMgr::
lookupImage(const std::string &name)
{
  CImageSizedFileSrc src(name, size_.width, size_.height, true);

  return image_mgr_.lookupImage(src);
}

//--------------

static CImageMgr *s_instance;

CImageMgr *
CImageMgr::
instance()
{
  if (! s_instance)
    s_instance = new CImageMgr;

  return s_instance;
}

void
CImageMgr::
release()
{
  delete s_instance;

  s_instance = nullptr;
}

CImageMgr::
CImageMgr()
{
  addFmt(CFILE_TYPE_IMAGE_BMP , CImageBMPInst );
  addFmt(CFILE_TYPE_IMAGE_GIF , CImageGIFInst );
  addFmt(CFILE_TYPE_IMAGE_ICO , CImageICOInst );
  addFmt(CFILE_TYPE_IMAGE_IFF , CImageIFFInst );
  addFmt(CFILE_TYPE_IMAGE_JPG , CImageJPGInst );
  addFmt(CFILE_TYPE_IMAGE_PCX , CImagePCXInst );
  addFmt(CFILE_TYPE_IMAGE_PNG , CImagePNGInst );
  addFmt(CFILE_TYPE_IMAGE_PPM , CImagePPMInst );
  addFmt(CFILE_TYPE_IMAGE_PSP , CImagePSPInst );
  addFmt(CFILE_TYPE_IMAGE_SGI , CImageSGIInst );
  addFmt(CFILE_TYPE_IMAGE_SIX , CImageSIXInst );
  addFmt(CFILE_TYPE_IMAGE_TGA , CImageTGAInst );
  addFmt(CFILE_TYPE_IMAGE_TIF , CImageTIFInst );
  addFmt(CFILE_TYPE_IMAGE_WEBP, CImageWebPInst);
  addFmt(CFILE_TYPE_IMAGE_XBM , CImageXBMInst );
  addFmt(CFILE_TYPE_IMAGE_XPM , CImageXPMInst );
  addFmt(CFILE_TYPE_IMAGE_XWD , CImageXWDInst );
}

CImageMgr::
~CImageMgr()
{
  clearFmts();

  clearFileMap();
}

bool
CImageMgr::
addFmt(CFileType type, CImageFmt *fmt)
{
  fmt_map_[type] = fmt;

  return true;
}

bool
CImageMgr::
getFmt(CFileType type, CImageFmt **fmt)
{
  *fmt = nullptr;

  auto p = fmt_map_.find(type);

  if (p == fmt_map_.end())
    return false;

  *fmt = (*p).second;

  return true;
}

void
CImageMgr::
clearFmts()
{
  for (auto &fmt_map : fmt_map_)
    delete fmt_map.second;

  fmt_map_.clear();
}

void
CImageMgr::
setPrototype(CImagePtr ptr)
{
  prototype_ = ptr;
}

CImagePtr
CImageMgr::
createImageI()
{
  creating_ = true;

  auto image = newImage();

  creating_ = false;

  return image;
}

CImagePtr
CImageMgr::
createImage(const CImageSrc &src)
{
  auto ptr = lookupImage(src);

  if (! ptr) {
    creating_ = true;

    ptr = newImage();

    creating_ = false;
  }

  return ptr;
}

CImagePtr
CImageMgr::
createImageI(const CISize2D &size)
{
  creating_ = true;

  auto image = newImage();

  image->setDataSize(size);

  creating_ = false;

  return image;
}

CImagePtr
CImageMgr::
createImageI(int width, int height)
{
  return createImageI(CISize2D(width, height));
}

CImagePtr
CImageMgr::
createImageI(const CImage &image)
{
  creating_ = true;

  auto image1 = newImage();

  *image1 = image;

  creating_ = false;

  return CImagePtr(image1);
}

CImagePtr
CImageMgr::
createImageI(const CImage &image, int x, int y, int width, int height)
{
  creating_ = true;

  auto image1 = newImage();

  image1->setDataSize(width, height);

  *image1 = *image.subImage(x, y, width, height).get();

  creating_ = false;

  return image1;
}

bool
CImageMgr::
addImage(CImage *image)
{
  image_list_.push_back(image);

  return true;
}

bool
CImageMgr::
deleteImage(CImage *image)
{
  image_list_.remove(image);

  return true;
}

CImagePtr
CImageMgr::
lookupImage(const std::string &fileName)
{
  CImageFileSrc src(fileName);

  return lookupImage(src);
}

CImagePtr
CImageMgr::
lookupImage(const CImageSrc &src)
{
  auto *file = lookupFile(src);

  if (file)
    return CImagePtr(file->getImage());

  //---

  if      (src.isType(CImageSrc::DATA_SRC)) {
    const auto *data = dynamic_cast<const CImageDataSrc *>(&src);

    creating_ = true;

    auto image = newImage();

    creating_ = false;

    if (! image->read(reinterpret_cast<const uchar *>(data->getDataP()), data->getDataLen())) {
      CImage::errorMsg("Failed to read image");
      return CImagePtr();
    }

    return image;
  }
  else if (src.isType(CImageSrc::XPM_SRC)) {
    const auto *data = dynamic_cast<const CImageXPMSrc *>(&src);

    creating_ = true;

    auto image = newImage();

    creating_ = false;

    if (! image->readXPM(data->getStrs(), data->getNumStrs())) {
      CImage::errorMsg("Failed to read image");
      return CImagePtr();
    }

    return image;
  }
  else if (src.isType(CImageSrc::XBM_SRC)) {
    const auto *data = dynamic_cast<const CImageXBMSrc *>(&src);

    creating_ = true;

    auto image = newImage();

    creating_ = false;

    if (! image->readXBM(data->getData(), data->getWidth(), data->getHeight())) {
      CImage::errorMsg("Failed to read image");
      return CImagePtr();
    }

    return image;
  }

  return CImagePtr();
}

CImageFile *
CImageMgr::
lookupFile(const CImageSrc &src)
{
  if      (src.isType(CImageSrc::SIZED_FILE_SRC)) {
    const auto *sized_file = dynamic_cast<const CImageSizedFileSrc *>(&src);

    const std::string &fileName = sized_file->getFilename();

    int  width       = sized_file->getWidth();
    int  height      = sized_file->getHeight();
    bool keep_aspect = sized_file->getKeepAspect();

    auto *sfile = lookupSizedFile(fileName, width, height, keep_aspect);

    return sfile;
  }
  else if (src.isType(CImageSrc::FILE_SRC)) {
    const auto *file = dynamic_cast<const CImageFileSrc *>(&src);

    const std::string &fileName = file->getFilename();

    auto *ifile = lookupFile(fileName);

    return ifile;
  }
  else
    return nullptr;
}

CImageFile *
CImageMgr::
lookupFile(const std::string &fileName)
{
  auto *data = lookupFileI(fileName);

  if (data)
    return data;

  return addFile(fileName);
}

CImageFile *
CImageMgr::
lookupFileI(const std::string &fileName)
{
  auto p = image_file_map_.find(fileName);

  if (p != image_file_map_.end())
    return (*p).second;

  return nullptr;
}

CImageFile *
CImageMgr::
addFile(const std::string &fileName)
{
  auto *data = addFileI(fileName);

  return data;
}

CImageFile *
CImageMgr::
addFileI(const std::string &fileName)
{
  auto *file = new CImageFile(fileName);

  image_file_map_[fileName] = file;

  if (debug_)
    CImage::infoMsg("Created Image File '" + fileName + "'");

  return file;
}

bool
CImageMgr::
removeFile(const std::string &fileName)
{
  image_file_map_[fileName] = nullptr;

  return true;
}

void
CImageMgr::
clearFileMap()
{
  for (auto &image_file : image_file_map_)
    delete image_file.second;

  image_file_map_.clear();
}

CImageSizedFile *
CImageMgr::
lookupSizedFile(const std::string &fileName, int width, int height, bool keep_aspect)
{
  auto *data = lookupSizedFileI(fileName, width, height, keep_aspect);

  if (data)
    return data;

  return addSizedFile(fileName, width, height, keep_aspect);
}

CImageSizedFile *
CImageMgr::
lookupSizedFileI(const std::string &fileName, int width, int height, bool keep_aspect)
{
  auto *data = lookupFileI(fileName);

  if (data)
    return data->lookupSizedFile(width, height, keep_aspect);

  return nullptr;
}

CImageSizedFile *
CImageMgr::
addSizedFile(const std::string &fileName, int width, int height, bool keep_aspect)
{
  auto *data = addSizedFileI(fileName, width, height, keep_aspect);

  return data;
}

CImageSizedFile *
CImageMgr::
addSizedFileI(const std::string &fileName, int width, int height, bool keep_aspect)
{
  auto *data = lookupFileI(fileName);

  if (! data)
    data = addFileI(fileName);

  std::string fileName1 = fileName +
    "?width=" + CStrUtil::toString(width) +
    "&height=" + CStrUtil::toString(height) +
    "&keep_aspect=" + CStrUtil::toString(keep_aspect);

  auto *sized_file = data->addSizedFile(width, height, keep_aspect);

  image_sized_file_map_[fileName1] = sized_file;

  if (debug_)
    CImage::infoMsg("Created Sized Image File '" + fileName + "'");

  return sized_file;
}

bool
CImageMgr::
removeSizedFile(const std::string &fileName, int width, int height, bool keep_aspect)
{
  auto *data = lookupFileI(fileName);

  if (! data)
    return false;

  return data->removeSizedFile(width, height, keep_aspect);
}

CImagePtr
CImageMgr::
newImage()
{
  if (prototype_)
    return prototype_->dup();

  auto *image = new CImage;

  return CImagePtr(image);
}

//---

std::string
CImageSizedFileSrc::
getName() const
{
  return "sized_file:" + getFilename() +
         "?width="  + CStrUtil::toString(width_) +
         "&height=" + CStrUtil::toString(height_) +
         "&keep_aspect=" + (keep_aspect_ ? "true" : "false");
}

//---

std::string
CImageDataSrc::
getName() const
{
  return "data:" + CStrUtil::toString(id_);
}

//---

std::string
CImageXPMSrc::
getName() const
{
  return "xpm:" + CStrUtil::toString(id_);
}

//---

std::string
CImageXBMSrc::
getName() const
{
  return "xbm:" + CStrUtil::toString(id_);
}
