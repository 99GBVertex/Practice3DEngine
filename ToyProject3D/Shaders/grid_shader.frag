#version 450

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
	vec4 position;
} push;

void main() {
  // r g b a
  outColor = vec4(.6f, .6f, .6f, 1.f);
}