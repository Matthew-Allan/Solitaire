#version 330 core
in vec2 uv;
flat in int card;

uniform usampler2D atlas;

out vec4 col;

const vec4 colours[3] = vec4[3](
    vec4(245, 160, 151, 255) / 255,
    vec4(254, 243, 192, 255) / 255,
    vec4(250, 214, 184, 255) / 255
);

const vec4 suit_cols[2] = vec4[2](
    vec4(155, 23, 45, 255) / 255,
    vec4(20, 16, 19, 255) / 255
);

#define int_mod(x, m) ((((x) % (m)) + (m)) % (m))
#define line(pos, opp) int_mod((pos).y opp (pos).x, 8)
#define in_box(pos, l, r, t, b) ((pos).x > (l) && (pos).x < (r) && (pos).y > (t) && (pos).y < (b))

vec4 pattern(ivec2 pos, int suit) {
    int posThick = int_mod((pos.x + pos.y) / 8, 5);
    int negThick = int_mod((pos.y - pos.x - int(pos.y - pos.x < 0) * 7) / 8, 5);
    
    int posLine = line(pos, +), negLine = line(pos, -);
    ivec2 patPos = (ivec2(posLine) + ivec2(negLine - 4) * ivec2(-1, 1)) / 2;

    if((posLine / 2) * (negLine / 2) == 0)
        return colours[1];

    if(texelFetch(atlas, patPos + ivec2(suit * 5, 118), 0).r == 255u) {
        return colours[min((posThick + 2 * negThick) % 5, 2)];
    }
    return colours[1];
}

vec4 content(ivec2 vals, int suit, int value) {
    if(value == 0 && in_box(vals, 4, 37, 23, 54)) {
        vec4 ace = vec4(texelFetch(atlas, vals + ivec2(64 - 5, 66 - 24) + ivec2((suit % 2 * 32), (suit / 2 * 30)), 0)) / 255;
        if(ace.a == 1) {
            return ace;
        }
    } else if (value >= 1 && value <= 9 && in_box(vals, 2, 39, 5, 72)) {
        uvec4 centre = texelFetch(atlas, vals + ivec2(62 - 3, -6), 0);
        if(centre.a != 0u && centre.a <= uint(value))
            return vec4(texelFetch(atlas, ivec2(centre) + ivec2(21 + (9 * suit), 118), 0)) / 255;
        if(centre.a != 0u)
            return colours[1];
    }
    return pattern(vals, suit);
}

void main() {
    int suit = card / 13, value = card % 13;
    int row  = value / 7, coll  = value % 7;
    
    uvec4 vals = texelFetch(atlas, ivec2(uv), 0);

    switch (vals.a) {
        case 255u:
            col = vec4(vals) / 255;
            break;
        case 128u:
            col = content(ivec2(vals), suit, value);
            return;
        case 64u:
            col = vec4(texelFetch(atlas, ivec2(vals) + ivec2((coll * 9), 98 + (row * 10)), 0)) / 255;
            break;
        case 32u:
            col = vec4(texelFetch(atlas, ivec2(vals) + ivec2((suit * 5) + int(suit == 3), 118), 0)) / 255;
            break;
        default:
            discard;
    }

    if(col.r == 0) {
        col = colours[1];
    } else if (col.r == 1) {
        col = suit_cols[int(suit > 1)];
    }
}