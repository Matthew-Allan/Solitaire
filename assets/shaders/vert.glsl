#version 330 core
layout (location = 0) in vec2 v_pos;
layout (location = 1) in vec2 v_uv;
layout (location = 2) in vec2 v_card_pos;
layout (location = 3) in int v_card;

uniform mat4 camera;

out vec2 uv;
flat out int card;

void main() {
    uv = v_uv;
    card = v_card;
    gl_Position = camera * vec4((v_pos + v_card_pos), float(gl_InstanceID) / 255, 1.0);
}