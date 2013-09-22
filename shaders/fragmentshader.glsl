#version 120
uniform sampler2DRect texture;
void main(void)
{
gl_FragColor = texture2DRect(texture, gl_TexCoord[0].st);
//	 gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}

