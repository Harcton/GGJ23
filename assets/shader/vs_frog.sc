$input a_position, a_normal, a_tangent, a_bitangent, a_weight, a_indices, a_texcoord0
$output v_position, v_normal, v_tangent, v_bitangent, v_texcoord0

#include "se_shader.sh"

void main()
{
    mat4 boneMatrix =
          mul(u_model[a_indices.x], a_weight.x)
        + mul(u_model[a_indices.y], a_weight.y)
        + mul(u_model[a_indices.z], a_weight.z)
        + mul(u_model[a_indices.w], a_weight.w);

	vec4 position = mul(boneMatrix, vec4(a_position, 1.0));
	v_position = position.xyz;
	gl_Position = mul(u_viewProj, position);

    mat4 normalMatrix =
          mul(u_normal[a_indices.x], a_weight.x)
        + mul(u_normal[a_indices.y], a_weight.y)
        + mul(u_normal[a_indices.z], a_weight.z)
        + mul(u_normal[a_indices.w], a_weight.w);

	v_normal = normalize(mul(normalMatrix, vec4(a_normal, 1.0)).xyz);
	v_tangent = normalize(mul(normalMatrix, vec4(a_tangent, 1.0)).xyz);
	v_bitangent = normalize(mul(normalMatrix, vec4(a_bitangent, 1.0)).xyz);

	v_texcoord0 = a_texcoord0;
}
