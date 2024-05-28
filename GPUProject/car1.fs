#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;
uniform vec3 viewPos;
uniform vec3 areaLightPos; // 면광원의 위치
uniform vec3 areaLightColor; // 면광원의 색상
uniform float areaLightIntensity // 면광원의 강도

void main()
{           
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    float gamma = 2.2;
    color = pow(color, vec3(gamma));
    vec3 normal = normalize(fs_in.Normal);
    
    // ambient
    vec3 ambient = 0.3 * areaLightColor;
    
    // area light diffuse
    vec3 areaLightDir = normalize(areaLightPos - fs_in.FragPos);
    float areaDiff = max(dot(areaLightDir, normal), 0.0);
    vec3 areaDiffuse = areaDiff * areaLightColor;

    // area light specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 areaHalfwayDir = normalize(-areaLightDir + viewDir);  
    float areaSpec = pow(max(dot(normal, areaHalfwayDir), 0.0), 64.0);
    vec3 areaSpecular = areaSpec * areaLightColor;

    // 최종 조명 계산
    vec3 lighting = (ambient + areaDiffuse + areaSpecular) * color;
    
    FragColor = vec4(lighting, 1.0);
}