//
//   HydraxShaderDepth.hlsl for Ogre
//   Xavier Verguín González
//   December 2007
//

void main_vp(// IN
             float4 iPosition         : POSITION,
             // OUT
             out float4 oPosition     : POSITION,
             out float  oPosition_    : TEXCOORD0,
             // UNIFORM,
             uniform float            uPlaneYPos,
             uniform float4x4         uWorld,
             uniform float4x4         uWorldViewProj)
{
	oPosition = mul(uWorldViewProj, iPosition);
    //output
	oPosition_ = mul(uWorld, iPosition).y;
	//modify the geometry y coord , to take into account plane height
	oPosition_-=uPlaneYPos;
}

void main_fp(// IN
             float  iPosition     : TEXCOORD0,
	         // OUT
	         out float4 oColor    : COLOR,
	         // UNIFORM
	         uniform float        uDepthLimit)
{
    float pixelYDepth = (iPosition*uDepthLimit+1);
	//clamp values to 0,1
	pixelYDepth = saturate(pixelYDepth);

    oColor = float4(pixelYDepth,pixelYDepth,pixelYDepth,pixelYDepth);
}

void main_vp_caustics(// IN
                      float4 iPosition         : POSITION,
                      // OUT
                      out float4 oPosition     : POSITION,
                      out float  oPosition_    : TEXCOORD0,
                      out float2 oUvWorld      : TEXCOORD1,
                      // UNIFORM,
                      uniform float            uPlaneYPos,
                      uniform float4x4         uWorld,
                      uniform float4x4         uWorldViewProj)
{
	oPosition = mul(uWorldViewProj, iPosition);
    //output
	oPosition_ = mul(uWorld, iPosition).y;
	//modify the geometry y coord , to take into account plane height
	oPosition_-=uPlaneYPos;

	oUvWorld = float2(iPosition.x, iPosition.z);
}

void main_fp_caustics(// IN
                      float  iPosition     : TEXCOORD0,
                      float2 iUvWorld      : TEXCOORD1,
	                  // OUT
	                  out float4 oColor    : COLOR,
	                  // UNIFORM
	                  uniform float        uDepthLimit,
	                  uniform float        uCausticsScale,
	                  uniform sampler2D    uCaustics : register(s0))
{
    float pixelYDepth = (iPosition*uDepthLimit+1);
	//clamp values to 0,1
	pixelYDepth = saturate(pixelYDepth);

    oColor = float4(pixelYDepth,pixelYDepth,pixelYDepth,0);

    oColor.g = tex2D(uCaustics, iUvWorld/uCausticsScale).r;
}
