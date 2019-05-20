/***************************
 * File: vshader42.glsl:
 *   A simple vertex shader.
 *
 * - Vertex attributes (positions & colors) for all vertices are sent
 *   to the GPU via a vertex buffer object created in the OpenGL program.
 *
 * - This vertex shader uses the Model-View and Projection matrices passed
 *   on from the OpenGL program as uniform variables of type mat4.
 ***************************/

// #version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version

/**
 * For Mac OS X, GLSL version is by default 120 (OpenGL 2.1 with compatibility profile).
 * We need to use legacy synatx in GLSL 120:
 * 1. Replace keyword 'in' with 'attributes' for the input vertex attributes.
 * 2. Replace keyword 'out' with 'varying' for variables sent to fragment shader.
 */

/* GLSL 120 */
attribute vec4 vPosition;
attribute vec4 vColor;
attribute vec3 vNormal;
attribute vec3 vSmooth;
attribute vec2 vTexCoord;
//attribute vec3 vVelo;

varying float z;

varying vec4 color;
varying vec2 texCoord;
varying vec2 latticeCoord;

//varying float discar;


uniform bool lightflag;
uniform bool flatshade;
uniform bool spotlight;
uniform bool floortexture;
uniform bool spheretexture;
uniform bool spheretex2D;
uniform bool verti;
uniform bool objsp;
uniform bool lattice;
uniform bool latticeup;

//uniform bool fireworkflag;
//uniform float fireworktime;


uniform mat4 modelview;
uniform mat4 projection;

uniform mat3 normalmat;

// global ambient light source
uniform vec4 glob_ambient;

// directional(distant) light source
uniform vec4 direc;
uniform vec4 direc_ambient, direc_diffuse, direc_specular;

// spotlight / point light source
uniform vec4 light_pos;
uniform vec4 light_ambient, light_diffuse, light_specular;
uniform float constatten, linearatten, quadatten;
// spotlight specification
uniform vec4 spot_end;
uniform float spot_exp, spot_cutoff; // in radian

// material surface reflection coefficient
uniform vec4 material_ambient, material_diffuse, material_specular;
uniform float material_shiny;


void main(){
//    if (fireworkflag){
//        vec3 pos = (modelview * vPosition).xyz;
//        
//        float x = pos.x + 0.001 * vVelo.x * fireworktime;
//        float y = pos.y + 0.001 * vVelo.y * fireworktime + 0.5 * -0.00000049 * fireworktime * fireworktime;
//        float z = pos.z + 0.001 * vVelo.z * fireworktime;
//        
//        if (y < 0.1) discar = 1.0;
//        else discar = 0.0;
//        
//        color = vColor;
//        gl_Position = projection * vec4(x,y,z,1);
//        
//    } else {
        if (!lightflag){
            color = vColor;
            
        } else {
            // Transform vertex position into eye coordinates
            vec3 pos = (modelview * vPosition).xyz;
            
            // phony reflection model
            
            // global ambient light
            vec4 global_co = glob_ambient * material_ambient;
            
            
            // normal vector
            vec3 N = (flatshade) ? normalize(normalmat * vNormal) : normalize(normalmat * vSmooth);
            
            // directional (distance) light source
            vec3 L = normalize(-direc.xyz);
            vec3 V = normalize(-pos);
            vec3 H = normalize(L + V);
            
            float diffuse = max(dot(L,N), 0.0);
            
            float direc_atten = 1.0;
            vec4 direc_am = direc_ambient * material_ambient;
            vec4 direc_di = diffuse * direc_diffuse * material_diffuse;
            vec4 direc_sp = (diffuse >= 0.0) ? pow(max(dot(N,H), 0.0), material_shiny) * direc_specular * material_specular: vec4(0.0, 0.0, 0.0, 1.0);
            
            vec4 direc_co = direc_atten * (direc_am + direc_di + direc_sp);
            
            
            // spotlight / point light source
            // coordinates in wolrd originally -> transformed into eye frame already
            L = normalize(light_pos.xyz - pos);
            V = normalize(-pos);
            H = normalize(L + V);
            
            diffuse = max(dot(L,N), 0.0);
            
            vec3 dist = light_pos.xyz - pos;
            float distance = sqrt(pow(dist.x, 2.0) + pow(dist.y, 2.0) + pow(dist.z, 2.0));
            
            float light_atten = 1.0 / (constatten + linearatten * distance + quadatten * pow(distance, 2.0));
            if (spotlight){
                vec3 Lf = normalize(spot_end - light_pos).xyz;
                float attri = dot(Lf, -L);
                if (attri < cos(spot_cutoff)) light_atten = 0.0;
                else light_atten *= pow(attri, spot_exp);
            }
            
            vec4 light_am = material_ambient * light_ambient;
            vec4 light_di = material_diffuse * light_diffuse * diffuse;
            vec4 light_sp = (diffuse >= 0.0) ? pow(max(dot(N,H), 0.0), material_shiny) * light_specular * material_specular: vec4(0.0, 0.0, 0.0, 1.0);
            
            vec4 light_co = light_atten * (light_am + light_di + light_sp);
            
            color = global_co + direc_co + light_co;
            
        }
        
        
        if (floortexture) texCoord = vTexCoord;
        else if (spheretexture){
            vec3 texpos = (modelview * vPosition).xyz;
            if (objsp) texpos = vPosition.xyz;
            
            if (!spheretex2D) {
                if (verti) texCoord = vec2(2.5 * texpos.x, 0.0);
                else texCoord = vec2(1.5 * (texpos.x + texpos.y + texpos.z), 0.0);
            } else {
                if (verti) texCoord = vec2(0.5 * (texpos.x + 1.0), 0.5 * (texpos.y + 1.0));
                else texCoord = vec2(0.3 * (texpos.x + texpos.y + texpos.z), 0.3 * (texpos.x - texpos.y + texpos.z));
            }
        }
        
        
        if (lattice){
            if (latticeup) latticeCoord = vec2(0.5 * (vPosition.x + 1.0), 0.5 * (vPosition.y + 1.0));
            else latticeCoord = vec2(0.3 * (vPosition.x + vPosition.y + vPosition.z), 0.3 * (vPosition.x - vPosition.y + vPosition.z));
        }
        
        
        gl_Position = projection * modelview * vPosition;
        z = gl_Position.z;
        

//    }
    
}
