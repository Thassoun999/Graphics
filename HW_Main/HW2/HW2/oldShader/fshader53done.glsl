/* 
File Name: "fshader53.glsl":
           Fragment Shader
*/

// #version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version

in  vec4 color;
in float camZ;

in  vec2 texCoord;


out vec4 fColor;

uniform float fogChoice;

uniform sampler2D texture_2D; 
/* Note: If using multiple textures,
                               each texture must be bound to a
                               different texture unit, with the
                               sampler uniform var set accordingly.
                               The (fragment) shader can access all texture units
                               simultaneously.
                               */
uniform int Texture_app_flag;
// 0: no texture application: obj color
// 1: texutre color
// 2: (obj color) * (texture color)

void main() 
{ 
    vec4 newColor = color;

    vec4 fogColor = vec4(0.7f, 0.7f, 0.7f, 0.5f);
    float f;

    //Linear Fog
    if(fogChoice == 1.0f){
	f = (18.0f - camZ) / (18.0f - 0.0f);
	f = clamp(f, 0, 1);
	newColor = (f * color + (1-f) * fogColor) ;
	
    } else if(fogChoice == 2.0f){ //Exponential Fog
	f =  exp(-0.09f * camZ);
	f = clamp(f, 0, 1);
	newColor = (f * color + (1-f) * fogColor);
    } else if(fogChoice == 3.0f){ //Exponential Square Fog
	f = exp(-pow(0.09f * camZ, 2));
	f = clamp(f, 0, 1);
	newColor = (f * color + (1-f) * fogColor);
    }

    if (Texture_app_flag == 1)
        fColor = newColor * texture( texture_2D, vec2(texCoord.x * 6.0, texCoord.y * 5.0) );
    else
        fColor = newColor;
 
} 

