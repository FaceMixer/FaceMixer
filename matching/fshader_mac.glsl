#version 150

flat in vec4 f_color;
smooth in vec4 s_color;
flat in int wf_flag;
flat in int sm_flag;
out vec4 color;

vec3 fragVertexEc;

const vec3 lightPosEc = vec3(0,0,10);
const vec3 lightColor = vec3(1.0,1.0,1.0);

void main() {
    if (wf_flag == 1) {
        if (sm_flag == 1)
            color = s_color;
        else
            color = f_color;
    } else {
        if (sm_flag == 1)
            color = s_color;
        else
            color = f_color;
    }
}

