#include <CImageLibI.h>

CImageAnim::
CImageAnim() :
 frames_()
{
}

CImageAnim::
~CImageAnim()
{
  for (int i = frames_.size() - 1; i >= 0; --i)
    delete frames_[i];
}

void
CImageAnim::
add(CImageFrame *frame)
{
  frames_.push_back(frame);
}

//-------

CImageFrame::
CImageFrame(CImagePtr image) :
 image_(image), delay_(0), dispose_(0), user_input_(false)
{
}

CImageFrame::
~CImageFrame()
{
}
