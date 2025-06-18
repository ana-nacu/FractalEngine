#version 410 core

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform vec3 viewPos;

void main() {
    // ✅ Normalizare pentru siguranță
    vec3 norm = normalize(Normal);
//    vec3 norm = normalize(cross(dFdx(FragPos), dFdy(FragPos)));
    // ✅ Direcția spre lumină
    vec3 lightDir = normalize(lightPos - FragPos);

    // ✅ Calculăm iluminarea difuză
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // ✅ Componenta ambientală
    vec3 ambient = 0.2 * objectColor;

    // ✅ Final Color
    vec3 result = (ambient + diffuse) * objectColor;
    FragColor = vec4(result, 1.0);
//    FragColor = vec4(1.0,1.0,1.0, 1.0);
//    FragColor = vec4(normalize(Normal) * 0.5 + 0.5, 1.0);
//    FragColor = vec4(norm * 0.5 + 0.5, 1.0); // colorează în funcție de normală
}