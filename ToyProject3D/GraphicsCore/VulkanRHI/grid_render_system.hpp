#pragma once

#include "lve_camera.hpp"
#include "lve_device.hpp"
#include "lve_game_object.hpp"
#include "lve_frame_info.hpp"
#include "lve_pipeline.hpp"

// std
#include <memory>
#include <vector>

namespace lve {
class GridRenderSystem {
 public:
  GridRenderSystem(LveDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetlayout);
  ~GridRenderSystem();

  GridRenderSystem(const GridRenderSystem &) = delete;
  GridRenderSystem &operator=(const GridRenderSystem &) = delete;

  void renderGrid(
	  FrameInfo& frameInfo,
	  LveGameObject& gridObject);

 private:
  void createPipelineLayout(VkDescriptorSetLayout globalSetlayout);
  void createPipeline(VkRenderPass renderPass);

  LveDevice &lveDevice;

  std::unique_ptr<LvePipeline> lvePipeline;
  VkPipelineLayout pipelineLayout;
};
}  // namespace lve
