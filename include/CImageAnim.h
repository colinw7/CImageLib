class CImageFrame {
 private:
  CImagePtr image_;
  int       delay_;      // hundreds of a second
  int       dispose_;    // 0=unknown, 1=no, 2=background, 4=previous
  bool      user_input_; // click for next frame

 public:
  CImageFrame(CImagePtr image);
 ~CImageFrame();

  CImagePtr getImage() const { return image_; }

  void setDelay(int delay) { delay_ = delay; }
  int  getDelay() const { return delay_; }

  void setDispose(int dispose) { dispose_ = dispose; }
  int  getDispose() const { return dispose_; }

  void setUserInput(bool user_input) { user_input_ = user_input; }
  bool getUserInput() const { return user_input_; }
};

class CImageAnim {
 private:
  typedef std::vector<CImageFrame *> FrameList;

  FrameList frames_;

 public:
  typedef FrameList::iterator       iterator;
  typedef FrameList::const_iterator const_iterator;

  CImageAnim();
 ~CImageAnim();

  void add(CImageFrame *frame);

  iterator begin() { return frames_.begin(); }
  iterator end  () { return frames_.end  (); }

  int size() const { return frames_.size(); }

  CImageFrame *operator[](int pos) { return frames_[pos]; }
};
