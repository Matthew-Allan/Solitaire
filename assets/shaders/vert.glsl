#version 330 core
layout (location = 0) in vec2 v_pos;
layout (location = 1) in vec2 v_uv;
layout (location = 2) in vec2 v_card_pos;
layout (location = 3) in int v_card;

uniform mat4 camera;
uniform int selected;

out vec2 uv;
flat out int card;
flat out int is_selected;

void main() {
    float inv_squash = abs(float(v_card >> 8) - 127.5) / 127.5;
    vec2 pos = vec2(v_pos.x * inv_squash, v_pos.y);
    uv = v_uv;
    card = v_card;
    is_selected = int(selected == gl_InstanceID);
    gl_Position = camera * vec4((pos + v_card_pos), float(gl_InstanceID) / 255, 1.0);
}