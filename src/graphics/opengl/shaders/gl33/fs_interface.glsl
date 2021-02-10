#version 320 es
precision highp float;

uniform sampler2D uni_Texture;

uniform bool uni_TextureEnabled;

in VertexData
{
    vec4 Color;
    vec2 TexCoord;
} data;

out vec4 out_FragColor;

void main()
{
    if (uni_TextureEnabled)
    {
        out_FragColor = data.Color * texture(uni_Texture, data.TexCoord);
    }
    else
    {
        out_FragColor = data.Color;
    }
}
