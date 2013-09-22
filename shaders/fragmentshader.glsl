#version 120
uniform sampler2DRect yTex;
uniform sampler2DRect uTex, vTex;
void main(void)
{
	 float nx, ny;

	 nx = gl_TexCoord[0].x;
	 ny = gl_TexCoord[0].y;

	 gl_FragColor = texture2DRect(yTex, vec2(nx, ny));
//	 gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}

