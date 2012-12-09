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

#ifndef _Hydrax_Module_H_
#define _Hydrax_Module_H_

#include "../Prerequisites.h"
#include "../Mesh.h"
#include "../MaterialManager.h"

namespace Hydrax{ namespace Module
{
	/** Base module class, 
	    Override it for create different ways of create water noise.
	 */
	class DllExport Module
	{
	public:
		/** Constructor
		    @param Name Module name
			@param MeshGeometrySupportedTypes Mesh::Type grids supported
			@param NormalMode Normal generation mode
		 */
		Module(const Ogre::String &Name, const Mesh::Type &MeshGeometrySupportedTypes, const MaterialManager::NormalMode &NormalMode);

		/** Destructor
		 */
		virtual ~Module();

		/** Create
		    @remark Not forgot call in the override class
		 */
		virtual void create();

		/** Call it each frame
		    @param timeSinceLastFrame Time since last frame(delta)
		 */
		virtual void update(const Ogre::Real &timeSinceLastFrame) = 0;

		/** Save config
		    @param Data String reference 
		 */
		virtual void saveCfg(Ogre::String &Data);

		/** Load config
		    @param CgfFile Ogre::ConfigFile reference 
			@return True if is the correct module config
		 */
		virtual bool loadCfg(Ogre::ConfigFile &CfgFile);

		/** Get module name
		    @return Module name
		 */
		inline const Ogre::String& getName() const
		{
			return mName;
		}

		/** Is created() called?
		    @return true if created() have been already called
		 */
		inline const bool& isCreated() const
		{
			return mCreated;
		}

		/** Is this type of grid geometry supported?
		    @param Type Mesh::Type to check
		    @return true if it's supported.
		 */
		inline bool isMeshGeometrySupported(const Mesh::Type &Type) const
		{
			if (mMeshGeomtrySupportedTypes & Type)
			{
				return true;
			}

			if (Type == Mesh::NONE && mMeshGeomtrySupportedTypes == Mesh::NONE)
			{
				return true;
			}

			if (Type == Mesh::ALL && mMeshGeomtrySupportedTypes == Mesh::ALL)
			{
				return true;
			}

			return false;
		}

		/** Get the normal generation mode
		    @return Module normal generation mode
		 */
		inline const MaterialManager::NormalMode& getNormalMode() const
		{
			return mNormalMode;
		}

		/** Get the current heigth at a especified world-space point
		    @param Position X/Z World position
			@return Heigth at the given position in y-World coordinates, if it's outside of the water return -1
		 */
		virtual float getHeigth(const Ogre::Vector2 &Position);

	private:
		/// Module name
		Ogre::String mName;
		/// Types of grid geomtry that supports
		Mesh::Type mMeshGeomtrySupportedTypes;
		/// Normal map generation mode
		MaterialManager::NormalMode mNormalMode;
		/// Is create() called?
		bool mCreated;
	};
}}

#endif