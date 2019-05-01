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
out vec4 color;

uniform vec4 dist_AmbientProduct, dist_DiffuseProduct, dist_SpecularProduct;
uniform vec4 mod_AmbientProduct, mod_DiffuseProduct, mod_SpecularProduct;
uniform mat4 ModelView;
uniform mat4 Projection;
uniform mat3 Normal_Matrix;
uniform vec4 LightPosition;   // Must be in Eye Frame
uniform float Shininess;

uniform float Spotlight;
uniform float Pointsource;

uniform float expSpot;
uniform float angSpot;

uniform vec4 dirSpotDest;

uniform float ConstAtt;  // Constant Attenuation
uniform float LinearAtt; // Linear Attenuation
uniform float QuadAtt;   // Quadratic Attenuation

void main()
{
    // Transform vertex  position into eye coordinates
    vec3 pos = (ModelView * vPosition).xyz;

    //New
    vec4 global_ambient_light =vec4(1.0,1.0,1.0,1);
    vec3 dist_l_direction = vec3(0.1, 0.0, -1);

    vec3 L_distant = normalize(-dist_l_direction); //Line 45
    vec3 E_distant = normalize(-pos);
    vec3 H_distant = normalize(L_distant + E_distant);

 
    vec3 L = normalize( LightPosition.xyz - pos );
    vec3 E = normalize( -pos );
    vec3 H = normalize( L + E );

    // Transform vertex normal into eye coordinates
      // vec3 N = normalize( ModelView*vec4(vNormal, 0.0) ).xyz;
    vec3 N = normalize(Normal_Matrix * vNormal);

// YJC Note: N must use the one pointing *toward* the viewer
//     ==> If (N dot E) < 0 then N must be changed to -N
//
   if ( dot(N, E) < 0 ) N = -N; //Line 61


/*--- To Do: Compute attenuation ---*/
    float attenuation = 1.0f;
    float spot_attenuation;
    vec3 cur_dir_vec = LightPosition.xyz - pos;
    float cur_distance = length(cur_dir_vec);

    vec3 spot_light_floor;
    vec3 spot_light_S;

    if(Spotlight == 1.0){
        spot_light_floor = normalize(dirSpotDest.xyz - LightPosition.xyz);
        spot_light_S = -L;
	spot_attenuation = pow(dot(spot_light_S, spot_light_floor), expSpot);
	attenuation = 1/(ConstAtt + LinearAtt*cur_distance + QuadAtt*cur_distance*cur_distance) * spot_attenuation;
	
    } else if(Pointsource == 1.0){
	attenuation = 1/(ConstAtt + LinearAtt*cur_distance + QuadAtt*cur_distance*cur_distance);
    }
 // Compute terms in the illumination equation

    //distant light
    vec4 ambient_dist = dist_AmbientProduct;

    float d = max( dot(L_distant, N), 0.0 );
    vec4  diffuse_dist = d * dist_DiffuseProduct;

    float s = pow( max(dot(N, H_distant), 0.0), Shininess );
    vec4  specular_dist = s * dist_SpecularProduct;
    
    if( dot(L_distant, N) < 0.0 ) {
	specular_dist = vec4(0.0, 0.0, 0.0, 1.0);
    } 

   //Spot/Point Lights
    vec4 ambient = mod_AmbientProduct;

    float d_mod = max( dot(L, N), 0.0 ); //Line 100
    vec4  diffuse = d_mod * mod_DiffuseProduct;

    float s_mod = pow( max(dot(N, H), 0.0), Shininess );
    vec4  specular = s_mod * mod_SpecularProduct;
    
    if( dot(L, N) < 0.0 ) {
	specular = vec4(0.0, 0.0, 0.0, 1.0);
    } 

    gl_Position = Projection * ModelView * vPosition;

/*--- attenuation below must be computed properly ---*/
//Change the output 
    vec4 base_color = global_ambient_light * ambient_dist + (1.0f * (ambient_dist  + diffuse_dist + specular_dist )); //For distant light source
   
    if(Spotlight == 1.0){
	vec4 base_color_pointee = attenuation * (ambient + diffuse + specular);
	vec4 base_color_spot = spot_attenuation * base_color_pointee;
	if(dot(spot_light_S, spot_light_floor) < angSpot){
		base_color_spot = vec4(0,0,0,1);
    	}
	color = base_color + base_color_spot;
    } else if(Pointsource == 1.0){
	vec4 base_color_pointee = attenuation * (ambient + diffuse + specular);
	color = base_color + base_color_pointee ;
    }else{
	color = base_color;
    }
    
    //color = attenuation * (ambient + diffuse + specular);
}
