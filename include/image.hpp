#pragma once

#include <cmath>
#include <fstream>

class Image
{
public:
  Image(const char* fileName);

  ~Image();

  bool savePPM(const char* fileName) const;

  int getWidth() const { return width_; }
  int getHeight() const { return height_; }

  char operator[](unsigned int index) const { return pixels_[index]; }
  char& operator[](unsigned int index) { return pixels_[index]; }

private:
  int width_;
  int height_;
  int len_;
  char* pixels_;
};