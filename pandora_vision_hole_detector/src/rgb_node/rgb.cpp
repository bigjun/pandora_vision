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
* Authors: Alexandros Filotheou, Manos Tsardoulias
*********************************************************************/

#include "rgb_node/rgb.h"

namespace vision
{
  //!< Constructor
  Rgb::Rgb(void)
  {
    ros::Duration(0.5).sleep();

    //!< Subscribe to the RGB image published by the
    //!< rgb_depth_synchronizer node
    rgbImageSubscriber_= nodeHandle_.subscribe(
      "/synchronized/camera/rgb/image_raw", 1,
      &Rgb::rgbImageCallback, this);


    //!< Advertise the candidate holes found by the depth node
    rgbCandidateHolesPublisher_ = nodeHandle_.advertise
      <vision_communications::RgbCandidateHolesVectorMsg>(
      "/synchronized/camera/rgb/candidate_holes", 1000);

    ROS_INFO("RGB node initiated");
  }


  //!< Destructor
  Rgb::~Rgb(void)
  {
    ROS_INFO("RGB node terminated");
  }

  /**
    @brief Callback for the rgbImage acquired through the
    rgb_depth_synchronizer node
    @param[in] rgbImage [const sensor_msgs::Image&] The RGB image
    @return void
   **/
  void Rgb::rgbImageCallback(const sensor_msgs::Image& inImage)
  {
    #ifdef DEBUG_SHOW
    ROS_INFO("RGB node callback");
    #endif

    HoleFilters::HolesConveyor rgbHolesConveyor;
    std::vector<cv::Point2f> rectangles;
    std::vector<cv::Point> outlines;

    for(int i = 0; i < 1; i++)
    {
      cv::KeyPoint keypoint(i,i, 1);
      rgbHolesConveyor.keyPoints.push_back(keypoint);

      for (int v = 0; v < 4; v++)
      {
        float k = (float)v;
        cv::Point vertex(k,k);
        rectangles.push_back(vertex);
      }
      rgbHolesConveyor.rectangles.push_back(rectangles);

      for (int o = 0; o < 10; o++)
      {
        cv::Point2f outline(o,o);
        outlines.push_back(outline);
      }
      rgbHolesConveyor.outlines.push_back(outlines);
    }

    vision_communications::RgbCandidateHolesVectorMsg rgbCandidateHolesMsg;
    createCandidateHolesMessage(rgbHolesConveyor,
      inImage, rgbCandidateHolesMsg, sensor_msgs::image_encodings::TYPE_32FC1);

    rgbCandidateHolesPublisher_.publish(rgbCandidateHolesMsg);
  }




  void Rgb::createCandidateHolesMessage(
    const HoleFilters::HolesConveyor& conveyor,
    const sensor_msgs::Image& rgbImage,
    vision_communications::RgbCandidateHolesVectorMsg& rgbCandidateHolesMsg,
    const std::string& encoding)
  {
    //!< Fill the vision_communications::RgbCandidateHolesVectorMsg's
    //!< candidateHoles vector
    for (unsigned int i = 0; i < conveyor.keyPoints.size(); i++)
    {
      vision_communications::CandidateHoleMsg holeMsg;

      //!< Push back the keypoint
      holeMsg.keypointX = conveyor.keyPoints[i].pt.x;
      holeMsg.keypointY = conveyor.keyPoints[i].pt.y;

      //!< Push back the bounding rectangle's vertices
      for (int v = 0; v < conveyor.rectangles[i].size(); v++)
      {
        holeMsg.verticesX.push_back(conveyor.rectangles[i][v].x);
        holeMsg.verticesY.push_back(conveyor.rectangles[i][v].y);
      }

      //!< Push back the blob's outline points
      for (int o = 0; o < conveyor.outlines[i].size(); o++)
      {
        holeMsg.outlineX.push_back(conveyor.outlines[i][o].x);
        holeMsg.outlineY.push_back(conveyor.outlines[i][o].y);
      }

      //!< Push back one hole to the holes vector message
      rgbCandidateHolesMsg.candidateHoles.push_back(holeMsg);
    }

    //!< Fill vision_communications::DepthCandidateHolesVectorMsg's
    //!< sensor_msgs/Image interpolatedDepthImage
    rgbCandidateHolesMsg.rgbImage = rgbImage;
  }
}