/******************************************************************************
 * Copyright 2018 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/
#ifndef MODULES_PERCEPTION_ONBOARD_COMPONENT_SEGMENTATION_COMPONENT_H_
#define MODULES_PERCEPTION_ONBOARD_COMPONENT_SEGMENTATION_COMPONENT_H_

#include <memory>

#include "cybertron/cybertron.h"

#include "modules/drivers/proto/pointcloud.pb.h"  // NOLINT
#include "modules/perception/lidar/app/lidar_obstacle_segmentation.h"
#include "modules/perception/lidar/common/lidar_frame.h"
#include "modules/perception/onboard/component/lidar_inner_component_messages.h"
#include "modules/perception/onboard/transform_wrapper/transform_wrapper.h"

namespace apollo {
namespace perception {
namespace onboard {

class SegmentationComponent : public cybertron::Component<drivers::PointCloud> {
 public:
  SegmentationComponent() : segmentor_(nullptr) {}

  ~SegmentationComponent() = default;

  bool Init() override;
  bool Proc(const std::shared_ptr<drivers::PointCloud>& message) override;

 private:
  bool InitAlgorithmPlugin();
  bool InternalProc(
      const std::shared_ptr<const drivers::PointCloud>& in_message,
      const std::shared_ptr<LidarFrameMessage>& out_message);

 private:
  static std::mutex s_mutex_;
  static uint32_t s_seq_num_;
  TransformWrapper velodyne2world_trans_;
  std::unique_ptr<lidar::LidarObstacleSegmentation> segmentor_;
  std::shared_ptr<apollo::cybertron::Writer<LidarFrameMessage>> writer_;
};

CYBERTRON_REGISTER_COMPONENT(SegmentationComponent);

}  // namespace onboard
}  // namespace perception
}  // namespace apollo

#endif  // MODULES_PERCEPTION_ONBOARD_COMPONENT_SEGMENTATION_COMPONENT_H_
