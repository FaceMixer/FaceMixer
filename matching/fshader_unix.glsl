#version 130

flat in vec4 f_color;
smooth in vec4 s_color;
flat in int wf_flag;
flat in int sm_flag;
out vec4 color;

varying vec3 fragVertexEc;

const vec3 lightPosEc = vec3(0,0,10);
const vec3 lightColor = vec3(1.0,1.0,1.0);

void main() {
    if (wf_flag == 1) {
        if (sm_flag == 1)
            color = s_color;
        else
            color = f_color;
    } else {
        vec3 X = dFdx(fragVertexEc);
        vec3 Y = dFdy(fragVertexEc);
        vec3 normal = normalize(cross(X, Y));

        vec3 lightDirection = normalize(lightPosEc - fragVertexEc);

        float lightPower = max(0.0, dot(lightDirection, normal));

        color = vec4(normal, 1.0);
        if (sm_flag == 1)
            color = vec4(lightColor * lightPower, 1.0) * s_color;
        else
            color = vec4(lightColor * lightPower, 1.0) * f_color;
    }
}

