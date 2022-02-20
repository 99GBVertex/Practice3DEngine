#include "grid_render_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <stdexcept>

namespace lve {

struct GridPushConstants {
	glm::mat4 projectionMatrix{ 1.f };
	glm::mat4 modelMatrix{ 1.f };
};

GridRenderSystem::GridRenderSystem(
	LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetlayout)
    : lveDevice{device} {
  createPipelineLayout(globalSetlayout);
  createPipeline(renderPass);
}

GridRenderSystem::~GridRenderSystem() {
  vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
}

void GridRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetlayout) {
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(GridPushConstants);

  std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetlayout };

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
  pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
  if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }
}

void GridRenderSystem::createPipeline(VkRenderPass renderPass) {
  assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

  PipelineConfigInfo pipelineConfig{};
  LvePipeline::gridPipelineConfigInfo(pipelineConfig);
  pipelineConfig.renderPass = renderPass;
  pipelineConfig.pipelineLayout = pipelineLayout;
  lvePipeline = std::make_unique<LvePipeline>(
      lveDevice,
      "ToyProject3D/Shaders/grid_shader_hlsl.vert.spv",
      "ToyProject3D/Shaders/grid_shader_hlsl.frag.spv",
      pipelineConfig);
}

void GridRenderSystem::renderGrid(
	FrameInfo& frameInfo,
	LveGameObject& gridObject)
{
  lvePipeline->bind(frameInfo.commandBuffer);

  vkCmdBindDescriptorSets(
	  frameInfo.commandBuffer,
	  VK_PIPELINE_BIND_POINT_GRAPHICS,
	  pipelineLayout,
	  0,
	  1,
	  &frameInfo.globalDescriptorSet,
	  0,
	  nullptr);

  GridPushConstants push{};
  push.modelMatrix = gridObject.transform.mat4();

  vkCmdPushConstants(
	  frameInfo.commandBuffer,
	  pipelineLayout,
	  VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
	  0,
	  sizeof(GridPushConstants),
	  &push);

  gridObject.model->bind(frameInfo.commandBuffer);
  gridObject.model->draw(frameInfo.commandBuffer);
}

}  // namespace lve
