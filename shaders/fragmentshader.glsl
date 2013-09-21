#version 120
uniform sampler2DRect texture;
void main(void)
{
	 gl_FragColor = texture2DRect(texture, gl_TexCoord[0].st);
}

