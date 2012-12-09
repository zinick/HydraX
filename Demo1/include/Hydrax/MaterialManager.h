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

#ifndef _Hydrax_MaterialManager_H_
#define _Hydrax_MaterialManager_H_

#include "Prerequisites.h"

#include "Enums.h"

namespace Hydrax
{
	class Hydrax;

	/** Material/Shader manager class
	 */
	class DllExport MaterialManager
	{
	public:
		/** Material type enum
		    @remarks Use in getMaterial(MaterialType)
		 */
		enum MaterialType
		{
			// Water material
			MAT_WATER = 0,
			// Depth material
			MAT_DEPTH = 1
		};

		/** Gpu program enum
		    @remarks Use in setGpuProgramParameter()
		 */
		enum GpuProgram
		{
			// Vertex program
			GPUP_VERTEX   = 0,
			// Fragment program
			GPUP_FRAGMENT = 1
		};

		/** Shader mode
		 */
		enum ShaderMode
		{
			// HLSL
			SM_HLSL = 0,
			// Cg
			SM_CG   = 1,
			// GLSL
			SM_GLSL = 2
		};

		/** Normal generation mode
		 */
		enum NormalMode
		{
			// Normal map from precomputed texture(CPU)
			NM_TEXTURE = 0,
			// Normal map from vertex(CPU)
			NM_VERTEX  = 1,
			// Normal map from RTT(GPU)
			NM_RTT     = 2
		};

		/** Material options
		 */
		struct Options
		{
			/** Default constructor
			 */
			Options()
				: SM(SM_HLSL)
				, NM(NM_TEXTURE)
			{
			}

			/** Constructor
			    @param _SM Shader mode
				@param _NM Normal generation mode
			 */
			Options(const ShaderMode &_SM,
				    const NormalMode &_NM)
				: SM(_SM)
				, NM(_NM)
			{
			}

			/// Shader mode
			ShaderMode SM;
			/// Normal map generation mode
			NormalMode NM;
		};

		/** Constructor
		    @param h Hydrax pointer
		 */
		MaterialManager(Hydrax *h);

		/** Destructor
		 */
        ~MaterialManager();

		/** Create materials
		    @param Components Components of the shader
			@param Options Material options
		 */
		bool createMaterials(const HydraxComponent &Components, const Options &Options);

		/** Remove materials
		 */
		void removeMaterials();

		/** Reload material
		    @param Material Material to reload
		 */
		void reload(const MaterialType &Material);

		/** Is createMaterials() already called?
		    @return true If yes
		 */
		inline const bool& isCreated() const
		{
			return mCreated;
		}

		/** Get material
		    @param Material Material to get
			@return Material to get
		 */
		inline Ogre::MaterialPtr &getMaterial(const MaterialType &Material)
		{
			return mMaterials[static_cast<int>(Material)];
		}

		/** Get the last MaterialManager::Options used in a material generation
		    @return Last MaterialManager::Options used in a material generation
		 */
		inline const Options &getLastOptions() const
		{
			return mOptions;
		}

		/** Add depth technique to a especified material
		    @param Material Material where depth technique will be added
			@param Index Depth technique creation index
			@remarks Call it after Hydrax::create() !!!

			         Add depth technique when a material is not an Ogre::Entity, such 
			         a terrains, PLSM2 materials, etc.
					 This depth technique will be added with "Depth" scheme in order
					 to can use it in the Depth RTT.
					 It will be update automatically with params updates.
		 */
		void addDepthTechnique(Ogre::MaterialPtr &Material, const int &Index);

		/** Set gpu program Ogre::Real parameter
		    @param GpuP Gpu program type (Vertex/Fragment)
			@param MType Water/Depth material
			@param Name param name
			@param Value value
		 */
		void setGpuProgramParameter(const GpuProgram &GpuP, const MaterialType &MType, const Ogre::String &Name, const Ogre::Real &Value);

		/** Set gpu program Ogre::Vector2 parameter
		    @param GpuP Gpu program type (Vertex/Fragment)
			@param MType Water/Depth material
			@param Name param name
			@param Value value
		 */
		void setGpuProgramParameter(const GpuProgram &GpuP, const MaterialType &MType, const Ogre::String &Name, const Ogre::Vector2 &Value); 

		/** Set gpu program Ogre::Vector3 parameter
		    @param GpuP Gpu program type (Vertex/Fragment)
			@param MType Water/Depth material
			@param Name param name
			@param Value value
		 */
		void setGpuProgramParameter(const GpuProgram &GpuP, const MaterialType &MType, const Ogre::String &Name, const Ogre::Vector3 &Value); 

	private: 
		/** Is component in the given list?
		    @param List Components list
			@param ToCheck Component to check
		    @return true if the component is in the given list.
		 */
		bool _isComponent(const HydraxComponent &List, const HydraxComponent &ToCheck) const;

		/** Create water material
		    @param Components Components of the shader
			@param Options Material options
		 */
		bool _createWaterMaterial(const HydraxComponent &Components, const Options &Options);

		/** Create depth material
		    @param Components Components of the shader
			@param Options Material options
		 */
		bool _createDepthMaterial(const HydraxComponent &Components, const Options &Options);

		/// Is createMaterials() already called?
		bool mCreated;
		/// Hydrax materials vector
		Ogre::MaterialPtr mMaterials[2];
		/// Actual material components
		HydraxComponent mComponents;
		/// Actual material options
		Options mOptions;
		/// Hydrax main pointer
		Hydrax *mHydrax;
	};
};

#endif