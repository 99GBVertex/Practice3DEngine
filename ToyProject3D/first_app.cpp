#include "first_app.hpp"

#include "keyboard_movement_controller.hpp"
#include "GraphicsCore/VulkanRHI/lve_buffer.hpp"
#include "GraphicsCore/VulkanRHI/lve_frame_info.hpp"
#include "GraphicsCore/VulkanRHI/lve_camera.hpp"
#include "GraphicsCore/VulkanRHI/simple_render_system.hpp"
#include "GraphicsCore/VulkanRHI/grid_render_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <chrono>
#include <cassert>
#include <stdexcept>
#include <filesystem>
#include <numeric>

namespace lve {

struct GlobalUbo {
	glm::mat4 projectionViewMatrix{ 1.f };
	glm::vec3 lightDirection = glm::normalize(glm::vec3{1.f, -3.f, -1.f});
};

FirstApp::FirstApp() {
	loadGameObjects();
	makeGridObject();
	globalPool = LveDescriptorPool::Builder(lveDevice)
		.setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT * gameObjects.size())
		.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT * gameObjects.size())
		.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, LveSwapChain::MAX_FRAMES_IN_FLIGHT * gameObjects.size())
		.build();
}

FirstApp::~FirstApp() {}

void FirstApp::run() {
	// find lowest common multiple
	auto minOffsetAlignment = std::lcm(
		lveDevice.properties.limits.minUniformBufferOffsetAlignment,
		lveDevice.properties.limits.nonCoherentAtomSize
	);

	std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < uboBuffers.size(); ++i)
	{
		uboBuffers[i] = std::make_unique<LveBuffer>(
			lveDevice,
			sizeof(GlobalUbo),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			/*VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			minOffsetAlignment);*/
		uboBuffers[i]->map();
	}

	//std::vector<std::unique_ptr<LveTexture>> imageInfos(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
	//for (int i = 0; i < imageInfos.size(); ++i)
	//{
	//	imageInfos[i] = std::make_unique<LveTexture>(lveDevice);
	//}

	auto globalSetLayout = LveDescriptorSetLayout::Builder(lveDevice)
		.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.build();

	std::vector<std::vector<VkDescriptorSet>> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < globalDescriptorSets.size(); i++) {
		for (auto& obj : gameObjects) {
			VkDescriptorSet objDescriptorSet;
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			auto ImageInfo = obj.texture->descriptorInfo();
			LveDescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.writeImage(1, &ImageInfo)
				.build(objDescriptorSet);

			globalDescriptorSets[i].push_back(objDescriptorSet);
		}
	}

  SimpleRenderSystem simpleRenderSystem{
	  lveDevice,
	  lveRenderer.getSwapChainRenderPass(),
	  globalSetLayout->getDescriptorSetLayout()};
  GridRenderSystem gridRenderSystem{
	  lveDevice,
	  lveRenderer.getSwapChainRenderPass(),
	  globalSetLayout->getDescriptorSetLayout() };
  LveCamera camera{};

  auto viewerObject = LveGameObject::createGameObject();
  viewerObject.transform.translation = { 0.f, -30.f, -50.f };
  viewerObject.transform.rotation = { glm::radians(-25.f), glm::radians(0.0001f), 0.f };
  KeyboardMovementController cameraController{};

  auto currentTime = std::chrono::high_resolution_clock::now();

  while (!lveWindow.shouldClose()) {
    glfwPollEvents();

	auto newTime = std::chrono::high_resolution_clock::now();
	float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
	currentTime = newTime;

	
	cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject);
	camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

	float aspect = lveRenderer.getAspectRatio();
	camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 3000.f);

    if (auto commandBuffer = lveRenderer.beginFrame()) {
		int frameIndex = lveRenderer.getFrameIndex();
		FrameInfo frameInfo{
			frameIndex,
			frameTime,
			commandBuffer,
			camera,
			nullptr
		};

		// update
		GlobalUbo ubo{};
		ubo.projectionViewMatrix = camera.getProjection() * camera.getView();
		uboBuffers[frameIndex]->writeToBuffer(&ubo);
		uboBuffers[frameIndex]->flush();

		// render
		lveRenderer.beginSwapChainRenderPass(commandBuffer);
		for (int i = 0 ; i < gameObjects.size() ; i ++)
		{
			auto& obj = gameObjects[i];
			frameInfo.globalDescriptorSet = globalDescriptorSets[frameIndex][i];
			// TODO reuse desctripor Writer
			/*auto ImageInfo = obj.texture->descriptorInfo();
			LveDescriptorWriter(*globalSetLayout, *globalPool)
				.writeImage(1, &ImageInfo)
				.overwrite(globalDescriptorSets[frameIndex]);*/
			simpleRenderSystem.renderGameObjects(frameInfo, obj);
		}
		gridRenderSystem.renderGrid(frameInfo, *gridObject.get());
		lveRenderer.endSwapChainRenderPass(commandBuffer);
		lveRenderer.endFrame();
    }
  }

  vkDeviceWaitIdle(lveDevice.device());
}

void FirstApp::loadGameObjects() {
	std::string currentPath = std::filesystem::current_path().string();
	std::vector<std::shared_ptr<LveModel>> lveModels;
	LveModel::createModelFromFile(lveModels, lveDevice, currentPath + "/ToyProject3D/Resources/Models/bb8.obj");
	std::shared_ptr<LveTexture> lveHeadDiffuseTexture = LveTexture::createTextureFromFile(lveDevice, currentPath + "/ToyProject3D/Resources/Textures/HEAD diff MAP.jpg");
	std::shared_ptr<LveTexture> lveBodyDiffuseTexture = LveTexture::createTextureFromFile(lveDevice, currentPath + "/ToyProject3D/Resources/Textures/Body diff MAP.jpg");
	defaultTexture = LveTexture::createTextureFromFile(lveDevice, currentPath + "/ToyProject3D/Resources/Textures/checker.jpg");


	auto objHeadPart = LveGameObject::createGameObject();
	objHeadPart.model = lveModels[0];
	objHeadPart.texture = lveHeadDiffuseTexture;
	objHeadPart.transform.translation = { .0f, .0f, .0f };
	objHeadPart.transform.rotation = { 0.f, glm::radians(90.f), glm::radians(180.f) };
	objHeadPart.transform.scale = glm::vec3(.1f);
	gameObjects.push_back(std::move(objHeadPart));

	auto objBodyPart = LveGameObject::createGameObject();
	objBodyPart.model = lveModels[1];
	objBodyPart.texture = lveBodyDiffuseTexture;
	objBodyPart.transform.translation = { .0f, .0f, .0f };
	objBodyPart.transform.rotation = { 0.f, glm::radians(90.f), glm::radians(180.f) };
	objBodyPart.transform.scale = glm::vec3(.1f);
	gameObjects.push_back(std::move(objBodyPart));
}

void FirstApp::makeGridObject()
{
	//draw x grid
	LveModel::Part grid;
	float gridDimension = 1000.0f;
	int gridSections = 50;
	float halfGridSection = gridSections * 0.5f;
	float divideGrid = 1.f / gridSections;

	//Draw -Z -> +Z (Red->White) //[X-Z Axis]
	LveModel::Vertex vertex{};
	for (int i = 0; i < gridSections; i++)
	{	
		float drawRatio = 1 - glm::abs((i - halfGridSection) * divideGrid);
		vertex.position = { 
			0.0f - gridDimension / 2 + i * gridDimension / gridSections,
			0.0f,
			-gridDimension / 2.0f
		};
		vertex.color = { 1.f, 1.f, 1.f };
		vertex.color *= drawRatio;
		vertex.color -= glm::vec3{ .5f, .5f, .5f };
		grid.vertices.emplace_back(vertex);

		vertex.position = {
			0.0f - gridDimension / 2 + i * gridDimension / gridSections,
			0.0f, 
			+(gridDimension) / 2.0f - gridDimension / gridSections
		};
		vertex.color = glm::vec3{ 1.f, 1.f, 1.f };
		vertex.color *= drawRatio;
		vertex.color -= glm::vec3{ .5f, .5f, .5f };
		grid.vertices.emplace_back(vertex);
	}

	//Draw -X -> +X (Blue->White)  //[X-Z Axis]
	for (int i = 0; i < gridSections; i++)
	{
		float drawRatio = 1 - glm::abs((i - halfGridSection) * divideGrid);
		vertex.position = {
			-gridDimension / 2.0f, 
			0.0f, 
			0.0f - gridDimension / 2 + i * gridDimension / gridSections
		};
		vertex.color = { 1.f, 1.f, 1.f };
		vertex.color *= drawRatio;
		vertex.color -= glm::vec3{ .5f, .5f, .5f };
		grid.vertices.emplace_back(vertex);

		vertex.position = {
			+gridDimension / 2.0f - gridDimension / gridSections,
			0.0f, 
			0.0f - gridDimension / 2 + i * gridDimension / gridSections
		};
		vertex.color = { 1.f, 1.f, 1.f };
		vertex.color *= drawRatio;
		vertex.color -= glm::vec3{ .5f, .5f, .5f };
		grid.vertices.emplace_back(vertex);
	}

	//Draw -X -> +X (Blue->White) //[X-Y Axis]
	/*for (int i = 0; i < gridSections; i++)
	{
		vertex.position = {
			-gridDimension / 2.0f, 
			0.0f - gridDimension / 2 + i * gridDimension / gridSections,
			gridDimension
		};
		vertex.color = { .0f, .0f, 1.f };
		grid.vertices.emplace_back(vertex);

		vertex.position = {
			+gridDimension / 2.0f - gridDimension / gridSections,
			0.0f - gridDimension / 2 + i * gridDimension / gridSections,
			gridDimension
		};
		vertex.color = { 1.f, 1.f, 1.f };
		grid.vertices.emplace_back(vertex);
	}*/

	//Draw -Y -> +Y (Green->White) //[X-Y Axis]
	/*for (int i = 0; i < gridSections; i++)
	{
		vertex.position = {
			0.0f - gridDimension / 2 + i * gridDimension / gridSections,
			-gridDimension / 2.0f,
			gridDimension
		};
		vertex.color = { .0f, 1.f, .0f };
		grid.vertices.emplace_back(vertex);

		vertex.position = {
			0.0f - gridDimension / 2 + i * gridDimension / gridSections, 
			+gridDimension / 2.0f - gridDimension / gridSections,
			gridDimension
		};
		vertex.color = { 1.f, 1.f, 1.f };
		grid.vertices.emplace_back(vertex);
	}*/
	grid.vertices.emplace_back(vertex);
	
	std::shared_ptr<LveModel> gridModel = std::make_unique<LveModel>(lveDevice, grid);

	auto gridObj = LveGameObject::createGameObject();
	gridObj.model = gridModel;
	gridObj.texture = defaultTexture;
	gridObj.transform.translation = { .0f, .0f, .0f };
	gridObj.transform.rotation = { 0.f, 0.f, 0.f };
	gridObj.transform.scale = glm::vec3(1.f);
	gridObject = std::make_unique<LveGameObject>(std::move(gridObj));
}

}  // namespace lve
