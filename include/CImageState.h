class CImageState {
 private:
  static bool debug_;

 public:
  static bool getDebug() { return debug_; }
  static void setDebug(bool debug) { debug_ = debug; }
};
