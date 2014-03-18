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

#include "hole_fusion_node/generic_filters.h"

namespace pandora_vision
{
  /**
    @brief Assimilates fragmented holes into existing whole ones
    from either source (RGB or Depth).
    @param[in][out] depthHolesConveyor [HoleFilters::HolesConveyor*]
    The candidate holes conveyor originated from the depth node
    @param[in][out] rgbHolesConveyor [HoleFilters::HolesConveyor*]
    The candidate holes conveyor originated from the rgb node
    @return void
   **/
  void GenericFilters::assimilateBilaterally(
    HoleFilters::HolesConveyor* depthHolesConveyor,
    HoleFilters::HolesConveyor* rgbHolesConveyor)
  {
    //!< Assimilating holes has a meaning only if both nodes have published
    //!< candidate holes
    if (depthHolesConveyor->keyPoints.size() > 0 &&
      rgbHolesConveyor->keyPoints.size() > 0)
    {
      assimilateUnilaterally(*rgbHolesConveyor, depthHolesConveyor);

      assimilateUnilaterally(*depthHolesConveyor, rgbHolesConveyor);
    }

  }



  /**
    @brief Assimilates the fragmented holes of @param assimilable into the
    existing whole ones of @param assimilator. It checks whether the each
    entry of the set of assimilable's bounding box vertices reside inside
    the assimilator's set of bounding boxes. If so, the latter keypoint
    etc are kept unchanged and the former one is deleted.
    @param[in][out] assimilator [const HoleFilters::HolesConveyor&]
    The candidate holes conveyor that will potentially assimilate the
    assimimable's holes
    @param[in][out] assimilable [HoleFilters::HolesConveyor*]
    The candidate holes conveyor whose holes will potentially by assimilated
    by the assimilator
    @return void
   **/
  void GenericFilters::assimilateUnilaterally(
    const HoleFilters::HolesConveyor& assimilator,
    HoleFilters::HolesConveyor* assimilable)
  {
    //!< Validate assimilable's holes against the assimilator's ones
    for (int i = 0; i < assimilator.keyPoints.size(); i++)
    {
      for (int j = assimilable->keyPoints.size() - 1; j >= 0; j--)
      {
        //!< Are all the vertices of assimilable inside the
        //!< assimilator's bounding box?
        bool allAssimilableVerticesInAssimilator = true;
        for (int av = 0; av < assimilable->rectangles[j].size(); av++)
        {
          if (cv::pointPolygonTest(assimilator.rectangles[i],
              assimilable->rectangles[j][av], false) < 0)
          {
            allAssimilableVerticesInAssimilator = false;
            break;
          }
        }

        //!< If all of assimilable's vertices reside inside the assimilator's
        //!< bounding box delete the keypoint along with its
        //!< associated conveyor entries
        if (allAssimilableVerticesInAssimilator)
        {
          assimilable->keyPoints.erase(assimilable->keyPoints.begin() + j);
          assimilable->outlines.erase(assimilable->outlines.begin() + j);
          assimilable->rectangles.erase(assimilable->rectangles.begin() + j);
        }
      }
    }
  }



  /**
    @brief Given the RGB and Depth HoleFilters::HolesConveyor* structs,
    the purpose of this function is to identify blobs that are overlapping
    each other but none of them is entirely inside the other, and merge
    them in one candidate hole: the one whose bounding rectangle has
    the greater area.
    @param[in][out] depthHolesConveyor [HoleFilters::HolesConveyor*]
    The candidate holes conveyor originated from the depth node
    @param[in][out] rgbHolesConveyor [HoleFilters::HolesConveyor*]
    The candidate holes conveyor originated from the rgb node
    @return void
   **/
  void GenericFilters::mergeBilaterally(
    HoleFilters::HolesConveyor* depthHolesConveyor,
    HoleFilters::HolesConveyor* rgbHolesConveyor)
  {
    //!< Merging holes has a meaning only if both nodes have published
    //!< candidate holes
    if (depthHolesConveyor->keyPoints.size() > 0 &&
      rgbHolesConveyor->keyPoints.size() > 0)
    {
      mergeUnilaterally(rgbHolesConveyor, depthHolesConveyor);

      mergeUnilaterally(depthHolesConveyor, rgbHolesConveyor);
    }
  }



  /**
    @brief Given two HoleFilters::HolesConveyor* structs, one with the
    potential of assimilating the other (assimilator) and the other with the
    potential of being assimilated by the other (assimilable), the purpose
    of this function is to identify blobs that are overlapping each other
    but none of them is entirely inside the other, while the assimilator's
    bounding box is greater than that of the assimilable's.
    If this is true for a given assimilator and assimilable,
    the assimilator will grow rectangle-wise and outline-wise
    by the size of the assimilator, while the assimilator's
    new keypoint will be the mean of the two keypoints. The assimilable
    then is rendered useless and deleted.
    @param[in][out] assimilator [HoleFilters::HolesConveyor*]
    The holes conveyor whose candidate holes will potentially
    assimilate candidate holes of @param assimilable
    @param[in][out] assimilable [HoleFilters::HolesConveyor*]
    The holes conveyor whose candidate holes will potentially
    will be assimilated by candidate holes of @param assimilator
    @return void
   **/
  void GenericFilters::mergeUnilaterally(
    HoleFilters::HolesConveyor* assimilator,
    HoleFilters::HolesConveyor* assimilable)
  {
    //!< Validate the assimilable's holes against the assimilator's ones
    for (int i = 0; i < assimilator->keyPoints.size(); i++)
    {
      for (int j = assimilable->keyPoints.size() - 1; j >= 0; j--)
      {
        //!< Are all the assimilable's outline points inside
        //!< the assimilator's bounding box? If not all but some, continue
        int numAssimilableOutlinePointsInAssimilator = 0;
        for (int av = 0; av < assimilable->outlines[j].size(); av++)
        {
          if (cv::pointPolygonTest(assimilator->outlines[i],
              assimilable->outlines[j][av], false) > 0)
          {
            numAssimilableOutlinePointsInAssimilator++;
          }
        }

        //!< If zero or all of assimilable's outine points
        //!< are inside the assimilator's outline, this assimilable is
        //!< not a candidate to be merged by the assimilator
        if (numAssimilableOutlinePointsInAssimilator == 0 ||
          numAssimilableOutlinePointsInAssimilator ==
          assimilable->outlines[j].size())
        {
          continue;
        }


        //!< Calculate the assimilator's bounding box area
        float assimilatorRectangleWidthX = assimilator->rectangles[i][0].x
          - assimilator->rectangles[i][1].x;
        float assimilatorRectangleWidthY = assimilator->rectangles[i][0].y
          - assimilator->rectangles[i][1].y;
        float assimilatorRectangleHeightX = assimilator->rectangles[i][1].x
          - assimilator->rectangles[i][2].x;
        float assimilatorRectangleHeightY = assimilator->rectangles[i][1].y
          - assimilator->rectangles[i][2].y;

        float assimilatorBoxArea = sqrt(pow(assimilatorRectangleWidthX, 2)
          + pow(assimilatorRectangleWidthY, 2)) *
          sqrt(pow(assimilatorRectangleHeightX, 2)
            + pow(assimilatorRectangleHeightY, 2));

        //!< Calculate the assimilable's bounding box area
        float assimilableRectangleWidthX = assimilable->rectangles[j][0].x
          - assimilable->rectangles[j][1].x;
        float assimilableRectangleWidthY = assimilable->rectangles[j][0].y
          - assimilable->rectangles[j][1].y;
        float assimilableRectangleHeightX = assimilable->rectangles[j][1].x
          - assimilable->rectangles[j][2].x;
        float assimilableRectangleHeightY = assimilable->rectangles[j][1].y
          - assimilable->rectangles[j][2].y;

        float assimilableBoxArea = sqrt(pow(assimilableRectangleWidthX, 2) +
          pow(assimilableRectangleWidthY, 2)) *
          sqrt(pow(assimilableRectangleHeightX, 2)
            + pow(assimilableRectangleHeightY, 2));


        //!< If the assimilable's area is smaller than the assimilator's,
        //!< merge the assimilator with the assimilable into the assimilator
        //!< and delete the assimilable
        if (assimilableBoxArea < assimilatorBoxArea)
        {
          //!< Viewing the two outlines as sets,
          //!< the final outline should not have the intersection
          //!< of the two sets.
          std::vector<cv::Point> assimilatorOutline;
          std::vector<cv::Point> assimilableOutline;

          //!< Add all the assimilable's outline points that are not located
          //!< inside the assimilator's outline, to the assimilableOutline
          //!< vector
          for (int o = 0; o < assimilable->outlines[j].size(); o++)
          {
            if (pointPolygonTest(assimilator->outlines[i],
                assimilable->outlines[j][o], false) <= 0)
            {
              assimilableOutline.push_back(assimilable->outlines[j][o]);
            }
          }

          //!< Add all the assimilator's outline points that are not located
          //!< inside the assimilable's outline, to the assimilatorOutline
          //!< vector
          for (int o = 0; o < assimilator->outlines[i].size(); o++)
          {
            if (pointPolygonTest(assimilable->outlines[j],
                assimilator->outlines[i][o], false) <= 0)
            {
              assimilatorOutline.push_back(assimilator->outlines[i][o]);
            }
          }

          //!< The assimilator's outline will be the sum of the outline points
          //!< that are not located inside each other
          assimilator->outlines[i] = assimilatorOutline;
          assimilator->outlines[i].insert(assimilator->outlines[i].end(),
            assimilableOutline.begin(), assimilableOutline.end());


          //!< The assimilable's new least area rotated bounding box will be the
          //!< one that encloses the new (merged) outline points
          cv::RotatedRect substituteRotatedRectangle =
            minAreaRect(assimilator->outlines[i]);

          //!< Obtain the four vertices of the new rotated rectangle
          cv::Point2f substituteVerticesArray[4];
          substituteRotatedRectangle.points(substituteVerticesArray);

          //!< Same as substituteVerticesArray array, but vector
          std::vector<cv::Point2f> substituteVerticesVector;

          //!< Store the four vertices to the substituteVerticesVector
          for(int v = 0; v < 4; v++)
          {
            substituteVerticesVector.push_back(substituteVerticesArray[v]);
          }

          //!< Replace the assimilator's vertices with the new vertices
          assimilator->rectangles[i] = substituteVerticesVector;


          //!< The overall candidate hole's keypoint
          assimilator->keyPoints[i].pt.x = (assimilator->keyPoints[i].pt.x +
            assimilable->keyPoints[j].pt.x) / 2;
          assimilator->keyPoints[i].pt.y = (assimilator->keyPoints[i].pt.y +
            assimilable->keyPoints[j].pt.y) / 2;

          //!< The assimilable has now been merged with the assimilator,
          //!< so delete it. So long, assimilable
          assimilable->keyPoints.erase(assimilable->keyPoints.begin() + j);
          assimilable->outlines.erase(assimilable->outlines.begin() + j);
          assimilable->rectangles.erase(assimilable->rectangles.begin() + j);
        }
      }
    }
  }

} // namespace pandora_vision
