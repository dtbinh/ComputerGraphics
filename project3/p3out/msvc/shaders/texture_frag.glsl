varying vec2 tex_coord;


uniform sampler2DRect preFrame;

void main(void)
{
	gl_FragColor = texture2DRect(preFrame, tex_coord); // output the previous frame texture
}
