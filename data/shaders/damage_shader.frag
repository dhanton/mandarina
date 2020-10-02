uniform sampler2D texture;

void main()
{
    //set rgb to red, alpha stays the same
    gl_FragColor = vec4(1.0, 0.0, 0.0, texture2D(texture, gl_TexCoord[0].st).a);
}
