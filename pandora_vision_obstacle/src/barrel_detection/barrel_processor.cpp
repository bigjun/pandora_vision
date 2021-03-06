/*********************************************************************
 *
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2015, P.A.N.D.O.R.A. Team.
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
 *   Bosdelekidis Vasilis <vasilis1bos@gmail.com>
 *   Chatzieleftheriou Eirini <eirini.ch0@gmail.com>
 *********************************************************************/

#include <string>
#include <vector>
#include <limits>
#include <algorithm>
#include <utility>
#include "pandora_vision_obstacle/barrel_detection/barrel_processor.h"

namespace pandora_vision
{
namespace pandora_vision_obstacle
{
  void BarrelProcessor::initialize(const std::string& ns, sensor_processor::Handler* handler)
  {
    sensor_processor::Processor<ImagesStamped, POIsStamped>::initialize(ns, handler);

    ros::NodeHandle processor_nh = this->getProcessorNodeHandle();

    detector_.reset( new BarrelDetector(this->getName(), processor_nh) );

    BarrelParametersHandler_.reset( new BarrelParametersHandler(this->getName(), processor_nh) );
  }

  BarrelProcessor::BarrelProcessor() :
    sensor_processor::Processor<ImagesStamped, POIsStamped>()
  {}

  bool BarrelProcessor::process(const ImagesStampedConstPtr& input,
      const POIsStampedPtr& output)
  { // ROS_INFO("Starting bag read");
    output->header = input->getHeader();
    output->frameWidth = input->getRgbImage().cols;
    output->frameHeight = input->getRgbImage().rows;

    output->pois = detector_->detectBarrel(
        input->getRgbImage(), input->getDepthImage());

    if (output->pois.empty())
    {
      return false;
    }
    return true;
  }
}  // namespace pandora_vision_obstacle
}  // namespace pandora_vision
