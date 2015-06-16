/*********************************************************************
 *
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2014, P.A.N.D.O.R.A. Team.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the P.A.N.D.O.R.A. Team nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors:
 *   Chatzieleftheriou Eirini <eirini.ch0@gmail.com>
 *   Kofinas Miltiadis <mkofinas@gmail.com>
 *********************************************************************/

#include "pandora_vision_msgs/ObstacleAlert.h"
#include "pandora_vision_obstacle/soft_obstacle_detection/soft_obstacle_detector.h"

namespace pandora_vision
{
  SoftObstacleDetector::SoftObstacleDetector()
  {
    cv::Mat kernelLow = (cv::Mat_<float>(2, 1) << 1 / sqrt(2), 1 / sqrt(2));
    cv::Mat kernelHigh = (cv::Mat_<float>(2, 1) << - 1 / sqrt(2), 1 / sqrt(2));

    dwtPtr_.reset(new DiscreteWaveletTransform(kernelLow, kernelHigh));
  }

  void SoftObstacleDetector::dilateImage(const MatPtr& image)
  {
    int nonZero = cv::countNonZero(*image);
    if (nonZero > image->rows * image->cols / 2)
    {
      *image = 255 - *image;
    }

    cv::Mat dilatedImage;
    // cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::dilate(*image, dilatedImage, cv::Mat());
    *image = dilatedImage;
  }

  std::vector<cv::Vec4i> SoftObstacleDetector::performProbHoughLines(const cv::Mat& image)
  {
    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(image, lines, 1, CV_PI / 180, 100, 100, 10);

    std::vector<cv::Vec4i> verticalLines;

    for (size_t ii = 0; ii < lines.size(); ii++)
    {
      cv::Vec4i line = lines[ii];
      bool awayFromBorder = (line[0] > 10 && line[0] < image.cols - 10) ||
        (line[2] > 10 && line[2] < image.cols - 10);

      float grad;
      if (line[0] == line[2])
      {
        grad = 90.0f;
      }
      else
      {
        grad = atan(static_cast<float>(line[3] - line[1]) / static_cast<float>(
            line[2] - line[0]));
        grad = static_cast<float>(fabs(grad * 180.0f / CV_PI));
      }

      if ((grad > 80.0f && grad < 100.0f) && awayFromBorder)
      {
        verticalLines.push_back(line);
      }
    }
    return verticalLines;
  }

float SoftObstacleDetector::detectROI(const std::vector<cv::Vec4i>& verticalLines,
      int frameHeight, const boost::shared_ptr<cv::Rect>& roiPtr)
  {
    float probability = 0.0f;

    if (verticalLines.size() > 2)
    {
      int minx = verticalLines[0][0];
      int maxx = verticalLines[0][0];
      int miny = verticalLines[0][1];
      int maxy = verticalLines[0][1];

      for (size_t ii = 1; ii < verticalLines.size(); ii++)
      {
        // Calculate min and max of line coordinates
        int x0 = verticalLines[ii][0];
        int x1 = verticalLines[ii][2];
        int y0 = verticalLines[ii][1];
        int y1 = verticalLines[ii][3];

        minx = x0 < minx ? x0 : minx;
        minx = x1 < minx ? x1 : minx;

        maxx = x0 > maxx ? x0 : maxx;
        maxx = x1 > maxx ? x1 : maxx;

        miny = y0 < miny ? y0 : miny;
        miny = y1 < miny ? y1 : miny;

        maxy = y0 > maxy ? y0 : maxy;
        maxy = y1 > maxy ? y1 : maxy;

        // Calculate ROI probability
        probability += static_cast<float>(abs(y1 - y0)) / static_cast<float>(frameHeight);
      }
      probability /= verticalLines.size();

      int width = maxx - minx;
      int height = maxy - miny;

      // The point inside this rect. should be the roi center, now it is the
      // upper left point in order to visualize
      cv::Rect roi(minx, miny, width, height);
      *roiPtr = roi;
    }
    return probability;
  }

  std::vector<POIPtr> SoftObstacleDetector::detectSoftObstacle(const cv::Mat& rgbImage,
      const cv::Mat& depthImage, int level)
  {
    // Perform DWT
    std::vector<MatPtr> LHImages = dwtPtr_->getLowHigh(rgbImage, level);
    MatPtr lhImage(LHImages[LHImages.size() - 1]);

    // Normalize image [0, 255]
    cv::Mat normalizedImage;
    cv::normalize(*lhImage, normalizedImage, 0, 255, cv::NORM_MINMAX);
    normalizedImage.convertTo(normalizedImage, CV_8UC1);

    // Convert image to binary with Otsu thresholding
    MatPtr otsuImage(new cv::Mat());
    cv::threshold(normalizedImage, *otsuImage, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

    // Dilate Image
    dilateImage(otsuImage);

    // Perform Hough Transform to detect lines (keep only vertical)
    std::vector<cv::Vec4i> verticalLines = performProbHoughLines(*otsuImage);

    // Detect bounding box that includes the vertical lines
    boost::shared_ptr<cv::Rect> roi(new cv::Rect());
    float probability = detectROI(verticalLines, otsuImage->rows, roi);

    std::vector<POIPtr> pois;
    if (roi->size().width > 0 && roi->size().height > 0)
    {
      ObstaclePOIPtr poi(new ObstaclePOI);

      poi->setPoint(cv::Point((roi->x + roi->width / 2) * pow(2, level),
            (roi->y + roi->height / 2) * pow(2, level)));
      poi->setWidth(roi->width * pow(2, level));
      poi->setHeight(roi->height * pow(2, level));

      poi->setProbability(probability);
      poi->setType(pandora_vision_msgs::ObstacleAlert::SOFT_OBSTACLE);

      // poi->setDepth();
      pois.push_back(poi);
    }
    return pois;
  }
}  // namespace pandora_vision
