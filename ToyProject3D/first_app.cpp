#include "first_app.hpp"

#include "keyboard_movement_controller.hpp"
#include "GraphicsCore/VulkanRHI/lve_buffer.hpp"
#include "GraphicsCore/VulkanRHI/lve_frame_info.hpp"
#include "GraphicsCore/VulkanRHI/lve_camera.hpp"
#include "GraphicsCore/VulkanRHI/simple_render_system.hpp"

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
	glm::mat4 projectionView{ 1.f };
	glm::vec3 lightDirection = glm::normalize(glm::vec3{1.f, -3.f, -1.f});
};

FirstApp::FirstApp() { loadGameObjects(); }

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

  SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass()};
  LveCamera camera{};

  auto viewerObject = LveGameObject::createGameObject();
  viewerObject.transform.rotation = { glm::radians(-5.f), 0.f, 0.f };
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
	camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 1000.f);

    if (auto commandBuffer = lveRenderer.beginFrame()) {
		int frameIndex = lveRenderer.getFrameIndex();
		FrameInfo frameInfo{
			frameIndex,
			frameTime,
			commandBuffer,
			camera
		};

		// update
		GlobalUbo ubo{};
		ubo.projectionView = camera.getProjection() * camera.getView();
		uboBuffers[frameIndex]->writeToBuffer(&ubo);
		uboBuffers[frameIndex]->flush();

		// render
		lveRenderer.beginSwapChainRenderPass(commandBuffer);
		simpleRenderSystem.renderGameObjects(frameInfo, gameObjects);
		lveRenderer.endSwapChainRenderPass(commandBuffer);
		lveRenderer.endFrame();
    }
  }

  vkDeviceWaitIdle(lveDevice.device());
}

void FirstApp::loadGameObjects() {
	std::string currentPath = std::filesystem::current_path().string();
	std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(lveDevice, currentPath + "/ToyProject3D/Resources/Models/bb8.obj");
	std::shared_ptr<LveTexture> lveTexture = LveTexture::createTextureFromFile(lveDevice, currentPath + "/ToyProject3D/Resources/Textures/Body diff MAP.jpg");

	auto gameObj = LveGameObject::createGameObject();
	gameObj.model = lveModel;
	gameObj.texture = lveTexture;
	gameObj.transform.translation = { .0f, 300.0f, 700.0f };
	gameObj.transform.rotation = { 0.f, glm::radians(90.f), glm::radians(180.f) };
	gameObj.transform.scale = glm::vec3(3.f);
	gameObjects.push_back(std::move(gameObj));
}

}  // namespace lve
