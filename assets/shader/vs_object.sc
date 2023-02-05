$input a_position, a_normal, a_tangent, a_bitangent, a_texcoord0
$output v_position, v_normal, v_tangent, v_bitangent, v_texcoord0

#include "se_shader.sh"

void main()
{
	vec4 position = mul(u_model[0], vec4(a_position, 1.0));
	v_position = position.xyz;
	gl_Position = mul(u_viewProj, position);

	v_normal = normalize(mul(u_normal[0], vec4(a_normal, 1.0)).xyz);
	v_tangent = normalize(mul(u_normal[0], vec4(a_tangent, 1.0)).xyz);
	v_bitangent = normalize(mul(u_normal[0], vec4(a_bitangent, 1.0)).xyz);

	v_texcoord0 = a_texcoord0;
}
