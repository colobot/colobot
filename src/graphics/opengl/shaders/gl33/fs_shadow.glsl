#version 320 es
precision highp float;

uniform sampler2D uni_Texture;

uniform bool uni_TextureEnabled;

uniform bool uni_AlphaTestEnabled;
uniform float uni_AlphaReference;

in VertexData
{
    vec2 TexCoord;
} data;

out vec4 out_FragColor;

void main()
{
    float alpha = 1.0f;

    if (uni_TextureEnabled)
    {
        alpha *= texture(uni_Texture, data.TexCoord).a;
    }

    if (uni_AlphaTestEnabled)
    {
        if(alpha < uni_AlphaReference)
            discard;
    }

    out_FragColor = vec4(1.0f);
}
