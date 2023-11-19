#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoordsIn;

out vec2 TexCoords;

out VS_OUT {
    vec3 positionOut;
    vec3 normalOut;
    vec2 texCoordsOut;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    vs_out.positionOut = vec3(model * vec4(position, 1.0));
    vs_out.normalOut = transpose(inverse(mat3(model))) * normal;
    vs_out.texCoordsOut = texCoordsIn;
    gl_Position = projection * view * model * vec4(position, 1.0);
}