uniform sampler2D normalmap;
uniform sampler2D colormap;
uniform sampler3D attenuation;

void main(void)
{
    vec4 normal = texture2D(normalmap, gl_TexCoord[0].xy);
    normal.rgb = normal.rgb * 2.0 - 1.0;
    
    vec3 lightvec = normalize(gl_TexCoord[1].xyz);

    float diffdot = clamp(dot(normal.rgb, lightvec), 0.0, 1.0);

    vec4 color = texture2D(colormap, gl_TexCoord[0].xy);

    vec3 halfvec = normalize(gl_TexCoord[2].xyz);
    float specdot = pow(clamp(dot(normal.rgb, halfvec), 0.0, 1.0), 16.0) * normal.a;

    float selfshadow = clamp(lightvec.z * 8.0, 0.0, 1.0);

    vec4 atten = texture3D(attenuation, gl_TexCoord[3].xyz);

    gl_FragColor = atten * gl_Color * ( color * diffdot * selfshadow + specdot);
}
