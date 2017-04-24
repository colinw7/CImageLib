#include <CImageGen.h>
#include <CImageGIF.h>
#include <CImageXBM.h>
#include <CImageXPM.h>

CImagePtr
CImage::
create(const std::string &filename)
{
  CFile file(filename);

  return create(&file);
}

CImagePtr
CImage::
create(const std::string &filename, CFileType type)
{
  CFile file(filename);

  return create(&file, type);
}

CImagePtr
CImage::
create(const char *filename)
{
  CFile file(filename);

  return create(&file);
}

CImagePtr
CImage::
create(const char *filename, CFileType type)
{
  CFile file(filename);

  return create(&file, type);
}

CImagePtr
CImage::
create(const uchar *data, size_t len)
{
  CTempFile temp_file;

  CFile *file = temp_file.getFile();

  file->write(data, len);

  file->close();

  return create(file);
}

CImagePtr
CImage::
create(CFile *file)
{
  CFileType type = CFileUtil::getImageType(file);

  if (type == CFILE_TYPE_NONE || type == CFILE_TYPE_INODE_REG)
    type = CFileUtil::getImageTypeFromName(file->getName());

  return create(file, type);
}

CImagePtr
CImage::
createBMP(CFile *file)
{
  return create(file, CFILE_TYPE_IMAGE_BMP);
}

CImagePtr
CImage::
createEPS(CFile *file)
{
  return create(file, CFILE_TYPE_IMAGE_EPS);
}

CImagePtr
CImage::
createGIF(CFile *file)
{
  return create(file, CFILE_TYPE_IMAGE_GIF);
}

CImagePtr
CImage::
createICO(CFile *file)
{
  return create(file, CFILE_TYPE_IMAGE_ICO);
}

CImagePtr
CImage::
createIFF(CFile *file)
{
  return create(file, CFILE_TYPE_IMAGE_IFF);
}

CImagePtr
CImage::
createJPG(CFile *file)
{
  return create(file, CFILE_TYPE_IMAGE_JPG);
}

CImagePtr
CImage::
createPCX(CFile *file)
{
  return create(file, CFILE_TYPE_IMAGE_PCX);
}

CImagePtr
CImage::
createPNG(CFile *file)
{
  return create(file, CFILE_TYPE_IMAGE_PNG);
}

CImagePtr
CImage::
createPPM(CFile *file)
{
  return create(file, CFILE_TYPE_IMAGE_PPM);
}

CImagePtr
CImage::
createPS(CFile *file)
{
  return create(file, CFILE_TYPE_IMAGE_PS);
}

CImagePtr
CImage::
createPSP(CFile *file)
{
  return create(file, CFILE_TYPE_IMAGE_PSP);
}

CImagePtr
CImage::
createSGI(CFile *file)
{
  return create(file, CFILE_TYPE_IMAGE_SGI);
}

CImagePtr
CImage::
createSIX(CFile *file)
{
  return create(file, CFILE_TYPE_IMAGE_SIX);
}

CImagePtr
CImage::
createSVG(CFile *file)
{
  return create(file, CFILE_TYPE_IMAGE_SVG);
}

CImagePtr
CImage::
createTGA(CFile *file)
{
  return create(file, CFILE_TYPE_IMAGE_TGA);
}

CImagePtr
CImage::
createTIF(CFile *file)
{
  return create(file, CFILE_TYPE_IMAGE_TIF);
}

CImagePtr
CImage::
createXBM(CFile *file)
{
  return create(file, CFILE_TYPE_IMAGE_XBM);
}

CImagePtr
CImage::
createXPM(CFile *file)
{
  return create(file, CFILE_TYPE_IMAGE_XPM);
}

CImagePtr
CImage::
createXPM(const char **strings, uint num_strings)
{
  return create(strings, num_strings, CFILE_TYPE_IMAGE_XPM);
}

CImagePtr
CImage::
createXWD(CFile *file)
{
  return create(file, CFILE_TYPE_IMAGE_XWD);
}

CImagePtr
CImage::
create(CFile *file, CFileType type)
{
  CImageFmt *fmt;

  if (CImageMgrInst->getFmt(type, &fmt)) {
    CImagePtr image = CImageMgrInst->createImage();

    if (! fmt->read(file, image))
      return CImagePtr();

    return image;
  }

  return CImagePtr();
}

CImagePtr
CImage::
create(const char **strings, uint num_strings, CFileType type)
{
  if (type == CFILE_TYPE_IMAGE_XPM) {
    CImagePtr image = CImageMgrInst->createImage();

    if (! image->readXPM(strings, num_strings))
      return CImagePtr();

    return image;
  }
  else {
    CASSERT(false, "Unsupported type for CImage::create");

    return CImagePtr();
  }
}

//-----------

CImagePtr
CImage::
createHeader(const std::string &filename)
{
  CFile file(filename);

  return createHeader(&file);
}

CImagePtr
CImage::
createHeader(const std::string &filename, CFileType type)
{
  CFile file(filename);

  return createHeader(&file, type);
}

CImagePtr
CImage::
createHeader(CFile *file)
{
  CFileType type = CFileUtil::getImageType(file);

  if (type == CFILE_TYPE_NONE || type == CFILE_TYPE_INODE_REG)
    type = CFileUtil::getImageTypeFromName(file->getName());

  return createHeader(file, type);
}

CImagePtr
CImage::
createBMPHeader(CFile *file)
{
  return createHeader(file, CFILE_TYPE_IMAGE_BMP);
}

CImagePtr
CImage::
createEPSHeader(CFile *file)
{
  return createHeader(file, CFILE_TYPE_IMAGE_EPS);
}

CImagePtr
CImage::
createGIFHeader(CFile *file)
{
  return createHeader(file, CFILE_TYPE_IMAGE_GIF);
}

CImagePtr
CImage::
createICOHeader(CFile *file)
{
  return createHeader(file, CFILE_TYPE_IMAGE_ICO);
}

CImagePtr
CImage::
createIFFHeader(CFile *file)
{
  return createHeader(file, CFILE_TYPE_IMAGE_IFF);
}

CImagePtr
CImage::
createJPGHeader(CFile *file)
{
  return createHeader(file, CFILE_TYPE_IMAGE_JPG);
}

CImagePtr
CImage::
createPCXHeader(CFile *file)
{
  return createHeader(file, CFILE_TYPE_IMAGE_PCX);
}

CImagePtr
CImage::
createPNGHeader(CFile *file)
{
  return createHeader(file, CFILE_TYPE_IMAGE_PNG);
}

CImagePtr
CImage::
createPPMHeader(CFile *file)
{
  return createHeader(file, CFILE_TYPE_IMAGE_PPM);
}

CImagePtr
CImage::
createPSHeader(CFile *file)
{
  return createHeader(file, CFILE_TYPE_IMAGE_PS);
}

CImagePtr
CImage::
createPSPHeader(CFile *file)
{
  return createHeader(file, CFILE_TYPE_IMAGE_PSP);
}

CImagePtr
CImage::
createSGIHeader(CFile *file)
{
  return createHeader(file, CFILE_TYPE_IMAGE_SGI);
}

CImagePtr
CImage::
createSVGHeader(CFile *file)
{
  return createHeader(file, CFILE_TYPE_IMAGE_SVG);
}

CImagePtr
CImage::
createTGAHeader(CFile *file)
{
  return createHeader(file, CFILE_TYPE_IMAGE_TGA);
}

CImagePtr
CImage::
createTIFHeader(CFile *file)
{
  return createHeader(file, CFILE_TYPE_IMAGE_TIF);
}

CImagePtr
CImage::
createXBMHeader(CFile *file)
{
  return createHeader(file, CFILE_TYPE_IMAGE_XBM);
}

CImagePtr
CImage::
createXPMHeader(CFile *file)
{
  return createHeader(file, CFILE_TYPE_IMAGE_XPM);
}

CImagePtr
CImage::
createXWDHeader(CFile *file)
{
  return createHeader(file, CFILE_TYPE_IMAGE_XWD);
}

CImagePtr
CImage::
createHeader(CFile *file, CFileType type)
{
  CImageFmt *fmt;

  if (CImageMgrInst->getFmt(type, &fmt)) {
    CImagePtr image = CImageMgrInst->createImage();

    if (! fmt->readHeader(file, image))
      return CImagePtr();

    return image;
  }

  CImagePtr image;

  if (! (type & CFILE_TYPE_IMAGE)) {
    type = CFileUtil::getImageType(file);

    if (type == CFILE_TYPE_NONE || type == CFILE_TYPE_INODE_REG)
      type = CFileUtil::getImageTypeFromName(file->getName());
  }

  switch (type) {
    case CFILE_TYPE_IMAGE_SGI:
      image = CImage::createSGIHeader(file);
      break;
    case CFILE_TYPE_IMAGE_XBM:
      image = CImage::createXBMHeader(file);
      break;
    case CFILE_TYPE_IMAGE_XPM:
      image = CImage::createXPMHeader(file);
      break;
    case CFILE_TYPE_IMAGE_XWD:
      image = CImage::createXWDHeader(file);
      break;
    default:
      image = CImagePtr();
      break;
  }

  return image;
}

//-----------

bool
CImage::
read(const uchar *data, size_t len, CFileType type)
{
  CTempFile temp_file;

  CFile *file = temp_file.getFile();

  file->write(data, len);

  file->close();

  if (type != CFILE_TYPE_NONE)
    return read(file, type);
  else
    return read(file);
}

bool
CImage::
read(const std::string &filename, CFileType type)
{
  CFile file(filename);

  if (type != CFILE_TYPE_NONE)
    return read(&file, type);
  else
    return read(&file);
}

bool
CImage::
read(CFile *file)
{
  CFileType type = CFileUtil::getType(file);

  if (type == CFILE_TYPE_NONE || type == CFILE_TYPE_INODE_REG)
    type = CFileUtil::getImageTypeFromName(file->getName());

  if (! (type & CFILE_TYPE_IMAGE))
    return false;

  return read(file, type);
}

bool
CImage::
readBMP(CFile *file)
{
  return read(file, CFILE_TYPE_IMAGE_BMP);
}

bool
CImage::
readEPS(CFile *file)
{
  return read(file, CFILE_TYPE_IMAGE_EPS);
}

bool
CImage::
readGIF(CFile *file)
{
  return read(file, CFILE_TYPE_IMAGE_GIF);
}

bool
CImage::
readICO(CFile *file)
{
  return read(file, CFILE_TYPE_IMAGE_ICO);
}

bool
CImage::
readIFF(CFile *file)
{
  return read(file, CFILE_TYPE_IMAGE_IFF);
}

bool
CImage::
readJPG(CFile *file)
{
  return read(file, CFILE_TYPE_IMAGE_JPG);
}

bool
CImage::
readPCX(CFile *file)
{
  return read(file, CFILE_TYPE_IMAGE_PCX);
}

bool
CImage::
readPNG(CFile *file)
{
  return read(file, CFILE_TYPE_IMAGE_PNG);
}

bool
CImage::
readPPM(CFile *file)
{
  return read(file, CFILE_TYPE_IMAGE_PPM);
}

bool
CImage::
readPS(CFile *file)
{
  return read(file, CFILE_TYPE_IMAGE_PS);
}

bool
CImage::
readPSP(CFile *file)
{
  return read(file, CFILE_TYPE_IMAGE_PSP);
}

bool
CImage::
readSGI(CFile *file)
{
  return read(file, CFILE_TYPE_IMAGE_SGI);
}

bool
CImage::
readSIX(CFile *file)
{
  return read(file, CFILE_TYPE_IMAGE_SIX);
}

bool
CImage::
readSVG(CFile *file)
{
  return read(file, CFILE_TYPE_IMAGE_SVG);
}

bool
CImage::
readTGA(CFile *file)
{
  return read(file, CFILE_TYPE_IMAGE_TGA);
}

bool
CImage::
readTIF(CFile *file)
{
  return read(file, CFILE_TYPE_IMAGE_TIF);
}

bool
CImage::
readXBM(CFile *file)
{
  return read(file, CFILE_TYPE_IMAGE_XBM);
}

bool
CImage::
readXPM(CFile *file)
{
  return read(file, CFILE_TYPE_IMAGE_XPM);
}

bool
CImage::
readXWD(CFile *file)
{
  return read(file, CFILE_TYPE_IMAGE_XWD);
}

bool
CImage::
read(CFile *file, CFileType type)
{
  CImageFmt *fmt;

  if (CImageMgrInst->getFmt(type, &fmt)) {
    CImagePtr image = dup();

    bool flag = fmt->read(file, image);

    if (! flag)
      return false;

    replace(image);

    return true;
  }

  return false;
}

//-----------

bool
CImage::
readHeader(const uchar *data, size_t len)
{
  CTempFile temp_file;

  CFile *file = temp_file.getFile();

  file->write(data, len);

  file->close();

  return readHeader(file);
}

bool
CImage::
readHeader(const std::string &filename)
{
  CFile file(filename);

  return readHeader(&file);
}

bool
CImage::
readHeader(CFile *file)
{
  CFileType type = CFileUtil::getType(file);

  if (! (type & CFILE_TYPE_IMAGE))
    return false;

  return readHeader(file, type);
}


bool
CImage::
readBMPHeader(CFile *file)
{
  return readHeader(file, CFILE_TYPE_IMAGE_BMP);
}

bool
CImage::
readEPSHeader(CFile *file)
{
  return readHeader(file, CFILE_TYPE_IMAGE_EPS);
}

bool
CImage::
readGIFHeader(CFile *file)
{
  return readHeader(file, CFILE_TYPE_IMAGE_GIF);
}

bool
CImage::
readICOHeader(CFile *file)
{
  return readHeader(file, CFILE_TYPE_IMAGE_ICO);
}

bool
CImage::
readIFFHeader(CFile *file)
{
  return readHeader(file, CFILE_TYPE_IMAGE_IFF);
}

bool
CImage::
readJPGHeader(CFile *file)
{
  return readHeader(file, CFILE_TYPE_IMAGE_JPG);
}

bool
CImage::
readPCXHeader(CFile *file)
{
  return readHeader(file, CFILE_TYPE_IMAGE_PCX);
}

bool
CImage::
readPNGHeader(CFile *file)
{
  return readHeader(file, CFILE_TYPE_IMAGE_PNG);
}

bool
CImage::
readPPMHeader(CFile *file)
{
  return readHeader(file, CFILE_TYPE_IMAGE_PPM);
}

bool
CImage::
readPSHeader(CFile *file)
{
  return readHeader(file, CFILE_TYPE_IMAGE_PS);
}

bool
CImage::
readPSPHeader(CFile *file)
{
  return readHeader(file, CFILE_TYPE_IMAGE_PSP);
}

bool
CImage::
readSGIHeader(CFile *file)
{
  return readHeader(file, CFILE_TYPE_IMAGE_SGI);
}

bool
CImage::
readSVGHeader(CFile *file)
{
  return readHeader(file, CFILE_TYPE_IMAGE_SVG);
}

bool
CImage::
readTGAHeader(CFile *file)
{
  return readHeader(file, CFILE_TYPE_IMAGE_TGA);
}

bool
CImage::
readTIFHeader(CFile *file)
{
  return readHeader(file, CFILE_TYPE_IMAGE_TIF);
}

bool
CImage::
readXBMHeader(CFile *file)
{
  return readHeader(file, CFILE_TYPE_IMAGE_XBM);
}

bool
CImage::
readXPMHeader(CFile *file)
{
  return readHeader(file, CFILE_TYPE_IMAGE_XPM);
}

bool
CImage::
readXWDHeader(CFile *file)
{
  return readHeader(file, CFILE_TYPE_IMAGE_XWD);
}

bool
CImage::
readHeader(CFile *file, CFileType type)
{
  CImageFmt *fmt;

  if (CImageMgrInst->getFmt(type, &fmt)) {
    CImagePtr image = dup();

    bool flag = fmt->readHeader(file, image);

    replace(image);

    return flag;
  }

  return false;
}

//-----------

bool
CImage::
write(const std::string &filename, CFileType type)
{
  CFile file(filename);

  return write(&file, type);
}

bool
CImage::
write(const char *filename, CFileType type)
{
  CFile file(filename);

  return write(&file, type);
}

bool
CImage::
write(const std::string &filename)
{
  CFile file(filename);

  return write(&file);
}

bool
CImage::
write(const char *filename)
{
  CFile file(filename);

  return write(&file);
}

bool
CImage::
write(CFile *file)
{
  CFileType type = getType();

  if (type == CFILE_TYPE_NONE || type == CFILE_TYPE_INODE_REG)
    type = CFileUtil::getImageTypeFromName(file->getName());

  return write(file, type);
}

bool
CImage::
writeBMP(CFile *file)
{
  return write(file, CFILE_TYPE_IMAGE_BMP);
}

bool
CImage::
writeEPS(CFile *file)
{
  return write(file, CFILE_TYPE_IMAGE_EPS);
}

bool
CImage::
writeGIF(CFile *file)
{
  return write(file, CFILE_TYPE_IMAGE_GIF);
}

bool
CImage::
writeICO(CFile *file)
{
  return write(file, CFILE_TYPE_IMAGE_ICO);
}

bool
CImage::
writeIFF(CFile *file)
{
  return write(file, CFILE_TYPE_IMAGE_IFF);
}

bool
CImage::
writeJPG(CFile *file)
{
  return write(file, CFILE_TYPE_IMAGE_JPG);
}

bool
CImage::
writePCX(CFile *file)
{
  return write(file, CFILE_TYPE_IMAGE_PCX);
}

bool
CImage::
writePNG(const std::string &filename)
{
  CFile file(filename);

  return writePNG(&file);
}

bool
CImage::
writePNG(CFile *file)
{
  return write(file, CFILE_TYPE_IMAGE_PNG);
}

bool
CImage::
writePPM(CFile *file)
{
  return write(file, CFILE_TYPE_IMAGE_PPM);
}

bool
CImage::
writePS(CFile *file)
{
  return write(file, CFILE_TYPE_IMAGE_PS);
}

bool
CImage::
writePSP(CFile *file)
{
  return write(file, CFILE_TYPE_IMAGE_PSP);
}

bool
CImage::
writeSGI(CFile *file)
{
  return write(file, CFILE_TYPE_IMAGE_SGI);
}

bool
CImage::
writeSIX(CFile *file)
{
  return write(file, CFILE_TYPE_IMAGE_SIX);
}

bool
CImage::
writeSVG(CFile *file)
{
  return write(file, CFILE_TYPE_IMAGE_SVG);
}

bool
CImage::
writeTGA(CFile *file)
{
  return write(file, CFILE_TYPE_IMAGE_TGA);
}

bool
CImage::
writeTIF(CFile *file)
{
  return write(file, CFILE_TYPE_IMAGE_TIF);
}

bool
CImage::
writeXBM(CFile *file)
{
  return write(file, CFILE_TYPE_IMAGE_XBM);
}

bool
CImage::
writeXPM(CFile *file)
{
  return write(file, CFILE_TYPE_IMAGE_XPM);
}

bool
CImage::
writeXWD(CFile *file)
{
  return write(file, CFILE_TYPE_IMAGE_XWD);
}

bool
CImage::
write(CFile *file, CFileType type)
{
  CImageFmt *fmt;

  if (CImageMgrInst->getFmt(type, &fmt)) {
    CImagePtr image = dup();

    bool flag = fmt->write(file, image);

    return flag;
  }

  return false;
}

//-----------

// Extras (abstract ?)

CImageAnim *
CImage::
createGIFAnim(CFile *file)
{
  return CImageGIFInst->createAnim(file);
}

bool
CImage::
readXBM(const uchar *data, int width, int height)
{
  CImagePtr image = dup();

  if (! CImageXBMInst->read(data, image, width, height))
    return false;

  replace(image);

  return true;
}

bool
CImage::
readXPM(const char **strs, uint num_strs)
{
  CImagePtr image = dup();

  if (! CImageXPMInst->read(strs, num_strs, image))
    return false;

  replace(image);

  return true;
}
