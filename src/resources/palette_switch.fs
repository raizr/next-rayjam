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
    float idx = clamp(texelColor, 0.0, 1.0) * float(MAX_INDEXED_COLORS-1);
    ivec3 color = palette[int(idx)];
    gl_FragColor = vec4(float(color.x)/255.0, float(color.y)/255.0, float(color.z)/255.0, texelColor.a);
}
