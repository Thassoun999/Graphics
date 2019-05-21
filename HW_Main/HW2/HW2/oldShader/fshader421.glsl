/*****************************
 * File: fshader42.glsl
 *       A simple fragment shader
 *****************************/

// #version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version

/**
 * For Mac OS X, GLSL version is by default 120 (OpenGL 2.1 with compatibility profile).
 * We need to use legacy synatx in GLSL 120:
 * 1. Replace 'out' with 'varying' for variables sent from vertex shader.
 * 2. Use built-in variable 'gl_FragColor' for the final output color ('out vec4 fColor' is not needed).
 */

/* GLSL 120 */

varying float z;
varying vec4 color;
varying vec2 texCoord;
varying vec2 latticeCoord;

//varying float discar;

uniform int fogswitch;
uniform float fogstart;
uniform float fogend;
uniform float fogdens;
uniform vec4 fogcolor;

// texture

uniform sampler2D texture_2D;
uniform bool floortexture;
uniform bool spheretexture;
uniform bool lattice;


void main(){
    
//    if (discar == 1.0) discard;

    // lattice
    if (lattice) if (fract(4.0 * latticeCoord.x) < 0.35 && fract(4.0 * latticeCoord.y) < 0.35) discard;
    
    // texture mapping
    if (floortexture) gl_FragColor = color * texture2D(texture_2D, texCoord);
    else if (spheretexture){
        gl_FragColor = color * texture2D(texture_2D, texCoord);
        if (texture2D(texture_2D, texCoord).r == 0.0) gl_FragColor = color * vec4(0.9, 0.1, 0.1, 1.0);
    } else gl_FragColor = color;
    
    
    // fog
    float fogfac = 0.0;
    if (fogswitch == 1) fogfac = (fogend - z) / (fogend - fogstart);
    else if (fogswitch == 2) fogfac = exp(-(fogdens * z));
    else if (fogswitch == 3) fogfac = exp(-(pow(fogdens * z, 2.0)));
    
    if (fogswitch != 0){
        gl_FragColor = mix(fogcolor, gl_FragColor, clamp(fogfac, 0.0, 1.0));
    }

    
}

