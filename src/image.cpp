#include "image.hpp"

#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Image::Image(const char* filename)
{
  int components = 0;
  unsigned char* data = stbi_load(filename, &width_, &height_, &components, 0);

  if(!data)
  {
    throw std::runtime_error("Could not load provided image file!\n");
  }

  len_ = 3*width_*height_;
  pixels_ = std::make_unique<unsigned char[]>(len_);

  memcpy(pixels_.get(), data, len_ * sizeof(char));

  stbi_image_free(data);
}

Image::Image(const Image& other)
{
  width_ = other.width_;
  height_ = other.height_;
  len_ = other.len_;

  pixels_ = std::make_unique<unsigned char[]>(len_);
  memcpy(pixels_.get(), other.pixels_.get(), len_ * sizeof(unsigned char));
}

Image::Image(Image&& other)
{
  width_ = other.width_;
  height_ = other.height_;
  len_ = other.len_;
  pixels_ = std::move(other.pixels_);
}

bool Image::savePPM(const char* fileName) const
{
  std::ofstream file;
  file.open(fileName, std::ios::binary);
  if(!file.is_open()) return false;

  file << "P6\n" << width_ << " " << height_ << "\n255\n";
  file.write(reinterpret_cast<char*>(pixels_.get()), len_);

  file.close();
  return true;
}

Image& Image::operator=(const Image& other)
{
  if(&other != this)
  {
    width_ = other.width_;
    height_ = other.height_;
    len_ = other.len_;

    pixels_ = std::make_unique<unsigned char[]>(len_);
    memcpy(pixels_.get(), other.pixels_.get(), len_ * sizeof(unsigned char));
  }

  return *this;
}

Image& Image::operator=(Image&& other)
{
  if(&other != this)
  {
    width_ = other.width_;
    height_ = other.height_;
    len_ = other.len_;
    pixels_ = std::move(other.pixels_);
  }

  return *this;
}