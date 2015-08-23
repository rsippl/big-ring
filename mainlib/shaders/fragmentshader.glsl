#version 110
/*
 * Copyright (c) 2012-2015 Ilja Booij (ibooij@gmail.com)
 *
 * This file is part of Big Ring Indoor Video Cycling
 *
 * Big Ring Indoor Video Cycling is free software: you can redistribute
 * it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Big Ring Indoor Video Cycling  is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Big Ring Indoor Video Cycling.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
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

