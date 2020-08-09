#define MAX_ITER 50

struct PS_IN
{
	float4 s_position : SV_POSITION;
	float3 position : P_POSITION;
};

struct PS_OUT
{
	float4 color : SV_TARGET;
};

cbuffer Complex : register(b0)
{
	float2 complex_c;
};

float3 getMandelbrotColor(float2 complex1, float2 complex2);

PS_OUT main(PS_IN pIn)
{
	PS_OUT po;
	po.color = float4(getMandelbrotColor(complex_c, pIn.position.xy), 1);
	return po;
}

float3 getMandelbrotColor(float2 complex1, float2 complex2)
{
	float2 comp = complex1;
	int i;
	for (i = 0; i < MAX_ITER; ++i)
	{
		if (comp.x * comp.x + comp.y * comp.y > 4.0)
			break;
		comp = float2(comp.x * comp.x - comp.y * comp.y, 2 * comp.x * comp.y) + complex2;
	}
	if (i >= MAX_ITER)
		return float3(1, 1, 1);
	else
		return float3(0, 0, 0);

	//return float3(i / MAX_ITER, i / MAX_ITER, i / MAX_ITER);
}