#include <iostream>
#include <fstream>
#include <random>
#include <chrono>

bool saveImage(const char* fileName, int width, int height, const char* pixels)
{
  std::ofstream file;
  file.open(fileName, std::ios::binary);
  if(!file.is_open()) return false;

  file << "P6\n" << width << "\n" << height << "\n255\n";
  int len = 3*width*height;
  file.write(pixels, len);

  file.close();
  return true;
}

bool loadImage(const char* fileName, int& width, int& height, char*& pixels)
{
  std::ifstream file;
  file.open(fileName, std::ios::binary);
  if(!file.is_open()) return false;

  char* h = new char[3];
  file.read(h, 3);
  if(h[0] != 'P' || h[1] != '6') { delete[] h; return false; }
  delete[] h;

  while(file.get() == '#')
  {
    while(file.get() != '\n');
  }
  file.unget();

  int max;
  file >> width >> height >> max;
  file.get();

  int len = 3*width*height;
  pixels = new char[len];
  file.read(pixels, len);

  file.close();
  return true;
}

int main()
{
  int width, height;
  char *pixels = nullptr;
  loadImage("input.ppm", width, height, pixels);
  int numPixels = width*height;
  int numSuperpixels = 1000;
  char* newPixels = new char[3*numPixels];

  float m = 5.0f;
  int maxIter = -1;//negative num if there is no limit

  int superpixelsWidth = std::sqrt(numSuperpixels);
  int superpixelsHeight = numSuperpixels / superpixelsWidth; //NOTE: this may lead to non-squere grid
  int numSuperpixelsAdjustes = superpixelsWidth*superpixelsHeight;

  float* centroidsX = new float[numSuperpixelsAdjustes];
  float* centroidsY = new float[numSuperpixelsAdjustes];
  float* centroidsR = new float[numSuperpixelsAdjustes];
  float* centroidsG = new float[numSuperpixelsAdjustes];
  float* centroidsB = new float[numSuperpixelsAdjustes];
  int* centroidsCount = new int[numSuperpixelsAdjustes];

  for(int i = 0; i < numSuperpixelsAdjustes; ++i)
  {
    centroidsR[i] = 0;
    centroidsG[i] = 0;
    centroidsB[i] = 0;
    centroidsCount[i] = 0;
  }

  float widthPerCentroid = (float)width/superpixelsWidth;
  float heightPerCentroid = (float)height/superpixelsHeight;
  float deltaDscale = 1.0/(widthPerCentroid*widthPerCentroid+heightPerCentroid*heightPerCentroid);
  float deltaCscale = 1.0/(3*255*255);

  for(int ny = 0, n = 0; ny < superpixelsHeight; ++ny)
  {
    for(int nx = 0; nx < superpixelsWidth; ++nx)
    {
      float x = (nx + 0.5) * widthPerCentroid;
      float y = (ny + 0.5) * heightPerCentroid;
      centroidsX[n] = x;
      centroidsY[n] = y;
      n+=1;
    }
  }

  //TODO: move centroids out of edge

  int* toCentroidsMap = new int[numPixels];
  float* distances = new float[numPixels];

  for(int y = 0; y < height; ++y)
  {
    for (int x = 0; x < width; ++x)
    {
      int pixelIndex = y*width+x;
      int nx = x/widthPerCentroid;
      int ny = y/heightPerCentroid;
      int centroidIndex = ny*superpixelsWidth+nx;
      centroidsCount[centroidIndex] += 1;
      toCentroidsMap[pixelIndex] = centroidIndex;
      centroidsR[centroidIndex] += (unsigned char)pixels[3*pixelIndex];
      centroidsG[centroidIndex] += (unsigned char)pixels[3*pixelIndex+1];
      centroidsB[centroidIndex] += (unsigned char)pixels[3*pixelIndex+2];
    }
  }
  for(int i = 0; i < numSuperpixelsAdjustes; ++i)
  {
    centroidsR[i] /= centroidsCount[i];
    centroidsG[i] /= centroidsCount[i];
    centroidsB[i] /= centroidsCount[i];
  }

  for(int y = 0; y < height; ++y)
  {
    for (int x = 0; x < width; ++x)
    {
      int pixelIndex = y*width+x;
      int pixelR = (unsigned char)pixels[3*pixelIndex];
      int pixelG = (unsigned char)pixels[3*pixelIndex+1];
      int pixelB = (unsigned char)pixels[3*pixelIndex+2];

      int centeroidIndex = toCentroidsMap[pixelIndex];

      int centroidR = centroidsR[centeroidIndex];
      int centroidG = centroidsG[centeroidIndex];
      int centroidB = centroidsB[centeroidIndex];

      float deltaCr = pixelR - centroidR;
      float deltaCg = pixelG - centroidG;
      float deltaCb = pixelB - centroidB;
      float deltaC = deltaCr*deltaCr + deltaCg*deltaCg + deltaCb*deltaCb;
      deltaC *= deltaCscale;

      float deltaDx = (x-centroidsX[centeroidIndex]);
      float deltaDy = (y-centroidsY[centeroidIndex]);
      float deltaD = deltaDx*deltaDx + deltaDy*deltaDy;
      deltaD *= deltaDscale;

      float distance = m*deltaC + deltaD;
      distances[pixelIndex] = distance;
    }
  }

  bool somethingChanged = true;
  int j = 0;
  while(somethingChanged)
  {
    somethingChanged = false;
    j++;
    if(maxIter > 0 && j > maxIter) break;
    for(int n = 0; n < numSuperpixelsAdjustes; ++n)
    {
      int minX = centroidsX[n] - widthPerCentroid;
      int maxX = centroidsX[n] + widthPerCentroid;
      int minY = centroidsY[n] - heightPerCentroid;
      int maxY = centroidsY[n] + heightPerCentroid;
      if(minX < 0) minX = 0;
      else if(maxX > width - 1) maxX = width - 1;
      if(minY < 0) minY = 0;
      else if(maxY > height - 1) maxY = height - 1;

      for(int y = minY; y <= maxY; ++y)
      {
        for(int x = minX; x <= maxX; ++x)
        {
          int pixelIndex = y*width+x;

          int pixelR = (unsigned char)pixels[3*pixelIndex];
          int pixelG = (unsigned char)pixels[3*pixelIndex+1];
          int pixelB = (unsigned char)pixels[3*pixelIndex+2];

          int centroidR = centroidsR[n];
          int centroidG = centroidsG[n];
          int centroidB = centroidsB[n];

          float deltaCr = pixelR - centroidR;
          float deltaCg = pixelG - centroidG;
          float deltaCb = pixelB - centroidB;
          float deltaC = deltaCr*deltaCr + deltaCg*deltaCg + deltaCb*deltaCb;
          deltaC *= deltaCscale;

          float deltaDx = (x-centroidsX[n]);
          float deltaDy = (y-centroidsY[n]);
          float deltaD = deltaDx*deltaDx + deltaDy*deltaDy;
          deltaD *= deltaDscale;

          float distance = m*deltaC + deltaD;

          if(distance < distances[pixelIndex])
          {
            somethingChanged = true;
            toCentroidsMap[pixelIndex] = n;
            distances[pixelIndex] = distance;
          }
        }
      }
    }

    for(int i = 0; i < numSuperpixelsAdjustes; ++i)
    {
      centroidsX[i] = 0;
      centroidsY[i] = 0;
      centroidsR[i] = 0;
      centroidsG[i] = 0;
      centroidsB[i] = 0;
      centroidsCount[i] = 0;
    }
    for(int y = 0; y < height; ++y)
    {
      for (int x = 0; x < width; ++x)
      {
        int pixelIndex = y*width+x;
        int centroidIndex = toCentroidsMap[pixelIndex];
        centroidsCount[centroidIndex] += 1;
        centroidsX[centroidIndex] += x;
        centroidsY[centroidIndex] += y;
        centroidsR[centroidIndex] += (unsigned char)pixels[3*pixelIndex];
        centroidsG[centroidIndex] += (unsigned char)pixels[3*pixelIndex+1];
        centroidsB[centroidIndex] += (unsigned char)pixels[3*pixelIndex+2];
      }
    }
    for(int i = 0; i < numSuperpixelsAdjustes; ++i)
    {
      centroidsX[i] /= centroidsCount[i];
      centroidsY[i] /= centroidsCount[i];
      centroidsR[i] /= centroidsCount[i];
      centroidsG[i] /= centroidsCount[i];
      centroidsB[i] /= centroidsCount[i];
    }
  }

  for(int y = 0; y < height; ++y)
  {
    for (int x = 0; x < width; ++x)
    {
      int pixelIndex = y*width+x;
      newPixels[3*pixelIndex] = centroidsR[toCentroidsMap[pixelIndex]];
      newPixels[3*pixelIndex+1] = centroidsG[toCentroidsMap[pixelIndex]];
      newPixels[3*pixelIndex+2] = centroidsB[toCentroidsMap[pixelIndex]];
    }
  }
  saveImage("output.ppm", width, height, newPixels);

  delete[] centroidsX;
  delete[] centroidsY;
  delete[] centroidsR;
  delete[] centroidsG;
  delete[] centroidsB;
  delete[] centroidsCount;
  delete[] toCentroidsMap;
  delete[] distances;
  delete[] newPixels;
  delete[] pixels;

  return 0;
}
