#version 450
#extension GL_ARB_separate_shader_objects : enable

out gl_PerVertex {
	vec4  gl_Position;
};

vec2 inputVertex[3] = {
	vec2(0.0, 0.5),
	vec2(0.5, 0.0),
	vec2(-0.5, 0.0)
};

vec3 inputColor[3] = vec3[](
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0)
);

layout(location = 0) out vec3 fragColor;

void main() {
	gl_Position = vec4(inputVertex[gl_VertexIndex], 0.0, 1.0);
	fragColor = inputColor[gl_VertexIndex];
}