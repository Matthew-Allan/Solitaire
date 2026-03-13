#version 330 core

in vec2 uv;

out vec4 col;

uniform usampler2D atlas;
uniform int card = 0;

// Diamond pattern function
uvec4 diamond(ivec2 pos, int suit) {
    int posLine = (pos.y + pos.x) % 8;
    int negLine = (pos.y - pos.x) % 8;
    if(negLine < 0) negLine += 8;

    ivec2 grr = ivec2(posLine - negLine + 4, posLine + negLine - 4) / 2;

    int posLineOff = (pos.y + pos.x + 2) % 8;
    int negLineOff = (pos.y - pos.x + 2) % 8;
    if(negLineOff < 0) negLineOff += 8;
    int posThick = (posLineOff + pos.x + pos.y + 2) / 8 % 5;
    int negThick = int(floor(float(pos.y - pos.x) / 8.0)) % 5;
    if(negThick < 0) negThick += 5;

    vec4 colour;
    int val = (posThick + (2 * negThick)) % 5;
    if(val == 0)
        colour = vec4(245, 160, 151, 255);
    else if(val == 1)
        colour = vec4(0, 0, 0, 255);
    else
        colour = vec4(250, 214, 184, 255);

    return (posLine / 2) * (negLine / 2) > 0 ?
        uvec4(vec4(texelFetch(atlas, grr + ivec2(suit * 5, 118), 0)) * (colour / 255)) :
        uvec4(0, 0, 0, 255);
}

void main() {
    uvec4 vals = texelFetch(atlas, ivec2(uv), 0);
    int suit = card / 13;
    int value = card % 13;
    if(vals.a == 128u) {
        uvec4 ace = texelFetch(atlas, ivec2(vals) + ivec2(64 - 5, 66 - 24) + ivec2((suit % 2 * 32), (suit / 2 * 30)), 0);
        uvec4 content = texelFetch(atlas, ivec2(vals) + ivec2(62 - 3, -6), 0);
        if(value == 0 && vals.x > 4u && vals.y > 23u && vals.x < 37u && vals.y < 54u && ace.a == 255u) {
            vals = ace;
        } else if(value >= 1 && value <= 9 && vals.x > 2u && vals.y > 5u && vals.x < 39u && vals.y < 72u && content.a > uint(value)) {
            vals = uvec4(0, 0, 0, 255);
        } else if(value >= 1 && value <= 9 && vals.x > 2u && vals.y > 5u && vals.x < 39u && vals.y < 72u && content.a > 0u) {
            vals = texelFetch(atlas, ivec2(content) + ivec2(21 + (9 * suit), 118), 0);
        } else {
            vals = diamond(ivec2(vals), suit);
        }
    } else if(vals.a == 64u) {
        int row = value / 7;
        int coll = value % 7;
        vals = texelFetch(atlas, ivec2(vals) + ivec2((coll * 9), 98 + (row * 10)), 0);
    } else if(vals.a == 32u) {
        vals = texelFetch(atlas, ivec2(vals) + ivec2((suit * 5) + int(suit == 3), 118), 0);
    }

    vec4 base = mix(vec4(155, 23, 45, 255), vec4(20, 16, 19, 255), float(suit > 1));

    col = mix(vec4(vals), base, float(all(equal(vals, ivec4(255)))));
    col = mix(col, vec4(254, 243, 192, 255), float(all(equal(vals, ivec4(0, 0, 0, 255))))) / 255.0;
}