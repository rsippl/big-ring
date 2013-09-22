#version 120
uniform sampler2DRect yTex;
uniform sampler2DRect uTex, vTex;
void main(void)
{
	 float nx, ny, y, u, v, r, g, b;

	 nx = gl_TexCoord[0].x;
	 ny = gl_TexCoord[0].y;

	 y = texture2DRect(yTex, vec2(nx, ny)).r;
	 u = texture2DRect(uTex, vec2(nx * 0.5, ny * 0.5)).r;
	 v = texture2DRect(vTex, vec2(nx * 0.5, ny * 0.5)).r;

	 y=1.1643*(y-0.0625);
	 u=u-0.5;
	 v=v-0.5;

	 r=y+1.5958*v;
	 g=y-0.39173*u-0.81290*v;
	 b=y+2.017*u;

	 gl_FragColor = vec4(r, g, b, 1.0);
}

