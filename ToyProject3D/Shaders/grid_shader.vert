#version 450

layout(push_constant) uniform Push {
	vec4 position;
} push;


void main() {
  gl_Position = push.position;
}