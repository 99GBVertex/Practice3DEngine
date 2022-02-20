@echo make SPIR-V file with vulkan SDK

%VULKAN_SDK%\Bin\glslc.exe %cd%\ToyProject3D\Shaders\simple_shader.vert -o %cd%\ToyProject3D\Shaders\simple_shader.vert.spv
%VULKAN_SDK%\Bin\glslc.exe %cd%\ToyProject3D\Shaders\simple_shader.frag -o %cd%\ToyProject3D\Shaders\simple_shader.frag.spv

%VULKAN_SDK%\Bin\glslc.exe %cd%\ToyProject3D\Shaders\simple_shader.vert -S -o %cd%\ToyProject3D\Shaders\simple_shader.vert.txt
%VULKAN_SDK%\Bin\glslc.exe %cd%\ToyProject3D\Shaders\simple_shader.frag -S -o %cd%\ToyProject3D\Shaders\simple_shader.frag.txt

%VULKAN_SDK%\Bin\glslc.exe %cd%\ToyProject3D\Shaders\grid_shader.vert -o %cd%\ToyProject3D\Shaders\grid_shader.vert.spv
%VULKAN_SDK%\Bin\glslc.exe %cd%\ToyProject3D\Shaders\grid_shader.frag -o %cd%\ToyProject3D\Shaders\grid_shader.frag.spv