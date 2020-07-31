#include "image.hpp"

#include <fstream>

Image::Image(const char* fileName)
{
  std::ifstream file;
  file.open(fileName, std::ios::binary);
  if(!file.is_open())
    throw std::runtime_error("Failed to read provided image file!");

  char h[3];
  file.read(h, 3);
  if(h[0] != 'P' || h[1] != '6') 
    throw std::runtime_error("Invalid image format!");;

  while(file.get() == '#')
  {
    while(file.get() != '\n');
  }
  file.unget();

  int max;
  file >> width_ >> height_ >> max;
  file.get();

  len_ = 3*width_*height_;
  pixels_ = std::make_unique<char[]>(len_);
  file.read(pixels_.get(), len_);

  file.close();
}

Image::Image(const Image& other)
{
  width_ = other.width_;
  height_ = other.height_;

  int len = 3*width_*height_;
  pixels_ = std::make_unique<char[]>(len);

  for(int i = 0; i < len; ++i)
  {
    pixels_[i] = other.pixels_[i];
  }
}

Image::Image(Image&& other)
{
  width_ = other.width_;
  height_ = other.height_;
  pixels_ = std::move(other.pixels_);
}

bool Image::savePPM(const char* fileName) const
{
  std::ofstream file;
  file.open(fileName, std::ios::binary);
  if(!file.is_open()) return false;

  file << "P6\n" << width_ << " " << height_ << "\n255\n";
  file.write(pixels_.get(), len_);

  file.close();
  return true;
}

Image& Image::operator=(const Image& other)
{
  if(&other != this)
  {
    width_ = other.width_;
    height_ = other.height_;

    int len = 3*width_*height_;
    pixels_ = std::make_unique<char[]>(len);

    for(int i = 0; i < len; ++i)
    {
      pixels_[i] = other.pixels_[i];
    }
  }

  return *this;
}

Image& Image::operator=(Image&& other)
{
  if(&other != this)
  {
    width_ = other.width_;
    height_ = other.height_;
    pixels_ = std::move(other.pixels_);
  }

  return *this;
}