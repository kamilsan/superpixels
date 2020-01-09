#include <iostream>
#include <fstream>
#include <cmath>
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
  const int numPixels = width*height;
  const int numSuperpixels = 1000;
  char* newPixels = new char[3*numPixels];

  const float compactnessParam = 5.0f;
  const int maxIter = -1; // Negative if there is no limit

  const int superpixelsAcrossWidth = std::sqrt(numSuperpixels);
  const int superpixelsAcrossHeight = numSuperpixels / superpixelsAcrossWidth; // NOTE: this may lead to non-squere grid
  const int numSuperpixelsAdjustes = superpixelsAcrossWidth*superpixelsAcrossHeight;

  float* centroidsX = new float[numSuperpixelsAdjustes];
  float* centroidsY = new float[numSuperpixelsAdjustes];
  float* centroidsR = new float[numSuperpixelsAdjustes];
  float* centroidsG = new float[numSuperpixelsAdjustes];
  float* centroidsB = new float[numSuperpixelsAdjustes];
  unsigned int* centroidsCount = new unsigned int[numSuperpixelsAdjustes];

  auto then = std::chrono::steady_clock::now();

  for(int i = 0; i < numSuperpixelsAdjustes; ++i)
  {
    centroidsR[i] = 0;
    centroidsG[i] = 0;
    centroidsB[i] = 0;
    centroidsCount[i] = 0;
  }

  const float widthPerCentroid = (float)width/superpixelsAcrossWidth;
  const float heightPerCentroid = (float)height/superpixelsAcrossHeight;

  // Initial placing of centroids
  for(int ny = 0, n = 0; ny < superpixelsAcrossHeight; ++ny)
  {
    for(int nx = 0; nx < superpixelsAcrossWidth; ++nx)
    {
      const float x = (nx + 0.5) * widthPerCentroid;
      const float y = (ny + 0.5) * heightPerCentroid;
      centroidsX[n] = x;
      centroidsY[n] = y;
      n += 1;
    }
  }

  // TODO: move centroids out of edge

  int* toCentroidsMap = new int[numPixels];
  float* distances = new float[numPixels];

  // Assign pixels to centroids
  for(int y = 0; y < height; ++y)
  {
    for (int x = 0; x < width; ++x)
    {
      const int pixelIndex = y*width+x;
      const int nx = x/widthPerCentroid;
      const int ny = y/heightPerCentroid;
      const int centroidIndex = ny*superpixelsAcrossWidth+nx;
      
      centroidsCount[centroidIndex] += 1;
      toCentroidsMap[pixelIndex] = centroidIndex;
      
      centroidsR[centroidIndex] += (unsigned char)pixels[3*pixelIndex];
      centroidsG[centroidIndex] += (unsigned char)pixels[3*pixelIndex+1];
      centroidsB[centroidIndex] += (unsigned char)pixels[3*pixelIndex+2];
    }
  }

  // Calculate average color for each centroid
  for(int i = 0; i < numSuperpixelsAdjustes; ++i)
  {
    centroidsR[i] /= centroidsCount[i];
    centroidsG[i] /= centroidsCount[i];
    centroidsB[i] /= centroidsCount[i];
  }

  // Define scaling factors for distance metric
  const float spatialFactor = 1.0/
    (widthPerCentroid*widthPerCentroid+heightPerCentroid*heightPerCentroid);
  const float colorFactor = 1.0/(3*255*255);

  // For each pixel, calucate it's distance to assigned centroid
  for(int y = 0; y < height; ++y)
  {
    for (int x = 0; x < width; ++x)
    {
      const int pixelIndex = y*width+x;
      const int pixelR = (unsigned char)pixels[3*pixelIndex];
      const int pixelG = (unsigned char)pixels[3*pixelIndex+1];
      const int pixelB = (unsigned char)pixels[3*pixelIndex+2];

      const int centeroidIndex = toCentroidsMap[pixelIndex];

      const int centroidR = centroidsR[centeroidIndex];
      const int centroidG = centroidsG[centeroidIndex];
      const int centroidB = centroidsB[centeroidIndex];

      const float deltaCr = pixelR - centroidR;
      const float deltaCg = pixelG - centroidG;
      const float deltaCb = pixelB - centroidB;
      float deltaC = deltaCr*deltaCr + deltaCg*deltaCg + deltaCb*deltaCb;
      deltaC *= colorFactor;

      const float deltaDx = x - centroidsX[centeroidIndex];
      const float deltaDy = y - centroidsY[centeroidIndex];
      float deltaD = deltaDx*deltaDx + deltaDy*deltaDy;
      deltaD *= spatialFactor;

      const float distance = compactnessParam*deltaC + deltaD;
      distances[pixelIndex] = distance;
    }
  }

  // Repeat the algorithm to converge to final result
  bool somethingHasChanged = true;
  int j = 0;
  while(somethingHasChanged && (maxIter < 0 || j < maxIter))
  {
    somethingHasChanged = false;
    j++;
    for(int n = 0; n < numSuperpixelsAdjustes; ++n)
    {
      // Calculate region of interest bounds
      int minX = centroidsX[n] - widthPerCentroid;
      int maxX = centroidsX[n] + widthPerCentroid;
      int minY = centroidsY[n] - heightPerCentroid;
      int maxY = centroidsY[n] + heightPerCentroid;

      if(minX < 0) minX = 0;
      else if(maxX > (int)width - 1) maxX = width - 1;
      if(minY < 0) minY = 0;
      else if(maxY > (int)height - 1) maxY = height - 1;

      // Recalculate distance to centroid
      for(int y = minY; y <= maxY; ++y)
      {
        for(int x = minX; x <= maxX; ++x)
        {
          const int pixelIndex = y*width+x;

          const int pixelR = (unsigned char)pixels[3*pixelIndex];
          const int pixelG = (unsigned char)pixels[3*pixelIndex+1];
          const int pixelB = (unsigned char)pixels[3*pixelIndex+2];

          const int centroidR = centroidsR[n];
          const int centroidG = centroidsG[n];
          const int centroidB = centroidsB[n];

          const float deltaCr = pixelR - centroidR;
          const float deltaCg = pixelG - centroidG;
          const float deltaCb = pixelB - centroidB;
          float deltaC = deltaCr*deltaCr + deltaCg*deltaCg + deltaCb*deltaCb;
          deltaC *= colorFactor;

          const float deltaDx = x - centroidsX[n];
          const float deltaDy = y - centroidsY[n];
          float deltaD = deltaDx*deltaDx + deltaDy*deltaDy;
          deltaD *= spatialFactor;

          float distance = compactnessParam*deltaC + deltaD;

          // If pixel is closer to the new centroid, assign it to the new one
          if(distance < distances[pixelIndex])
          {
            somethingHasChanged = true;
            toCentroidsMap[pixelIndex] = n;
            distances[pixelIndex] = distance;
          }
        }
      }
    }

    // Recalculate centroid parameters
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
        const int pixelIndex = y*width+x;
        const int centroidIndex = toCentroidsMap[pixelIndex];
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

  // Color pixels according to assigned centroid
  for(int y = 0; y < height; ++y)
  {
    for (int x = 0; x < width; ++x)
    {
      const int pixelIndex = y*width+x;
      newPixels[3*pixelIndex] = centroidsR[toCentroidsMap[pixelIndex]];
      newPixels[3*pixelIndex+1] = centroidsG[toCentroidsMap[pixelIndex]];
      newPixels[3*pixelIndex+2] = centroidsB[toCentroidsMap[pixelIndex]];
    }
  }

  auto now = std::chrono::steady_clock::now();
  std::chrono::duration<double, std::milli> elapsedTime = now - then;
  std::cout << "Elapsed time: " << elapsedTime.count() << "ms\n";

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
