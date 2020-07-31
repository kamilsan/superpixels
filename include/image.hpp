#pragma once

#include <memory>

class Image
{
public:
  Image(const char* fileName);
  Image(const Image& other);
  Image(Image&& other);

  bool savePPM(const char* fileName) const;

  int getWidth() const { return width_; }
  int getHeight() const { return height_; }

  Image& operator=(const Image& other);
  Image& operator=(Image&& other);

  char operator[](unsigned int index) const { return pixels_[index]; }
  char& operator[](unsigned int index) { return pixels_[index]; }

private:
  int width_;
  int height_;
  int len_;
  std::unique_ptr<char[]> pixels_;
};