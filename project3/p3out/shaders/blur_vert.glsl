varying vec2 tex_coord;
void main(void)
{
   
	gl_Position =ftransform();
    tex_coord = gl_MultiTexCoord0.st;
	
}
