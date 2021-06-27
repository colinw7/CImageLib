#ifndef CIMAGE_FILE_H
#define CIMAGE_FILE_H

class CImageSizedFile;

class CImageFile {
 public:
  explicit CImageFile(const std::string &fileName);

  virtual ~CImageFile();

  virtual std::string getFilename() const { return fileName_; }

  virtual CImagePtr getImage() const;

  virtual bool unload();

  CImageSizedFile *lookupSizedFile(int width, int height,
                                   bool keep_aspect=false);

  CImageSizedFile *addSizedFile(int width, int height,
                                bool keep_aspect=false);

  bool removeSizedFile(int width, int height, bool keep_aspect=true);

 private:
  using SizedFileList = std::list<CImageSizedFile *>;

  std::string   fileName_;
  CImagePtr     image_;
  bool          loaded_ { false };
  SizedFileList sized_file_list_;
};

//---

class CImageSizedFile : public CImageFile {
 public:
  CImageSizedFile(CImageFile *file, int width, int height, bool keep_aspect=false);
 ~CImageSizedFile();

  std::string getFilename() const;

  CImagePtr getImage() const;

  bool unload();

  bool match(uint width, uint height, bool keep_aspect) const;

 private:
  CImageSizedFile(const CImageSizedFile &rhs);
  CImageSizedFile &operator=(const CImageSizedFile &rhs);

 private:
  CImageFile *file_ { nullptr };
  CISize2D    size_;
  bool        keep_aspect_ { false };
  CImagePtr   image_;
  bool        loaded_ { false };
};

#endif
