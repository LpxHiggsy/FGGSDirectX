//--------------------------------------------------------------------------------------
// File: DX11 Framework.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
	Texture2D txDiffuse : register(t0);
	SamplerState samLinear : register(s0);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
	
	float4 AmbientMtrl;
	float4 AmbientLight;
	
	float4 SpecularMtrl;
	float4 SpecularLight;
	float SpecularPower;
	float3 EyePosW;
	
	float4 DiffuseMtrl;
	float4 DiffuseLight;
	float3 LightVecW;
	float gTime;

}

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float3 PosW : POSITION;
	float3 Norm : NORMAL;
	float2 Tex : TEXCOORD0;

};
//------------------------------------------------------------------------------------
// Vertex Shader 
//------------------------------------------------------------------------------------
VS_OUTPUT VS(float4 Pos : POSITION, float3 NormalL : NORMAL, float2 Tex : TEXCOORD0)
{
	//Pos.xy += 0.5f * sin(Pos.x) * sin(3.0f * gTime);
	//Pos.z *= 0.6f + 0.4f * sin(2.0f * gTime);
	VS_OUTPUT output = (VS_OUTPUT)0;

	output.Norm = mul(float4(NormalL, 0.0f), World).xyz;
	output.Norm = normalize(output.Norm);

	output.PosW = mul(Pos, World);
	output.Pos = mul(Pos, World);
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);

	output.Tex = Tex;

	return output;
}
//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( VS_OUTPUT input ) : SV_Target
{
	
	float4 textureColour = txDiffuse.Sample(samLinear, input.Tex);
	
	input.Norm = normalize(input.Norm);

	float3 toEye = normalize(EyePosW - input.PosW);
	float3 r = reflect(-LightVecW, input.Norm);
		
	// Compute Colour using Diffuse lighting only
	float diffuseAmount = max(dot(LightVecW, input.Norm), 0.0f);
	float specularAmount = pow(max(dot(r, toEye), 0.0f), SpecularPower);
	
	float3 diffuse = diffuseAmount * (DiffuseMtrl * DiffuseLight).rgb;
	float3 ambient = (AmbientMtrl * AmbientLight).rgb;
	float3 specular = specularAmount * (SpecularMtrl * SpecularLight).rgb;

	float4 Color;

	Color.rgb = ambient + diffuse + specular + textureColour;
	Color.a = DiffuseMtrl.a;
	

	return Color;
}
