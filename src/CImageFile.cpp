#include "CImageLibI.h"

using std::string;
using std::cerr;
using std::endl;

CImageFile::
CImageFile(const string &fileName) :
 fileName_(fileName), image_(), loaded_(false), sized_file_list_()
{
}

CImageFile::
~CImageFile()
{
  SizedFileList::iterator p1 = sized_file_list_.begin();
  SizedFileList::iterator p2 = sized_file_list_.end  ();

  for ( ; p1 != p2; ++p1)
    delete *p1;
}

CImagePtr
CImageFile::
getImage() const
{
  if (! loaded_) {
    CImageFile *th = const_cast<CImageFile *>(this);

    th->image_ = CImageMgrInst->createImage();

    if (! th->image_->read(fileName_)) {
      cerr << "Failed to read " << fileName_ << endl;

      th->image_ = CImagePtr();
    }

    th->loaded_ = true;
  }

  return image_;
}

bool
CImageFile::
unload()
{
  if (loaded_) {
    SizedFileList::iterator p1 = sized_file_list_.begin();
    SizedFileList::iterator p2 = sized_file_list_.end  ();

    for ( ; p1 != p2; ++p1)
      (*p1)->unload();

    image_ = CImagePtr();

    loaded_ = false;
  }

  return true;
}

CImageSizedFile *
CImageFile::
lookupSizedFile(int width, int height, bool keep_aspect)
{
  SizedFileList::iterator p1 = sized_file_list_.begin();
  SizedFileList::iterator p2 = sized_file_list_.end  ();

  for ( ; p1 != p2; ++p1)
    if ((*p1)->match(width, height, keep_aspect))
      return (*p1);

  return NULL;
}

CImageSizedFile *
CImageFile::
addSizedFile(int width, int height, bool keep_aspect)
{
  CImageSizedFile *data =
    new CImageSizedFile(this, width, height, keep_aspect);

  sized_file_list_.push_back(data);

  return data;
}

bool
CImageFile::
removeSizedFile(int width, int height, bool keep_aspect)
{
  CImageSizedFile *data = lookupSizedFile(width, height, keep_aspect);

  sized_file_list_.remove(data);

  return true;
}

//--------------

CImageSizedFile::
CImageSizedFile(CImageFile *file, int width, int height, bool keep_aspect) :
 CImageFile(""), file_(file), size_(width, height), keep_aspect_(keep_aspect),
 image_(), loaded_(false)
{
}

CImageSizedFile::
~CImageSizedFile()
{
}

string
CImageSizedFile::
getFilename() const
{
  return file_->getFilename();
}

CImagePtr
CImageSizedFile::
getImage() const
{
  if (! loaded_) {
    CImageSizedFile *th = const_cast<CImageSizedFile *>(this);

    CImagePtr image = file_->getImage();

    if (image.isValid()) {
      if (size_.width  != (int) image->getWidth () ||
          size_.height != (int) image->getHeight()) {
        if (keep_aspect_)
          th->image_ = image->resizeKeepAspect(size_.width, size_.height);
        else
          th->image_ = image->resize(size_.width, size_.height);
      }
      else
        th->image_ = image;
    }
    else
      th->image_ = CImagePtr();

    th->loaded_ = true;
  }

  return image_;
}

bool
CImageSizedFile::
unload()
{
  if (loaded_) {
    image_ = CImagePtr();

    loaded_ = false;
  }

  return true;
}

bool
CImageSizedFile::
match(uint width, uint height, bool keep_aspect) const
{
  return ((int) width  == size_.width  &&
          (int) height == size_.height &&
          keep_aspect  == keep_aspect_);
}
