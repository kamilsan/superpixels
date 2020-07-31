#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>

#include "centroid.hpp"
#include "image.hpp"

int main()
{
  Image image{"input.ppm"};

  const int numPixels = image.getWidth()*image.getHeight();
  const int numSuperpixels = 1000;

  const float compactnessParam = 5.0f;
  const int maxIter = -1; // Negative if there is no limit

  const int superpixelsAcrossWidth = std::sqrt(numSuperpixels);
  const int superpixelsAcrossHeight = numSuperpixels / superpixelsAcrossWidth; // NOTE: this may lead to non-squere grid
  const int numSuperpixelsAdjustes = superpixelsAcrossWidth*superpixelsAcrossHeight;

  Centroid* centroids = new Centroid[numSuperpixelsAdjustes];
  int* centroidsCount = new int[numSuperpixelsAdjustes];

  auto then = std::chrono::steady_clock::now();

  const float widthPerCentroid = (float)image.getWidth()/superpixelsAcrossWidth;
  const float heightPerCentroid = (float)image.getHeight()/superpixelsAcrossHeight;

  // Initial placing of centroids
  for(int ny = 0, n = 0; ny < superpixelsAcrossHeight; ++ny)
  {
    for(int nx = 0; nx < superpixelsAcrossWidth; ++nx)
    {
      const float x = (nx + 0.5) * widthPerCentroid;
      const float y = (ny + 0.5) * heightPerCentroid;
      
      centroids[n].x = x;
      centroids[n].y = y;

      n += 1;
    }
  }

  // TODO: move centroids out of edge

  int* toCentroidsMap = new int[numPixels];
  float* distances = new float[numPixels];

  // Assign pixels to centroids
  for(int y = 0; y < image.getHeight(); ++y)
  {
    for (int x = 0; x < image.getWidth(); ++x)
    {
      const int pixelIndex = y*image.getWidth()+x;
      const int nx = x/widthPerCentroid;
      const int ny = y/heightPerCentroid;
      const int centroidIndex = ny*superpixelsAcrossWidth+nx;
      centroidsCount[centroidIndex] += 1;
      toCentroidsMap[pixelIndex] = centroidIndex;
      centroids[centroidIndex].r += (unsigned char)image[3*pixelIndex];
      centroids[centroidIndex].g += (unsigned char)image[3*pixelIndex+1];
      centroids[centroidIndex].b += (unsigned char)image[3*pixelIndex+2];
    }
  }

  // Calculate average color for each centroid
  for(int i = 0; i < numSuperpixelsAdjustes; ++i)
  {
    centroids[i].r /= centroidsCount[i];
    centroids[i].g /= centroidsCount[i];
    centroids[i].b /= centroidsCount[i];
  }

  // Define scaling factors for distance metric
  const float spatialFactor = 1.0/
    (widthPerCentroid*widthPerCentroid+heightPerCentroid*heightPerCentroid);

  // For each pixel, calucate it's distance to assigned centroid
  for(int y = 0; y < image.getHeight(); ++y)
  {
    for (int x = 0; x < image.getWidth(); ++x)
    {
      const int pixelIndex = y*image.getWidth() + x;
      const int centeroidIndex = toCentroidsMap[pixelIndex];
      
      const float distance = centroids[centeroidIndex].calcDistance(image, x, y, spatialFactor, compactnessParam);
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
      int minX = centroids[n].x - widthPerCentroid;
      int maxX = centroids[n].x + widthPerCentroid;
      int minY = centroids[n].y - heightPerCentroid;
      int maxY = centroids[n].y + heightPerCentroid;

      if(minX < 0) minX = 0;
      else if(maxX > image.getWidth() - 1) maxX = image.getWidth() - 1;
      if(minY < 0) minY = 0;
      else if(maxY > image.getHeight() - 1) maxY = image.getHeight() - 1;

      // Recalculate distance to centroid
      for(int y = minY; y <= maxY; ++y)
      {
        for(int x = minX; x <= maxX; ++x)
        {
          const int pixelIndex = y*image.getWidth() + x;
          const float distance = centroids[n].calcDistance(image, x, y, spatialFactor, compactnessParam);

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
      centroids[i].x = 0;
      centroids[i].y = 0;
      centroids[i].r = 0;
      centroids[i].g = 0;
      centroids[i].b = 0;
      centroidsCount[i] = 0;
    }

    for(int y = 0; y < image.getHeight(); ++y)
    {
      for (int x = 0; x < image.getWidth(); ++x)
      {
        const int pixelIndex = y*image.getWidth()+x;
        const int centroidIndex = toCentroidsMap[pixelIndex];
        centroidsCount[centroidIndex] += 1;
        centroids[centroidIndex].x += x;
        centroids[centroidIndex].y += y;
        centroids[centroidIndex].r += (unsigned char)image[3*pixelIndex];
        centroids[centroidIndex].g += (unsigned char)image[3*pixelIndex+1];
        centroids[centroidIndex].b += (unsigned char)image[3*pixelIndex+2];
      }
    }

    for(int i = 0; i < numSuperpixelsAdjustes; ++i)
    {
      centroids[i].x /= centroidsCount[i];
      centroids[i].y /= centroidsCount[i];
      centroids[i].r /= centroidsCount[i];
      centroids[i].g /= centroidsCount[i];
      centroids[i].b /= centroidsCount[i];
    }
  }

  // Color pixels according to assigned centroid
  for(int y = 0; y < image.getHeight(); ++y)
  {
    for (int x = 0; x < image.getWidth(); ++x)
    {
      const int pixelIndex = y*image.getWidth()+x;
      image[3*pixelIndex] = centroids[toCentroidsMap[pixelIndex]].r;
      image[3*pixelIndex+1] = centroids[toCentroidsMap[pixelIndex]].g;
      image[3*pixelIndex+2] = centroids[toCentroidsMap[pixelIndex]].b;
    }
  }

  auto now = std::chrono::steady_clock::now();
  std::chrono::duration<double, std::milli> elapsedTime = now - then;
  std::cout << "Elapsed time: " << elapsedTime.count() << "ms\n";

  image.savePPM("output.ppm");

  delete[] centroids;
  delete[] centroidsCount;
  delete[] toCentroidsMap;
  delete[] distances;

  return 0;
}
