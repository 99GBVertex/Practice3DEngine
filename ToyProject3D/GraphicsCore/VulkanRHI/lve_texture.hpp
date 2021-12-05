#pragma once

#include "lve_device.hpp"

namespace lve {

class LveTexture {

public:
	LveTexture(LveDevice &device);
	LveTexture(LveDevice &device, const std::string &filepath);
	~LveTexture();

	LveTexture(const LveTexture &) = delete;
	LveTexture &operator=(const LveTexture &) = delete;

	static std::unique_ptr<LveTexture> createTextureFromFile(LveDevice &device, const std::string &filepath);

	VkDescriptorImageInfo descriptorInfo();

	VkImageView textureImageView;
	VkSampler textureSampler;

	static std::string DEFAULT_TEXTURE_PATH;

private:
	void createTexture(const std::string &filepath);
	void createTextureImageView(VkImage image);
	void createTextureSampler();

	LveDevice &lveDevice;

	VkImage textureImage;
	VkDeviceMemory textureImageMemory;

};
}