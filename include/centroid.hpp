#pragma once

#include "image.hpp" 

struct Centroid
{
  Centroid(): x(0), y(0), r(0), g(0), b(0) {}

  float x;
  float y;

  float r;
  float g;
  float b;

  inline float calcDistance(const Image& image, int x, int y, float spatialFactor, float compactness);
};

float Centroid::calcDistance(const Image& image, int x, int y, float spatialFactor, float compactness)
{
  const static float colorFactor = 1.0 / (3*255*255);

  const int pixelIndex = y*image.getWidth()+x;
  const int pixelR = (unsigned char)image[3*pixelIndex];
  const int pixelG = (unsigned char)image[3*pixelIndex+1];
  const int pixelB = (unsigned char)image[3*pixelIndex+2];

  const float deltaCr = pixelR - this->r;
  const float deltaCg = pixelG - this->g;
  const float deltaCb = pixelB - this->b;
  float deltaC = deltaCr*deltaCr + deltaCg*deltaCg + deltaCb*deltaCb;
  deltaC *= colorFactor;

  const float deltaDx = x - this->x;
  const float deltaDy = y - this->y;
  float deltaD = deltaDx*deltaDx + deltaDy*deltaDy;
  deltaD *= spatialFactor;

  return compactness*deltaC + deltaD;
}