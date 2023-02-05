$input v_position, v_normal, v_tangent, v_bitangent, v_texcoord0

#include "se_shader.sh"
#include "se_materials.sh"

SAMPLER2D(s_texColor, 0);
SAMPLER2D(s_texNormal, 1);
SAMPLER2D(s_texRoughness, 2);


void main()
{
    vec4 color = texture2D(s_texColor, v_texcoord0);
	vec3 normal = CALC_NORMAL_TEXTURE;
    float specular = 1.0 - texture2D(s_texRoughness, v_texcoord0).r;

    gl_FragColor =
        u_primitiveColor *
        color *
        phong(v_position, normal, specular);
}
