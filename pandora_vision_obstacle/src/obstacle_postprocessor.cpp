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
 *   Chatzieleftheriou Eirini <eirini.ch0@gmail.com>
 *********************************************************************/

#include <string>
#include "pandora_common_msgs/GeneralAlertVector.h"
#include "pandora_vision_obstacle/obstacle_poi.h"
#include "pandora_vision_obstacle/obstacle_postprocessor.h"

namespace pandora_vision
{
namespace pandora_vision_obstacle
{
  ObstaclePostProcessor::ObstaclePostProcessor() :
    VisionPostProcessor<pandora_vision_msgs::ObstacleAlertVector>()
  {
  }

  bool ObstaclePostProcessor::postProcess(const POIsStampedConstPtr& input,
    const ObstacleAlertVectorPtr& output)
  {
    pandora_common_msgs::GeneralAlertVector alertVector = getGeneralAlertInfo(input);
    output->header = alertVector.header;

    pandora_vision_msgs::ObstacleAlert obstacleAlert;

    for (int ii = 0; ii < alertVector.alerts.size(); ii++)
    {
      obstacleAlert.pointsYaw[ii] = alertVector.alerts[ii].yaw;
      obstacleAlert.pointsPitch[ii] = alertVector.alerts[ii].pitch;
      obstacleAlert.probability = alertVector.alerts[ii].probability;

      boost::shared_ptr<ObstaclePOI> obstaclePOI(boost::dynamic_pointer_cast<ObstaclePOI>(
        input->pois[ii]));
      obstacleAlert.type = obstaclePOI->getType();
      obstacleAlert.pointsDepth[ii] = obstaclePOI->getDepth();
    }
    output->alerts.push_back(obstacleAlert);

    if (output->alerts.empty())
    {
      return false;
    }
    return true;
  }
}  // namespace pandora_vision_obstacle
}  // namespace pandora_vision
