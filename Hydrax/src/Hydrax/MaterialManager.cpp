/*
--------------------------------------------------------------------------------
This source file is part of Hydrax.
Visit ---

Copyright (C) 2008 Xavier Verguín González <xavierverguin@hotmail.com>
                                           <xavyiy@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA  02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
--------------------------------------------------------------------------------
*/

#include "MaterialManager.h"

#include "Hydrax.h"

#define _def_Water_Material_Name  "_Hydrax_Water_Material"
#define _def_Water_Shader_VP_Name "_Hydrax_Water_VP"
#define _def_Water_Shader_FP_Name "_Hydrax_Water_FP"

#define _def_Depth_Material_Name  "_Hydrax_Depth_Material"
#define _def_Depth_Shader_VP_Name "_Hydrax_Depth_VP"
#define _def_Depth_Shader_FP_Name "_Hydrax_Depth_FP"

namespace Hydrax
{
	MaterialManager::MaterialManager(Hydrax *h)
		: mComponents(HYDRAX_COMPONENTS_NONE)
		, mHydrax(h)
		, mCreated(false)
	{
		for (int k = 0; k < 2; k++)
		{
			mMaterials[k].setNull();
		}
	}

	MaterialManager::~MaterialManager()
	{
		removeMaterials();
	}

	void MaterialManager::removeMaterials()
	{
		if (Ogre::MaterialManager::getSingleton().resourceExists(_def_Water_Material_Name))
		{
			Ogre::MaterialManager::getSingleton().remove(_def_Water_Material_Name);

			Ogre::HighLevelGpuProgramManager::getSingleton().unload(_def_Water_Shader_VP_Name);
		    Ogre::HighLevelGpuProgramManager::getSingleton().unload(_def_Water_Shader_FP_Name);
			Ogre::HighLevelGpuProgramManager::getSingleton().remove(_def_Water_Shader_VP_Name);
		    Ogre::HighLevelGpuProgramManager::getSingleton().remove(_def_Water_Shader_FP_Name);
		}

		if (Ogre::MaterialManager::getSingleton().resourceExists(_def_Depth_Material_Name))
		{
			Ogre::MaterialManager::getSingleton().remove(_def_Depth_Material_Name);

			Ogre::HighLevelGpuProgramManager::getSingleton().unload(_def_Depth_Shader_VP_Name);
		    Ogre::HighLevelGpuProgramManager::getSingleton().unload(_def_Depth_Shader_FP_Name);
			Ogre::HighLevelGpuProgramManager::getSingleton().remove(_def_Depth_Shader_VP_Name);
		    Ogre::HighLevelGpuProgramManager::getSingleton().remove(_def_Depth_Shader_FP_Name);
		}

		mCreated = false;
	}

	bool MaterialManager::createMaterials(const HydraxComponent &Components, const Options &Options)
	{
		removeMaterials();

		HydraxLOG("Creating water material...");
		if (!_createWaterMaterial(Components, Options))
		{
			return false;
		}
		HydraxLOG("Water material created.");

		if (_isComponent(Components, HYDRAX_COMPONENT_DEPTH))
		{
			HydraxLOG("Creating depth material...");
			if(!_createDepthMaterial(Components, Options))
			{
				return false;
			}
			HydraxLOG("Depth material created.");
		}

		mComponents = Components;
		mOptions = Options;
		mCreated = true;

		return true;
	}

	bool MaterialManager::_createWaterMaterial(const HydraxComponent &Components, const Options &Options)
	{
		Ogre::String ShaderStr = "";
		Ogre::String ShaderModesStr[3] = {"hlsl", "cg", "glsl"};
		Ogre::String Profiles[2];

		const bool cDepth    = _isComponent(Components, HYDRAX_COMPONENT_DEPTH   );
		const bool cSmooth   = _isComponent(Components, HYDRAX_COMPONENT_SMOOTH  );
		const bool cSun      = _isComponent(Components, HYDRAX_COMPONENT_SUN     );
		const bool cFoam     = _isComponent(Components, HYDRAX_COMPONENT_FOAM    );
		const bool cCaustics = _isComponent(Components, HYDRAX_COMPONENT_CAUSTICS);

		// First: build our shader
		// Vertex program:

		switch (Options.SM)
		{
		    case SM_HLSL:
			{
				Profiles[0] = "target";
				Profiles[1] = "vs_1_1";
			}
			break;

			case SM_CG:
			{
				Profiles[0] = "profiles";
				Profiles[1] = "vs_1_1 arbvp1";
			}
			break;

			case SM_GLSL:
			{}
			break;
		}

		switch (Options.NM)
		{
		    case NM_TEXTURE:
		    {
				switch (Options.SM)
				{
				    case SM_HLSL: case SM_CG:
					{
						ShaderStr += 
						Ogre::String(
						"void main_vp(\n") +
						       // IN
                          	   "float4 iPosition         : POSITION,\n" +
                           	   "float2 iUv               : TEXCOORD0,\n" +
                           	   // OUT
                           	   "out float4 oPosition     : POSITION,\n" +
                               "out float4 oPosition_    : TEXCOORD0,\n" +
                               "out float2 oUvNoise      : TEXCOORD1,\n" +
                               "out float4 oUvProjection : TEXCOORD2,\n" +
                               // UNIFORM
                               "uniform float4x4         uWorldViewProj)\n" +
               	        "{\n" +
                  	        "oPosition_ = iPosition;\n" +
	              	        "oPosition = mul(uWorldViewProj, iPosition);\n" +
	               	        // Projective texture coordinates, adjust for mapping
	                	    "float4x4 scalemat = float4x4(0.5,   0,   0, 0.5,"+
	                                              	     "0,-0.5,   0, 0.5,"+
	                							  	     "0,   0, 0.5, 0.5,"+
	                							  	     "0,   0,   0,   1);\n" +
	               	        "oUvProjection = mul(scalemat, oPosition);\n" +
	               	        "oUvNoise = iUv;\n" +
               	         "}\n";
					}
					break;

					case SM_GLSL:
					{}
					break;
				}
		    }
		    break;

			case NM_VERTEX:
		    {
				switch (Options.SM)
				{
				    case SM_HLSL: case SM_CG:
					{
						ShaderStr += 
						Ogre::String(
						"void main_vp(\n") +
						       // IN
                          	   "float4 iPosition         : POSITION,\n" +
							   "float3 iNormal           : NORMAL,\n"+
                           	   // OUT
                           	   "out float4 oPosition     : POSITION,\n" +
                               "out float4 oPosition_    : TEXCOORD0,\n" +
                               "out float4 oUvProjection : TEXCOORD1,\n" +
							   "out float3 oNormal       : TEXCOORD2,\n" +
                               // UNIFORM
                               "uniform float4x4         uWorldViewProj)\n" +
               	        "{\n" +
                  	        "oPosition_ = iPosition;\n" +
	              	        "oPosition = mul(uWorldViewProj, iPosition);\n" +
	               	        // Projective texture coordinates, adjust for mapping
	                	    "float4x4 scalemat = float4x4(0.5,   0,   0, 0.5,"+
	                                              	     "0,-0.5,   0, 0.5,"+
	                							  	     "0,   0, 0.5, 0.5,"+
	                							  	     "0,   0,   0,   1);\n" +
	               	        "oUvProjection = mul(scalemat, oPosition);\n" +
							"oNormal = normalize(iNormal);\n"+
               	         "}\n";
					}
					break;

					case SM_GLSL:
					{}
					break;
				}
		    }
		    break;

			case NM_RTT:
		    {}
		    break;
		}

        Ogre::HighLevelGpuProgramPtr VP = 
			Ogre::HighLevelGpuProgramManager::getSingleton().
			      createProgram(_def_Water_Shader_VP_Name, 
			                    HYDRAX_RESOURCE_GROUP, 
					  		    ShaderModesStr[static_cast<int>(Options.SM)], 
			                    Ogre::GPT_VERTEX_PROGRAM);

	    VP->setSource(ShaderStr);
        VP->setParameter("entry_point", "main_vp");
        VP->setParameter(Profiles[0], Profiles[1]);
		VP->load();

		// Fragment program
		ShaderStr = "";

		switch (Options.SM)
		{
		    case SM_HLSL:
			{
				Profiles[0] = "target";
				Profiles[1] = "ps_2_0";
			}
			break;

			case SM_CG:
			{
				Profiles[0] = "profiles";
				Profiles[1] = "ps_2_0 arbfp1 fp20";
			}
			break;

			case SM_GLSL:
			{}
			break;
		}

		switch (Options.NM)
		{
		    case NM_TEXTURE: case NM_VERTEX:
		    {
				switch (Options.SM)
				{
				    case SM_HLSL: case SM_CG:
					{
						ShaderStr += 
							Ogre::String("float3 expand(float3 v)\n") +
						    "{\n" +
	                            "return (v - 0.5) * 2;\n" + 
							"}\n\n" +

							"void main_fp(" +
							    // IN
                                "float4 iPosition     : TEXCOORD0,\n";
						int TEXCOORDNUM = 1;
						if (Options.NM == NM_TEXTURE)
						{
							ShaderStr +=
								"float2 iUvNoise      : TEXCOORD" + Ogre::StringConverter::toString(TEXCOORDNUM) + ",\n";
							TEXCOORDNUM++;

						}
						ShaderStr +=
                                "float4 iUvProjection : TEXCOORD" + Ogre::StringConverter::toString(TEXCOORDNUM) + ",\n";
						TEXCOORDNUM++;
						if (Options.NM == NM_VERTEX)
						{
							ShaderStr += 
							    "float4 iNormal       : TEXCOORD" + Ogre::StringConverter::toString(TEXCOORDNUM) + ",\n";
						}
						ShaderStr +=
							Ogre::String(
	                            // OUT
	                            "out float4 oColor    : COLOR,\n") +
	                            // UNIFORM
	                            "uniform float3       uEyePosition,\n" +
	                            "uniform float        uFullReflectionDistance,\n" +
	                            "uniform float        uGlobalTransparency,\n" +
	                            "uniform float        uNormalDistortion,\n";

						if (cDepth)
						{
							ShaderStr += 
								"uniform float3       uDepthColor,\n";
						}
						if (cSmooth)
						{
							ShaderStr += 
								"uniform float        uSmoothPower,\n";
						}
						if (cSun)
						{
							ShaderStr += Ogre::String(
							    "uniform float3       uSunPosition,\n") +
	                            "uniform float        uSunStrength,\n" +
	                            "uniform float        uSunArea,\n" +
	                            "uniform float3       uSunColor,\n";
						}
						if (cFoam)
						{
							ShaderStr += Ogre::String(
							    "uniform float2       uFoamRange,\n") +
							    "uniform float        uFoamMaxDistance,\n" +
	                            "uniform float        uFoamScale,\n" +
	                            "uniform float        uFoamStart,\n" +
	                            "uniform float        uFoamTransparency,\n";
						}
						if (cCaustics)
						{
							ShaderStr += Ogre::String(
							   "uniform float        uCausticsPower,\n") +
	                           "uniform float        uCausticsEnd,\n";
						}

						int TexNum = 0;

						if (Options.NM == NM_TEXTURE)
						{
						    ShaderStr += 
							   "uniform sampler2D    uNormalMap       : register(s" + Ogre::StringConverter::toString(TexNum) + "),\n";
							TexNum++;
						}

						ShaderStr += 
							Ogre::String(
						       "uniform sampler2D    uReflectionMap   : register(s" + Ogre::StringConverter::toString(TexNum) + "),\n") +
	                           "uniform sampler2D    uRefractionMap   : register(s" + Ogre::StringConverter::toString(TexNum+1) + "),\n";

						TexNum += 2;

						if (cDepth)
						{
							ShaderStr += 
								"uniform sampler2D    uDepthMap        : register(s" + Ogre::StringConverter::toString(TexNum) + "),\n";
							TexNum++;
						}

						ShaderStr += 
								"uniform sampler1D    uFresnelMap      : register(s" + Ogre::StringConverter::toString(TexNum) + ")";
						TexNum++;

						if (cFoam)
						{
							ShaderStr += Ogre::String(
							 ",\nuniform sampler2D    uFoamMap         : register(s" + Ogre::StringConverter::toString(TexNum) + ")\n");
						}

						ShaderStr += 
							Ogre::String(                                            ")\n") +
							"{\n"     +
							    "float2 ProjectionCoord = iUvProjection.xy / iUvProjection.w;\n" +
                                "float3 camToSurface = iPosition.xyz - uEyePosition;\n" +
                                "float additionalReflection=camToSurface.x*camToSurface.x+camToSurface.z*camToSurface.z;\n";

						if (cFoam)
						{
							// Calculate the foam visibility as a function fo distance specified by user
							ShaderStr +=
								"float foamVisibility=1.0f-saturate(additionalReflection/uFoamMaxDistance);\n";
						}

						ShaderStr += 
							Ogre::String(
							    "additionalReflection/=uFullReflectionDistance;\n") +
								"camToSurface=normalize(-camToSurface);\n";
						if (Options.NM == NM_TEXTURE)
						{
							ShaderStr += Ogre::String(
								"float3 pixelNormal = tex2D(uNormalMap,iUvNoise);\n") +
								// Inverte y with z, because at creation our local normal to the plane was z
								"pixelNormal.yz=pixelNormal.zy;\n" +
								// Remap from [0,1] to [-1,1]
								"pixelNormal.xyz=expand(pixelNormal.xyz);\n";
						}
						else
						{
							ShaderStr += 
								"float3 pixelNormal = iNormal;\n";
						}
						ShaderStr += 
								"float2 pixelNormalModified = uNormalDistortion*pixelNormal.zx;\n";
						if (Options.NM == NM_TEXTURE)
						{
							ShaderStr +=
								"float dotProduct=dot(camToSurface,pixelNormal);\n";
						}
						else
						{
							ShaderStr +=
								"float dotProduct=dot(-camToSurface,pixelNormal);\n";
						}
						ShaderStr += 
							Ogre::String(
								"dotProduct=saturate(dotProduct);\n") +
								"float fresnel = tex1D(uFresnelMap,dotProduct);\n" +
								// Add additional reflection and saturate
								"fresnel+=additionalReflection;\n" +
								"fresnel=saturate(fresnel);\n" +
								// Decrease the transparency and saturate
								"fresnel-=uGlobalTransparency;\n" +
                                "fresnel=saturate(fresnel);\n" +
								// Get the reflection/refraction pixels. Make sure to disturb the texcoords by pixelnormal
								"float4 reflection=tex2D(uReflectionMap,ProjectionCoord.xy+pixelNormalModified);\n" +
								"float4 refraction=tex2D(uRefractionMap,ProjectionCoord.xy-pixelNormalModified);\n";

						if (cDepth)
						{
							if (cCaustics)
						    {
								ShaderStr += Ogre::String(
								"float2 depth = tex2D(uDepthMap,ProjectionCoord.xy-pixelNormalModified).rg;\n") +
								"refraction *= 1+saturate((uCausticsEnd-depth.x))*depth.y*uCausticsPower;\n" +
								"refraction = lerp(refraction,float4(uDepthColor,1),1-depth.x);\n";
						    }
							else
							{
								ShaderStr += Ogre::String(
								"float depth = 1.0f - tex2D(uDepthMap,ProjectionCoord.xy-pixelNormalModified).r;\n") +
								"refraction = lerp(refraction,float4(uDepthColor,1),depth);\n";
							}

							ShaderStr += 
								"normalize(refraction);\n";
						}

						ShaderStr += 
								"oColor = lerp(refraction,reflection,fresnel);\n";

						if (cSun)
						{
							ShaderStr += Ogre::String(
							    "float3 relfectedVector = normalize(reflect(-camToSurface,pixelNormal.xyz));\n") +
								"float3 surfaceToSun=normalize(uSunPosition-iPosition.xyz);\n" +
								"float3 sunlight = uSunStrength*pow(saturate(dot(relfectedVector,surfaceToSun)),uSunArea)*uSunColor;\n" +
								"oColor.xyz+=sunlight;\n";
						}

						if (cFoam)
						{
							ShaderStr += Ogre::String(
							    "float hmap = ((iPosition.y - uFoamRange.x)/uFoamRange.y)*foamVisibility;\n") +
								"float2 foamTex=iPosition.xz*uFoamScale+pixelNormalModified;\n" +
								"float foam=tex2D(uFoamMap,foamTex).r;\n" +
								"float foamTransparency=saturate(hmap-uFoamStart)*uFoamTransparency;\n" +
								"oColor=lerp(oColor,float4(1,1,1,1),foamTransparency*foam);\n";
						}

						if (cSmooth)
						{
							if (cCaustics)
							{
								ShaderStr += "oColor.w = saturate((1-depth.x)*uSmoothPower);\n";
							}
							else
							{
								ShaderStr += "oColor.w = saturate(depth*uSmoothPower);\n";
							}
						}

						ShaderStr +=
							"}\n";
					}
					break;

					case SM_GLSL:
					{}
					break;
				}
		    }
		    break;

			case NM_RTT:
		    {}
		    break;
		}

		Ogre::HighLevelGpuProgramPtr FP = 
			 Ogre::HighLevelGpuProgramManager::getSingleton().
			       createProgram(_def_Water_Shader_FP_Name, 
			                     HYDRAX_RESOURCE_GROUP, 
					  		     ShaderModesStr[static_cast<int>(Options.SM)], 
			                     Ogre::GPT_FRAGMENT_PROGRAM);

	    FP->setSource(ShaderStr);
        FP->setParameter("entry_point", "main_fp");
        FP->setParameter(Profiles[0], Profiles[1]);
		FP->load();

		// Second: build our material
		Ogre::MaterialPtr &WaterMaterial = getMaterial(MAT_WATER);
		WaterMaterial = Ogre::MaterialManager::getSingleton().
			create(_def_Water_Material_Name,
			       HYDRAX_RESOURCE_GROUP);

		Ogre::Pass *WM_Technique0_Pass0 = WaterMaterial->getTechnique(0)->getPass(0);

		if (cDepth)
		{
		    WM_Technique0_Pass0->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
		}
		WM_Technique0_Pass0->setDepthWriteEnabled(true);

		WM_Technique0_Pass0->setVertexProgram(_def_Water_Shader_VP_Name);
		WM_Technique0_Pass0->setFragmentProgram(_def_Water_Shader_FP_Name);

		Ogre::GpuProgramParametersSharedPtr VP_Parameters = WM_Technique0_Pass0->getVertexProgramParameters();
		Ogre::GpuProgramParametersSharedPtr FP_Parameters = WM_Technique0_Pass0->getFragmentProgramParameters();

		VP_Parameters->setNamedAutoConstant("uWorldViewProj", Ogre::GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
		FP_Parameters->setNamedAutoConstant("uEyePosition",   Ogre::GpuProgramParameters::ACT_CAMERA_POSITION_OBJECT_SPACE);

		FP_Parameters->setNamedConstant("uFullReflectionDistance", mHydrax->getFullReflectionDistance());
		FP_Parameters->setNamedConstant("uGlobalTransparency",     mHydrax->getGlobalTransparency());
		FP_Parameters->setNamedConstant("uNormalDistortion",       mHydrax->getNormalDistortion());

		if (cDepth)
		{
			FP_Parameters->setNamedConstant("uDepthColor", mHydrax->getDepthColor());
		}
		if (cSmooth)
		{
			FP_Parameters->setNamedConstant("uSmoothPower", mHydrax->getSmoothPower());
		}
		if (cSun)
		{
			FP_Parameters->setNamedConstant("uSunPosition", mHydrax->getSunPosition());
			FP_Parameters->setNamedConstant("uSunStrength", mHydrax->getSunStrength());
			FP_Parameters->setNamedConstant("uSunArea",     mHydrax->getSunArea());
			FP_Parameters->setNamedConstant("uSunColor",    mHydrax->getSunColor());
		}
		if (cFoam)
		{
			float FRange[2] = {mHydrax->getPosition().y, mHydrax->getStrength()};
			FP_Parameters->setNamedConstant("uFoamRange",        FRange, 1, 2);
			FP_Parameters->setNamedConstant("uFoamMaxDistance",  mHydrax->getFoamMaxDistance());
			FP_Parameters->setNamedConstant("uFoamScale",        mHydrax->getFoamScale());
			FP_Parameters->setNamedConstant("uFoamStart",        mHydrax->getFoamStart());
			FP_Parameters->setNamedConstant("uFoamTransparency", mHydrax->getFoamTransparency());
		}
		if (cCaustics)
		{
			FP_Parameters->setNamedConstant("uCausticsPower", mHydrax->getCausticsPower());
			FP_Parameters->setNamedConstant("uCausticsEnd",   mHydrax->getCausticsEnd());
		}

		if (Options.NM == NM_TEXTURE)
		{
		    WM_Technique0_Pass0->createTextureUnitState("HydraxNormalMap")->setTextureAddressingMode(Ogre::TextureUnitState::TAM_WRAP);
		}

		WM_Technique0_Pass0->createTextureUnitState("Reflection")->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
		WM_Technique0_Pass0->createTextureUnitState("Refraction")->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);

		if (cDepth)
		{
			WM_Technique0_Pass0->createTextureUnitState("Depth")->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
		}

		WM_Technique0_Pass0->createTextureUnitState("Fresnel.bmp")->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);

		if (cFoam)
		{
			WM_Technique0_Pass0->createTextureUnitState("Foam.png")->setTextureAddressingMode(Ogre::TextureUnitState::TAM_WRAP);
		}

		WaterMaterial->load();

		return true;
	}

	bool MaterialManager::_createDepthMaterial(const HydraxComponent &Components, const Options &Options)
	{
		Ogre::String ShaderStr = "";
		Ogre::String ShaderModesStr[3] = {"hlsl", "cg", "glsl"};
		Ogre::String Profiles[2];

		const bool cCaustics = _isComponent(Components, HYDRAX_COMPONENT_CAUSTICS);

		// First build our shader
		// Vertex program:

		switch (Options.SM)
		{
		    case SM_HLSL:
			{
				Profiles[0] = "target";
				Profiles[1] = "vs_1_1";
			}
			break;

			case SM_CG:
			{
				Profiles[0] = "profiles";
				Profiles[1] = "vs_1_1 arbvp1";
			}
			break;

			case SM_GLSL:
			{}
			break;
		}

		switch (Options.SM)
		{
		    case SM_HLSL: case SM_CG:
			{
				// No caustics
				if (!cCaustics)
				{
				    ShaderStr += 
					    Ogre::String(
					    "void main_vp(\n") +
					        // IN
					        "float4 iPosition         : POSITION,\n" +
						    // OUT 
						    "out float4 oPosition     : POSITION,\n" +
						    "out float  oPosition_    : TEXCOORD0,\n" +
						    // UNIFORM
						    "uniform float            uPlaneYPos,\n" +
                            "uniform float4x4         uWorld,\n" +
                            "uniform float4x4         uWorldViewProj)\n" +
					    "{\n" +
					       "oPosition = mul(uWorldViewProj, iPosition);\n" +
						   "oPosition_ = mul(uWorld, iPosition).y;\n" +
						   "oPosition_-=uPlaneYPos;\n" +
						"}\n";
				}
				else // Caustics
				{
					ShaderStr += 
					    Ogre::String(
					    "void main_vp(\n") +
					        // IN
					        "float4 iPosition         : POSITION,\n" +
						    // OUT 
						    "out float4 oPosition     : POSITION,\n" +
						    "out float  oPosition_    : TEXCOORD0,\n" +
							"out float2 oUvWorld      : TEXCOORD1,\n" +
						    // UNIFORM
						    "uniform float            uPlaneYPos,\n" +
                            "uniform float4x4         uWorld,\n" +
                            "uniform float4x4         uWorldViewProj)\n" +
					    "{\n" +
					       "oPosition = mul(uWorldViewProj, iPosition);\n" +
						   "oPosition_ = mul(uWorld, iPosition).y;\n" +
						   "oPosition_-=uPlaneYPos;\n" +
						   "oUvWorld = float2(iPosition.x, iPosition.z);\n" +
						"}\n";
				}
			}
			break;

			case SM_GLSL:
			{}
			break;
		}

		Ogre::HighLevelGpuProgramPtr VP = 
			Ogre::HighLevelGpuProgramManager::getSingleton().
			      createProgram(_def_Depth_Shader_VP_Name, 
			                    HYDRAX_RESOURCE_GROUP, 
					  		    ShaderModesStr[static_cast<int>(Options.SM)], 
			                    Ogre::GPT_VERTEX_PROGRAM);

	    VP->setSource(ShaderStr);
        VP->setParameter("entry_point", "main_vp");
        VP->setParameter(Profiles[0], Profiles[1]);
		VP->load();

		// Fragment program
		ShaderStr = "";

		switch (Options.SM)
		{
		    case SM_HLSL:
			{
				Profiles[0] = "target";
				Profiles[1] = "ps_2_0";
			}
			break;

			case SM_CG:
			{
				Profiles[0] = "profiles";
				Profiles[1] = "ps_2_0 arbfp1 fp20";
			}
			break;

			case SM_GLSL:
			{}
			break;
		}

		switch (Options.SM)
		{
		    case SM_HLSL: case SM_CG:
			{
				// No caustics
				if (!cCaustics)
				{
					ShaderStr += 
						Ogre::String(
						"void main_fp(\n") +
						    // IN
						    "float  iPosition     : TEXCOORD0,\n" +
							// OUT
							"out float4 oColor    : COLOR,\n" +
							// UNIFORM
							"uniform float        uDepthLimit)\n" +
						"{\n" +
						    "float pixelYDepth = (iPosition*uDepthLimit+1);\n" +
							"pixelYDepth = saturate(pixelYDepth);\n" +
							"oColor = float4(pixelYDepth,pixelYDepth,pixelYDepth,pixelYDepth);\n" +
						"}\n";
				}
				else // Caustics
				{
					ShaderStr += 
						Ogre::String(
						"void main_fp(\n") +
						    // IN
						    "float  iPosition     : TEXCOORD0,\n" +
                            "float2 iUvWorld      : TEXCOORD1,\n" +
							// OUT
							"out float4 oColor    : COLOR,\n" +
							// UNIFORM
							"uniform float        uDepthLimit,\n" +
							"uniform float        uCausticsScale,\n" +
							"uniform sampler2D    uCaustics : register(s0))\n" +
						"{\n" +
						    "float pixelYDepth = (iPosition*uDepthLimit+1);\n" +
							"pixelYDepth = saturate(pixelYDepth);\n" +
							"oColor = float4(pixelYDepth,pixelYDepth,pixelYDepth,0);\n" +
							"oColor.g = tex2D(uCaustics, iUvWorld/uCausticsScale).r;\n" +
						"}\n";
				}
			}
			break;

			case SM_GLSL:
			{}
			break;
		}

		Ogre::HighLevelGpuProgramPtr FP = 
			 Ogre::HighLevelGpuProgramManager::getSingleton().
			       createProgram(_def_Depth_Shader_FP_Name, 
			                     HYDRAX_RESOURCE_GROUP, 
					  		     ShaderModesStr[static_cast<int>(Options.SM)], 
			                     Ogre::GPT_FRAGMENT_PROGRAM);

	    FP->setSource(ShaderStr);
        FP->setParameter("entry_point", "main_fp");
        FP->setParameter(Profiles[0], Profiles[1]);
		FP->load();

		// Second: build our material
		Ogre::MaterialPtr &DepthMaterial = getMaterial(MAT_DEPTH);
		DepthMaterial = Ogre::MaterialManager::getSingleton().
			create(_def_Depth_Material_Name,
			       HYDRAX_RESOURCE_GROUP);

		DepthMaterial->getTechnique(0)->setSchemeName("HydraxDepth");

		Ogre::Pass *DM_Technique0_Pass0 = DepthMaterial->getTechnique(0)->getPass(0);

		DM_Technique0_Pass0->setVertexProgram(_def_Depth_Shader_VP_Name);
		DM_Technique0_Pass0->setFragmentProgram(_def_Depth_Shader_FP_Name);

		Ogre::GpuProgramParametersSharedPtr VP_Parameters = DM_Technique0_Pass0->getVertexProgramParameters();
		Ogre::GpuProgramParametersSharedPtr FP_Parameters = DM_Technique0_Pass0->getFragmentProgramParameters();

		VP_Parameters->setNamedAutoConstant("uWorldViewProj", Ogre::GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
		VP_Parameters->setNamedAutoConstant("uWorld", Ogre::GpuProgramParameters::ACT_WORLD_MATRIX);
		VP_Parameters->setNamedConstant("uPlaneYPos", mHydrax->getPosition().y);
		
		FP_Parameters->setNamedConstant("uDepthLimit", 1/mHydrax->getDepthLimit());

		if (cCaustics)
		{
			FP_Parameters->setNamedConstant("uCausticsScale", mHydrax->getCausticsScale());

			Ogre::TextureUnitState *TUS_Caustics = DM_Technique0_Pass0->createTextureUnitState("Caustics.bmp");
			TUS_Caustics->setTextureAddressingMode(Ogre::TextureUnitState::TAM_WRAP);
			TUS_Caustics->setAnimatedTextureName("Caustics.bmp", 32, 1.5);
		}

		DepthMaterial->load();

		return true;
	}

	void MaterialManager::reload(const MaterialType &Material)
	{
		Ogre::MaterialPtr &Mat = getMaterial(Material);

		if (Mat.isNull())
		{
			return;
		}

		Mat->reload();

		const bool cDepth    = _isComponent(mComponents, HYDRAX_COMPONENT_DEPTH   );
		const bool cSmooth   = _isComponent(mComponents, HYDRAX_COMPONENT_SMOOTH  );
		const bool cSun      = _isComponent(mComponents, HYDRAX_COMPONENT_SUN     );
		const bool cFoam     = _isComponent(mComponents, HYDRAX_COMPONENT_FOAM    );
		const bool cCaustics = _isComponent(mComponents, HYDRAX_COMPONENT_CAUSTICS);

		switch (Material)
		{
		    case MAT_WATER:
			{
				Ogre::Pass *M_Technique0_Pass0 = Mat->getTechnique(0)->getPass(0);

				switch (mOptions.NM)
				{
				    case NM_TEXTURE:
					{
						M_Technique0_Pass0->getTextureUnitState(0)->setTextureName("HydraxNormalMap");
						M_Technique0_Pass0->getTextureUnitState(1)->setTextureName("HydraxReflectionMap");
						M_Technique0_Pass0->getTextureUnitState(2)->setTextureName("HydraxRefractionMap");
						M_Technique0_Pass0->getTextureUnitState(3)->setTextureName("HydraxDepthMap");
					}
					break;

					case NM_VERTEX:
					{
						M_Technique0_Pass0->getTextureUnitState(0)->setTextureName("HydraxReflectionMap");
						M_Technique0_Pass0->getTextureUnitState(1)->setTextureName("HydraxRefractionMap");
						M_Technique0_Pass0->getTextureUnitState(2)->setTextureName("HydraxDepthMap");
					}
					break;

					case NM_RTT:
					{
					}
					break;
				}

				Ogre::GpuProgramParametersSharedPtr VP_Parameters = M_Technique0_Pass0->getVertexProgramParameters();
				Ogre::GpuProgramParametersSharedPtr FP_Parameters = M_Technique0_Pass0->getFragmentProgramParameters();

				VP_Parameters->setNamedAutoConstant("uWorldViewProj", Ogre::GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
				FP_Parameters->setNamedAutoConstant("uEyePosition",   Ogre::GpuProgramParameters::ACT_CAMERA_POSITION_OBJECT_SPACE);

				FP_Parameters->setNamedConstant("uFullReflectionDistance", mHydrax->getFullReflectionDistance());
				FP_Parameters->setNamedConstant("uGlobalTransparency",     mHydrax->getGlobalTransparency());
				FP_Parameters->setNamedConstant("uNormalDistortion",       mHydrax->getNormalDistortion());

				if (cDepth)
				{
					FP_Parameters->setNamedConstant("uDepthColor", mHydrax->getDepthColor());
				}
				if (cSmooth)
				{
					FP_Parameters->setNamedConstant("uSmoothPower", mHydrax->getSmoothPower());
				}
				if (cSun)
				{
					FP_Parameters->setNamedConstant("uSunPosition", mHydrax->getSunPosition());
					FP_Parameters->setNamedConstant("uSunStrength", mHydrax->getSunStrength());
					FP_Parameters->setNamedConstant("uSunArea",     mHydrax->getSunArea());
					FP_Parameters->setNamedConstant("uSunColor",    mHydrax->getSunColor());
				}
				if (cFoam)
				{
					float FRange[2] = {mHydrax->getPosition().y, mHydrax->getStrength()};
			        FP_Parameters->setNamedConstant("uFoamRange",        FRange, 1, 2);
					FP_Parameters->setNamedConstant("uFoamMaxDistance",  mHydrax->getFoamMaxDistance());
					FP_Parameters->setNamedConstant("uFoamScale",        mHydrax->getFoamScale());
					FP_Parameters->setNamedConstant("uFoamStart",        mHydrax->getFoamStart());
					FP_Parameters->setNamedConstant("uFoamTransparency", mHydrax->getFoamTransparency());
				}
				if (cCaustics)
				{
					FP_Parameters->setNamedConstant("uCausticsPower", mHydrax->getCausticsPower());
					FP_Parameters->setNamedConstant("uCausticsEnd",   mHydrax->getCausticsEnd());
				}
				
			}
			break;

			case MAT_DEPTH:
			{
				Ogre::Pass *M_Technique0_Pass0 = Mat->getTechnique(0)->getPass(0);

				Ogre::GpuProgramParametersSharedPtr VP_Parameters = M_Technique0_Pass0->getVertexProgramParameters();
				Ogre::GpuProgramParametersSharedPtr FP_Parameters = M_Technique0_Pass0->getFragmentProgramParameters();

				VP_Parameters->setNamedAutoConstant("uWorldViewProj", Ogre::GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
				VP_Parameters->setNamedAutoConstant("uWorld", Ogre::GpuProgramParameters::ACT_WORLD_MATRIX);
				VP_Parameters->setNamedConstant("uPlaneYPos", mHydrax->getPosition().y);

				FP_Parameters->setNamedConstant("uDepthLimit", 1/mHydrax->getDepthLimit());

				if (cCaustics)
				{
					FP_Parameters->setNamedConstant("uCausticsScale", mHydrax->getCausticsScale());
				}
			}
			break;
		}
	}

	void MaterialManager::addDepthTechnique(Ogre::MaterialPtr &Material, const int &Index)
	{
		if (!Ogre::MaterialManager::getSingleton().resourceExists(_def_Depth_Material_Name))
		{
			_createDepthMaterial(mComponents, mOptions);
		}

		Material->getTechnique(Index)->removeAllPasses();
		Material->getTechnique(Index)->setSchemeName("HydraxDepth");

		Ogre::Pass *DM_Technique0_Pass0 = Material->getTechnique(Index)->getPass(0);

		DM_Technique0_Pass0->setVertexProgram(_def_Depth_Shader_VP_Name);
		DM_Technique0_Pass0->setFragmentProgram(_def_Depth_Shader_FP_Name);

		Ogre::GpuProgramParametersSharedPtr VP_Parameters = DM_Technique0_Pass0->getVertexProgramParameters();
		Ogre::GpuProgramParametersSharedPtr FP_Parameters = DM_Technique0_Pass0->getFragmentProgramParameters();

		VP_Parameters->setNamedAutoConstant("uWorldViewProj", Ogre::GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
		VP_Parameters->setNamedAutoConstant("uWorld", Ogre::GpuProgramParameters::ACT_WORLD_MATRIX);
		VP_Parameters->setNamedConstant("uPlaneYPos", mHydrax->getPosition().y);
		
		FP_Parameters->setNamedConstant("uDepthLimit", 1/mHydrax->getDepthLimit());

		if (_isComponent(mComponents, HYDRAX_COMPONENT_CAUSTICS))
		{
			FP_Parameters->setNamedConstant("uCausticsScale", mHydrax->getCausticsScale());

			Ogre::TextureUnitState *TUS_Caustics = DM_Technique0_Pass0->createTextureUnitState("Caustics.bmp");
			TUS_Caustics->setTextureAddressingMode(Ogre::TextureUnitState::TAM_WRAP);
			TUS_Caustics->setAnimatedTextureName("Caustics.bmp", 32, 1.5);
		}
	}

	bool MaterialManager::_isComponent(const HydraxComponent &List, const HydraxComponent &ToCheck) const
	{
		if (List & ToCheck)
        {
            return true;
        }

        if (ToCheck == HYDRAX_COMPONENTS_NONE && List == HYDRAX_COMPONENTS_NONE)
        {
            return true;
        }

        if (ToCheck == HYDRAX_COMPONENTS_ALL && List == HYDRAX_COMPONENTS_ALL)
        {
            return true;
        }

        return false;
	}

	void MaterialManager::setGpuProgramParameter(const GpuProgram &GpuP, const MaterialType &MType, const Ogre::String &Name, const Ogre::Real &Value)
	{
		if (!isCreated())
		{
			return;
		}

		Ogre::GpuProgramParametersSharedPtr Parameters;

		switch (GpuP)
		{
		    case GPUP_VERTEX:
			{
				Parameters = getMaterial(MType)->getTechnique(0)->getPass(0)->getVertexProgramParameters();
			}
			break;

			case GPUP_FRAGMENT:
			{
				Parameters = getMaterial(MType)->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
			}
			break;
		}

		Parameters->setNamedConstant(Name, Value);
	}

	void MaterialManager::setGpuProgramParameter(const GpuProgram &GpuP, const MaterialType &MType, const Ogre::String &Name, const Ogre::Vector2 &Value)
	{
		if (!isCreated())
		{
			return;
		}

		Ogre::GpuProgramParametersSharedPtr Parameters;

		switch (GpuP)
		{
		    case GPUP_VERTEX:
			{
				Parameters = getMaterial(MType)->getTechnique(0)->getPass(0)->getVertexProgramParameters();
			}
			break;

			case GPUP_FRAGMENT:
			{
				Parameters = getMaterial(MType)->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
			}
			break;
		}

		float Value_[2] = {Value.x, Value.y};

		Parameters->setNamedConstant(Name, Value_, 1, 2);
	}

	void MaterialManager::setGpuProgramParameter(const GpuProgram &GpuP, const MaterialType &MType, const Ogre::String &Name, const Ogre::Vector3 &Value)
	{
		if (!isCreated())
		{
			return;
		}

		Ogre::GpuProgramParametersSharedPtr Parameters;

		switch (GpuP)
		{
		    case GPUP_VERTEX:
			{
				Parameters = getMaterial(MType)->getTechnique(0)->getPass(0)->getVertexProgramParameters();
			}
			break;

			case GPUP_FRAGMENT:
			{
				Parameters = getMaterial(MType)->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
			}
			break;
		}

		Parameters->setNamedConstant(Name, Value);
	}
}