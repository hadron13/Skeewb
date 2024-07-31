#version 330

uniform sampler2D texture1;

in vec3 fragPos;
in vec2 texCoord;
in vec3 normal;
out vec4 out_color;


void main(){
    
    vec3 lightColor = vec3(1.0f, 1.1f, 1.0f);
    vec3 lightPos = vec3(2.0f, 5.0f, 2.5f);    
    vec3 lightDir = normalize(lightPos - fragPos);

    float diff = max(dot(normal, lightDir), 0.0f);

    vec3 ambient = 0.2f * lightColor;
    vec3 diffuse = diff * lightColor;

     // specular
    float specularStrength = 0.7;
    vec3 viewDir = normalize(vec3(0) - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
    vec3 specular = specularStrength * spec * lightColor;  


    out_color = vec4(texture(texture1, texCoord).rgb * (ambient + diffuse + specular), 1.0f);
    //out_color = vec4(fragPos, 1.0f);
}
