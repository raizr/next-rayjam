#version 100

precision mediump float;

const int MAX_INDEXED_COLORS = 8;

varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform sampler2D texture0;
uniform ivec3 palette[MAX_INDEXED_COLORS];

void main()
{
    vec4 texelColor = texture2D(texture0, fragTexCoord) * fragColor;
    float r = clamp(texelColor.r, 0.0, 1.0);
    float g = clamp(texelColor.g, 0.0, 1.0);
    float b = clamp(texelColor.b, 0.0, 1.0);
    int idx = int(max(max(r, g), b) * float(MAX_INDEXED_COLORS-1));
    ivec3 color = ivec3(0);
    if (idx == 0) color = palette[0];
    else if (idx == 1) color = palette[1];
    else if (idx == 2) color = palette[2];
    else if (idx == 3) color = palette[3];
    else if (idx == 4) color = palette[4];
    else if (idx == 5) color = palette[5];
    else if (idx == 6) color = palette[6];
    else if (idx == 7) color = palette[7];
    gl_FragColor = vec4(float(color.x)/255.0, float(color.y)/255.0, float(color.z)/255.0, texelColor.a);
}
