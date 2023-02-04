struct VS_INPUT
{
	float4 pos : POSITION;
	float4 col : COLOR;
};
struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float4 col : COLOR;
};
PS_INPUT main(VS_INPUT input)
{
	PS_INPUT output;
	output.pos = input.pos;
	output.col = input.col;
	return output;
};