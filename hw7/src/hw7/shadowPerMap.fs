#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec4 FragPosLightSpace;

struct DirLight {
    vec3 direction;
    vec3 lightColor;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform vec3 objectColor;
uniform sampler2D depthMap;
uniform vec3 viewPos;
uniform DirLight dirLight;

uniform float far_plane;
uniform float near_plane; 

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir);
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);


void main()
{    
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result =  CalcDirLight(dirLight, norm, viewDir);
    FragColor = vec4(result, 1.0);
}


float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
   
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float currentDepth = LinearizeDepth(projCoords.z);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.025);
   
    float shadow = 0.0;
    //阴影锯齿消除
    vec2 texelSize = 1.0 / textureSize(depthMap, 0);
    //均值滤波
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(depthMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            pcfDepth = LinearizeDepth(pcfDepth);
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    if(projCoords.z > 1.0)
        shadow = 0.0;
    

    return shadow;
}



vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{

    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0f);
    // combine results
    vec3 ambient = light.ambient * light.lightColor;
    vec3 diffuse = light.diffuse * diff * light.lightColor;
    vec3 specular = light.specular * spec * light.lightColor;

    float shadow = ShadowCalculation(FragPosLightSpace, normal, lightDir);                      
    vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular)) * objectColor;    
    return result;
}