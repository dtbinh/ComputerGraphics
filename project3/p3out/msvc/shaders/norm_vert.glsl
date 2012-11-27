varying vec2 tex_coord;
varying vec3 norm;
void main(void)
{
   
	gl_Position =ftransform();
    tex_coord = gl_MultiTexCoord0.st;
	norm = gl_Normal;
}
