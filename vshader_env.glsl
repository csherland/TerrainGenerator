//Regular stuff
attribute vec4 vPosition;
attribute vec4 vColor;
attribute vec4 vText;
attribute vec3 vNormal;
varying vec4 color;

//Lighting Stuff
uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform mat4 ModelView;
uniform mat4 Projection;
uniform vec4 LightPosition;
uniform float Shininess;

//Texture stuff
attribute vec2 vTexCoord;
varying vec2 texCoord;
varying vec4 texts;

void main() 
{
    // Transform vertex position into eye coordinates
    vec3 pos = (ModelView * vPosition).xyz;
    
    vec4 LightPos = ModelView * LightPosition;
	
    vec3 L = normalize( LightPos.xyz - pos );
    vec3 E = normalize( -pos );
    vec3 H = normalize( L + E );

    // Transform vertex normal into eye coordinates
    vec3 N = normalize( ModelView*vec4(vNormal, 0.0) ).xyz;

    // Compute terms in the illumination equation
    vec4 ambient = AmbientProduct;

    float Kd = max( dot(L, N), 0.0 );
    vec4  diffuse = Kd*DiffuseProduct;

    float Ks = pow( max(dot(N, H), 0.0), Shininess );
    vec4  specular = Ks * SpecularProduct;
    
    if( dot(L, N) < 0.0 ) {
		specular = vec4(0.0, 0.0, 0.0, 1.0);
    } 

    gl_Position = Projection * ModelView * vPosition;
    texCoord    = vTexCoord;
    texts = vText;
    color = (ambient + diffuse + specular) * vColor;
} 
