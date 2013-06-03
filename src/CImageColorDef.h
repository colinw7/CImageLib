class CImageColorDef {
 public:
  static bool getRGB(const std::string &name, double *r, double *g, double *b);
  static bool getRGBI(const std::string &name, int *r, int *g, int *b);
};
