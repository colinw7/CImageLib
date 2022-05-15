#include <CImageLibI.h>

CImageAnim::
CImageAnim() :
 frames_()
{
}

CImageAnim::
~CImageAnim()
{
  auto n = frames_.size();

  for (size_t i = 0; i < n; ++i)
    delete frames_[n - i - 1];
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
