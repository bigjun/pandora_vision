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
 * Authors: Alexandros Philotheou, Manos Tsardoulias,
 *          Christos Tsirigotis <tsirif@gmail.com>
 *********************************************************************/

#include "pandora_vision_msgs/BlobVector.h"
#include "pandora_vision_msgs/Blob.h"
#include "pandora_vision_msgs/Keypoint.h"

#include "utils/blob_vector.h"

/**
  @namespace pandora_vision
  @brief The main namespace for PANDORA vision
 **/
namespace pandora_vision
{
  BlobVector::
  BlobVector() {}
  BlobVector::
  BlobVector(const pandora_vision_msgs::BlobVector& src)
  {
    this->blobVector_ = src;
  }
  BlobVector::
  ~BlobVector() {}

  void
  BlobVector::
  extend(const BlobVector& src)
  {
    this->blobVector_.blobs.insert(this->blobVector_.blobs.end(),
        src.blobVector_.blobs.begin(), src.blobVector_.blobs.end());
  }

  void
  BlobVector::
  extend(const pandora_vision_msgs::BlobVector& src)
  {
    this->blobVector_.blobs.insert(this->blobVector_.blobs.end(),
        src.blobs.begin(), src.blobs.end());
  }

  void
  BlobVector::
  append(const pandora_vision_msgs::Blob& blob)
  {
    this->blobVector_.blobs.push_back(blob);
  }

  void
  BlobVector::
  append(const cv::Point2f& rectangleUpperLeft,
      const cv::Point2f& outlineUpperLeft,
      int rx, int ry,
      int ox, int oy)
  {
    pandora_vision_msgs::Blob blob;

    // Assign area of interest
    blob.areaOfInterest.center.x = rectangleUpperLeft.x + rx / 2;
    blob.areaOfInterest.center.y = rectangleUpperLeft.y + ry / 2;
    blob.areaOfInterest.width = rx;
    blob.areaOfInterest.height = ry;

    // Assign the outline points
    blob.outline =
      generateRectangle(outlineUpperLeft, ox, oy, 1);

    // Append hole into conveyor
    append(blob);
  }
  
  void
  BlobVector::
  append(const std::vector<cv::Point2f>& rectanglePoints,
      const std::vector<cv::Point2f>& outlinePoints)
  {
    
  }

  void
  BlobVector::
  clear()
  {
    this->blobVector_.blobs.clear();
  }

  void
  BlobVector::
  copy(const pandora_vision_msgs::BlobVector& src)
  {
    this->blobVector_ = src;
  }

  void
  BlobVector::
  copy(const BlobVector& src)
  {
    this->blobVector_ = src.blobVector_;
  }
  
  int
  BlobVector::
  getSize()
  {
    return blobVector_.blobs.size();
  }
  
  pandora_vision_msgs::Blob
  BlobVector::
  getBlob(int index)
  {
    return this->blobVector_.blobs[index];
  }

  void
  BlobVector::
  merge(const BlobVector& srcA,
      const BlobVector& srcB)
  {
    // Clear the destination conveyor if not empty
    if (this->blobVector_.blobs.size() > 0) clear();
    // Extend with the first source
    extend(srcA);
    // Extend with the second source
    extend(srcB);
  }

  void
  BlobVector::
  merge(const pandora_vision_msgs::BlobVector& srcA,
      const BlobVector& srcB)
  {
    // Clear the destination conveyor if not empty
    if (this->blobVector_.blobs.size() > 0) clear();
    // Extend with the first source
    extend(srcA);
    // Extend with the second source
    extend(srcB);
  }

  void
  BlobVector::
  merge(const BlobVector& srcA,
      const pandora_vision_msgs::BlobVector& srcB)
  {
    // Clear the destination conveyor if not empty
    if (this->blobVector_.blobs.size() > 0) clear();
    // Extend with the first source
    extend(srcA);
    // Extend with the second source
    extend(srcB);
  }

  void
  BlobVector::
  merge(const pandora_vision_msgs::BlobVector& srcA,
      const pandora_vision_msgs::BlobVector& srcB)
  {
    // Clear the destination conveyor if not empty
    if (this->blobVector_.blobs.size() > 0) clear();
    // Extend with the first source
    extend(srcA);
    // Extend with the second source
    extend(srcB);
  }

  void
  BlobVector::
  removeHole(int index)
  {
    // Delete the respective keypoint
    this->blobVector_.blobs.erase(this->blobVector_.blobs.begin() + index);
  }

  void
  BlobVector::
  replace(const pandora_vision_msgs::BlobVector& src)
  {
    // Clear the dst
    clear();
    // Fill it with the src
    copy(src);
  }

  void
  BlobVector::
  replace(const BlobVector& src)
  {
    // Clear the dst
    clear();
    // Fill it with the src
    copy(src);
  }

  void
  BlobVector::
  replaceHole(const BlobVector& src,
    int srcIndex, int dstIndex)
  {
    // Replace the dst's dstIndex-th hole's keypoint
    this->blobVector_.blobs[dstIndex] = src.blobVector_.blobs[srcIndex];
  }

  void
  BlobVector::
  replaceHole(const pandora_vision_msgs::BlobVector& src,
    int srcIndex, int dstIndex)
  {
    // Replace the dst's dstIndex-th hole's keypoint
    this->blobVector_.blobs[dstIndex] = src.blobs[srcIndex];
  }

  void
  BlobVector::
  shuffle()
  {
    // Keep the original holes' arrangement
    BlobVector temp;
    temp.copy(*this);

    // Hollow-out the src
    clear();

    // The vector of holes' indices
    std::vector<int> indices;
    for (int i = 0; i < temp.getSize(); i++)
    {
      indices.push_back(i);
    }

    // Shuffle the indices
    std::random_shuffle(indices.begin(), indices.end());

    // Fill the src conveyor with the shuffled holes
    for (int i = 0; i < indices.size(); i++)
    {
      append(temp.getBlob(indices[i]));
    }
  }

  std::vector<pandora_vision_msgs::Keypoint>
  BlobVector::
  generateRectangle(const cv::Point2f& upperLeft, int x, int y, int intent)
  {
    // The vector of the rectangle's vertices
    std::vector<pandora_vision_msgs::Keypoint> rectangleVertices;
    // The four vertices of the rectangle
    cv::Point2f vertex_1(upperLeft.x, upperLeft.y);
    cv::Point2f vertex_2(upperLeft.x, upperLeft.y + y);
    cv::Point2f vertex_3(upperLeft.x + x, upperLeft.y + y);
    cv::Point2f vertex_4(upperLeft.x + x, upperLeft.y);

    if (intent == 0) {
      // Push them back into the vector
      rectangleVertices.push_back(cvToMsg(vertex_1));
      rectangleVertices.push_back(cvToMsg(vertex_2));
      rectangleVertices.push_back(cvToMsg(vertex_3));
      rectangleVertices.push_back(cvToMsg(vertex_4));
    }
    else {
      cv::Point2f a[] = {vertex_1, vertex_2, vertex_3, vertex_4};

      cv::Mat canvas = cv::Mat::zeros(480, 640, CV_8UC1);

      for (unsigned int j = 0; j < 4; j++)
      {
        cv::line(canvas, a[j], a[(j + 1) % 4], cv::Scalar(255, 0, 0), 1, 8);
      }

      for (int i = 0; i < 480; i++)
      {
        for (int j = 0; j < 640; j++)
        {
          if (canvas.at<unsigned char>(i, j) != 0)
          {
            rectangleVertices.push_back(cvToMsg(cv::Point2f(j, i)));
          }
        }
      }
    }
    return rectangleVertices;
  }

  pandora_vision_msgs::Keypoint
  BlobVector::
  cvToMsg(const cv::Point2f& point)
  {
    pandora_vision_msgs::Keypoint msg_point;
    msg_point.x = point.x;
    msg_point.y = point.y;
    return msg_point;
  }

  cv::Point2f
  BlobVector::
  msgToCv(const pandora_vision_msgs::Keypoint& point)
  {
    cv::Point2f cv_point(point.x, point.y);
    return cv_point;
  }

}  // namespace pandora_vision
