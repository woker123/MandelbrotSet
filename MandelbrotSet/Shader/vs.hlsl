struct VS_IN
{
	float3 position : V_POSITION;
};

struct VS_OUT
{
	float4 s_position : SV_POSITION;
	float3 position : P_POSITION;
};

cbuffer TransMatrix : register(b0)
{
	matrix model;
	matrix view;
	matrix proj;
	matrix mvp;
}

float2 vert[4] = { float2(-2, -2), float2(-2, 2), float2(2, -2), float2(2, 2) };

VS_OUT main(VS_IN vertex)
{
	VS_OUT vo;
	vo.s_position = mul(float4(vertex.position, 1), mvp);
	vo.position = mul(float4(vertex.position, 1), model);
	return vo;
}