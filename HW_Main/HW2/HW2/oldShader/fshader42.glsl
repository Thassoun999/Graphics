/*****************************
 * File: fshader42.glsl
 *       A simple fragment shader
 *****************************/

#version 150  // YJC: Comment/un-comment this line to resolve compilation errors
//      due to different settings of the default GLSL version

in  vec4 color;
in  vec2 texCoord;

out vec4 fColor;
//out vec2 texCoord;
uniform sampler2D texture_2D; /* Note: If using multiple textures,
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
    if (Texture_app_flag == 1)
        fColor = color * texture( texture_2D, vec2(texCoord.x * 6.0, texCoord.y * 5.0) );
    else
        fColor = color;
    
}