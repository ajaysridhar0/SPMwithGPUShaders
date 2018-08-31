#version 430 compatibility
//Default fragment shader

out vec4 fFragColor;
in vec2 vTexCoord;

uniform sampler2D prevRender; // texture with the previous render pass

 layout(pixel_center_integer) in vec4 gl_FragCoord;
 // will give the screen position of the current fragment

void main()
{
	vec4 currentValue = texture(prevRender, vTexCoord); // what is currently stored in this pixel
	fFragColor = vec4(currentValue.rgb, currentValue.a);
	//fFragColor = vec4(1., 1., 0., 1.);
}