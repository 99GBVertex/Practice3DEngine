@echo make SPIR-V file from HLSL

%cd%\ThirdParty\dxc\dxc_2021_12_08\bin\x64\dxc -spirv -T vs_6_6 -E VSMain %cd%\ToyProject3D\Shaders\simple_shader.hlsl -Fo %cd%\ToyProject3D\Shaders\simple_shader_hlsl.vert.spv
%cd%\ThirdParty\dxc\dxc_2021_12_08\bin\x64\dxc -spirv -T ps_6_6 -E PSMain %cd%\ToyProject3D\Shaders\simple_shader.hlsl -Fo %cd%\ToyProject3D\Shaders\simple_shader_hlsl.frag.spv

%cd%\ThirdParty\dxc\dxc_2021_12_08\bin\x64\dxc -spirv -T vs_6_6 -E VSMain %cd%\ToyProject3D\Shaders\grid_shader.hlsl -Fo %cd%\ToyProject3D\Shaders\grid_shader_hlsl.vert.spv
%cd%\ThirdParty\dxc\dxc_2021_12_08\bin\x64\dxc -spirv -T ps_6_6 -E PSMain %cd%\ToyProject3D\Shaders\grid_shader.hlsl -Fo %cd%\ToyProject3D\Shaders\grid_shader_hlsl.frag.spv

