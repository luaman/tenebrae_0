// Tenebrae bumpmapping without light filter vertex shader

void main(void)
{
    gl_Position = ftransform();
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_TexCoord[1] = gl_MultiTexCoord1;
    gl_TexCoord[2] = gl_MultiTexCoord2;
    gl_TexCoord[3] = gl_TextureMatrix[2] * gl_Vertex;
    gl_TexCoord[4] = gl_TextureMatrix[3] * gl_Vertex;
    gl_FrontColor = gl_Color;
}
