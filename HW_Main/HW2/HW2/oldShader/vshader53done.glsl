/*
 File Name: "vshader53.glsl":
 Vertex shader:
 - Per vertex shading for a single point light source;
 distance attenuation is Yet To Be Completed.
 - Entire shading computation is done in the Eye Frame.
 */

// #version 150  // YJC: Comment/un-comment this line to resolve compilation errors
//      due to different settings of the default GLSL version

in  vec4 vPosition;
in  vec3 vNormal;
in  vec2 vTexCoord;


out vec4 color;
out float camZ;
out vec2 texCoord;

uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform vec4 mod_AmbientProduct, mod_DiffuseProduct, mod_SpecularProduct;
uniform mat4 ModelView;
uniform mat4 Projection;
uniform mat3 Normal_Matrix;
uniform vec4 LightPosition2;   // Must be in Eye Frame
uniform float Shininess;

uniform float Spotlight;
uniform float Pointsource;

uniform vec4 dirSpotDest;

uniform float expSpot;

uniform float ConstAtt;  // Constant Attenuation
uniform float LinearAtt; // Linear Attenuation
uniform float QuadAtt;   // Quadratic Attenuation

void main()
{
    float angSpot= cos(20.0 * (3.14159) / 180); //angle for spotlight
    vec4 LightPosition;
    LightPosition = vec4(-0.1, 0.0, 1.0, 0.0);
    vec4 materialAmb;
    materialAmb = vec4( 0.2 , 0.2, 0.2, 1.0);
    // Transform vertex  position into eye coordinates
    vec3 pos = (ModelView * vPosition).xyz;
    camZ = -(pos.z);
    texCoord = vTexCoord;
    
    vec3 L = normalize( LightPosition.xyz - pos );
    vec3 E = normalize( -pos );
    vec3 H = normalize( L + E );
    
    // Transform vertex normal into eye coordinates
    // vec3 N = normalize( ModelView*vec4(vNormal, 0.0) ).xyz;
    vec3 N = normalize(Normal_Matrix * vNormal);
    
    // YJC Note: N must use the one pointing toward the viewer
    //     ==> If (N dot E) < 0 then N must be changed to -N
    //
    if ( dot(N, E) < 0 ) N = -N;
    
    
    /*--- To Do: Compute attenuation ---*/
    float attenuation = 1.0;
    
    // Compute terms in the illumination equation
    vec4 ambient = AmbientProduct;
    
    float d = max( dot(L, N), 0.0 );
    vec4  diffuse = d * DiffuseProduct;
    
    float s = pow( max(dot(N, H), 0.0), Shininess );
    vec4  specular = s * SpecularProduct;
    
    if( dot(L, N) < 0.0 ) {
        specular = vec4(0.0, 0.0, 0.0, 1.0);
    }
    
    gl_Position = Projection * ModelView * vPosition;
    
    /*--- attenuation below must be computed properly ---*/
    vec4 base_color;
    
    base_color = attenuation * (ambient + diffuse + specular);
    
    /*___________________________________________________________ part d*/
    //Spot/Point Lights
    vec3 L2 = normalize(LightPosition2.xyz - pos);
    vec3 H2 = normalize( L2 + E );
    vec4 ambient2 = mod_AmbientProduct;
    
    float d_mod = max( dot(L2, N), 0.0 );
    vec4 diffuse2 = d_mod * mod_DiffuseProduct;
    
    float s_mod = pow( max(dot(N, H2), 0.0), Shininess );
    vec4 specular2 = s_mod * mod_SpecularProduct;
    
    if( dot(L2, N) < 0.0 )
    {
        specular2 = vec4(0.0, 0.0, 0.0, 1.0);
    }
    float attenuation2 = 0.0f;
    float distance = length(L2);
    if(Pointsource == 1 || Spotlight == 1)
    {
        if(Pointsource == 1)
        {
            attenuation2 = 1/(ConstAtt + LinearAtt*distance + QuadAtt*distance*distance);
        }
        else if( Spotlight == 1)
        {
            vec3 Lf;
            Lf = normalize(dirSpotDest.xyz - LightPosition2.xyz);
            vec3 s = normalize(pos - LightPosition2.xyz);
            if(dot(s, Lf) < angSpot)
            {
                attenuation2 = 0.0f;
            }
            else
            {
                attenuation2 = 1/(ConstAtt + LinearAtt*distance + QuadAtt*distance*distance) * pow(dot(s, Lf), expSpot);
            }
        }
    color = base_color + attenuation2 * (ambient2 + diffuse2 + specular2)+ materialAmb;
    }
    else
    {
        color = base_color;
    }
   
}