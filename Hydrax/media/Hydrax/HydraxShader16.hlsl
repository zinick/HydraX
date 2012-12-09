//
//   HydraxShader16.hlsl for Ogre
//   Xavier Verguín González
//   December 2007
//

//   Refraction: Texture
//   Reflection: Texture
//   Additional Effects: Depth, Sunlight, Foam, Caustics

void main_vp(// IN
             float4 iPosition         : POSITION,
             float2 iUv               : TEXCOORD0,
             // OUT
             out float4 oPosition     : POSITION,
             out float4 oPosition_    : TEXCOORD0,
             out float2 oUvNoise      : TEXCOORD1,
             out float4 oUvProjection : TEXCOORD2,
             // UNIFORM,
             uniform float4x4         uWorldViewProj)
{
    oPosition_ = iPosition;
	oPosition = mul(uWorldViewProj, iPosition);

	// Projective texture coordinates, adjust for mapping
	float4x4 scalemat = float4x4(0.5,   0,   0, 0.5,
	                               0,-0.5,   0, 0.5,
								   0,   0, 0.5, 0.5,
								   0,   0,   0,   1);

	oUvProjection = mul(scalemat, oPosition);
	oUvNoise = iUv;
}

// Expand a range-compressed vector
float3 expand(float3 v)
{
	return (v - 0.5) * 2;
}

void main_fp(// IN
             float4 iPosition     : TEXCOORD0,
             float2 iUvNoise      : TEXCOORD1,
             float4 iUvProjection : TEXCOORD2,
	         // OUT
	         out float4 oColor    : COLOR,
	         // UNIFORM
	         uniform float3       uEyePosition,
	         uniform float        uFullReflectionDistance,
	         uniform float        uGlobalTransparency,
	         uniform float        uNormalDistortion,
	         uniform float3       uDepthColor,
	         uniform float3       uSunPosition,
	         uniform float        uSunStrength,
	         uniform float        uSunArea,
	         uniform float3       uSunColor,
	         uniform float        uFoamMaxDistance,
	         uniform float        uFoamScale,
	         uniform float        uFoamStart,
	         uniform float        uFoamTransparency,
	         uniform float        uCausticsPower,
	         uniform float        uCausticsEnd,
             uniform sampler2D    uNormalMap       : register(s0),
             uniform sampler2D    uReflectionMap   : register(s1),
             uniform sampler2D    uRefractionMap   : register(s2),
             uniform sampler2D    uDepthMap        : register(s3),
             uniform sampler1D    uFresnelMap      : register(s4),
             uniform sampler2D    uHeightMap       : register(s5),
             uniform sampler2D    uFoamMap         : register(s6))
{
    float2 ProjectionCoord = iUvProjection.xy / iUvProjection.w;

    float3 camToSurface = iPosition.xyz - uEyePosition;
    float additionalReflection=camToSurface.x*camToSurface.x+camToSurface.z*camToSurface.z;

    //calculate the foam visibility as a function fo distance specified by user
    float foamVisibility=additionalReflection;
    foamVisibility=1.0f-saturate(foamVisibility/uFoamMaxDistance);

    //compute the additiona reflection using  the "uFullReflectionDistance" param
    additionalReflection/=uFullReflectionDistance;

    camToSurface=normalize(-camToSurface);

    float3 pixelNormal = tex2D(uNormalMap,iUvNoise);

    //inverte y with z, because at creation our local normal to the plane was z
    pixelNormal.yz=pixelNormal.zy;
    //remap from [0,1] to [-1,1]
    pixelNormal.xyz=expand(pixelNormal.xyz);

    float2 pixelNormalModified = uNormalDistortion*pixelNormal.zx;

    float dotProduct=dot(camToSurface,pixelNormal.xyz);
    //saturate
    dotProduct=saturate(dotProduct);
    //get the fresnel term from our precomputed fresnel texture
    float fresnel = tex1D(uFresnelMap,dotProduct);

    //add additional refrection and saturate
    fresnel+=additionalReflection;
    fresnel=saturate(fresnel);

    //decrease the transparency and saturate
    fresnel-=uGlobalTransparency;
    fresnel=saturate(fresnel);

    //get the reflection pixel. make sure to disturb the texcoords by pixelnormal
    float4 reflection=tex2D(uReflectionMap,ProjectionCoord.xy+pixelNormalModified);
    //get the refraction pixel
    float4 refraction=tex2D(uRefractionMap,ProjectionCoord.xy-pixelNormalModified);

    float2 depth = tex2D(uDepthMap,ProjectionCoord.xy-pixelNormalModified).rg;

    // Add caustics(Stored in green channel of depth map)
    refraction *= 1+saturate((uCausticsEnd-depth.x))*depth.y*uCausticsPower;

    refraction = lerp(refraction,float4(uDepthColor,1),1-depth.x);

    normalize(refraction);

    oColor =lerp(refraction,reflection,fresnel);

    //calculating the reflection vector
    float3 relfectedVector = normalize(reflect(-camToSurface,pixelNormal.xyz));
    //the vecotr from vertex to sun
    float3 surfaceToSun=normalize(uSunPosition-iPosition.xyz);
    //the sunlight reflection
    float3 sunlight = uSunStrength*pow(saturate(dot(relfectedVector,surfaceToSun)),uSunArea)*uSunColor;

    //add the sunlight
    oColor.xyz+=sunlight;

    //get the heightmap texel, to calculate the foam amount for that pixel
    float hmap=tex2D(uHeightMap,iUvNoise).a*foamVisibility;
    //compute foam texture coordinates
    float2 foamTex=iUvNoise*uFoamScale+pixelNormalModified;
    //get the foam texel
    float4 foam=tex2D(uFoamMap,foamTex);

    //here we calculate the transparency of the foam, depending on the height
    //of the pixel in world space, by calculating the height from the heightmap
    float foamTransparency=saturate(hmap-uFoamStart)*uFoamTransparency;

    //add the foam to our final pixel, by linearly interpolating between finalColor and
    //foam texture using foamTransparency and foam alpha
    oColor=lerp(oColor,foam,foamTransparency*foam.a);

    oColor.w = 1;
}
