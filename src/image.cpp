#include "image.hpp"

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
  pixels_ = new char[len_];
  file.read(pixels_, len_);

  file.close();
}

Image::~Image()
{
  delete[] pixels_;
}

bool Image::savePPM(const char* fileName) const
{
  std::ofstream file;
  file.open(fileName, std::ios::binary);
  if(!file.is_open()) return false;

  file << "P6\n" << width_ << " " << height_ << "\n255\n";
  file.write(pixels_, len_);

  file.close();
  return true;
}