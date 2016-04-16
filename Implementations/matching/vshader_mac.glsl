#version 150

in vec3 v_position;
in vec4 v_color;
uniform mat4 v_mv;
uniform mat4 v_mvp;
uniform bool is_wireframe;
uniform bool is_colorful;
uniform bool is_smooth;

vec3 fragVertexEc;

flat out int wf_flag;
flat out int sm_flag;
flat out vec4 f_color;
smooth out vec4 s_color;

vec4 green = vec4(0.0f, 1.0f, 0.0f, 0.0f);

void main() {
    if (is_wireframe) {
        f_color = green;
    } else {
        if (is_colorful) {
            f_color = v_color;
        } else {
            f_color = vec4(1.0, 1.0, 1.0, 0.0);
        }
    }
    s_color = f_color;
    wf_flag = is_wireframe ? 1 : 0;
    sm_flag = is_smooth ? 1 : 0;
    fragVertexEc = (v_mv * vec4(v_position, 1.0)).xyz;
    gl_Position = v_mvp * vec4(v_position, 1.0);
}
