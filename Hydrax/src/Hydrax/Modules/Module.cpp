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

#include "Module.h"

namespace Hydrax{namespace Module
{
	Module::Module(const Ogre::String &Name, const Mesh::Type &MeshGeometrySupportedTypes, const MaterialManager::NormalMode &NormalMode)
		: mName(Name) 
		, mMeshGeomtrySupportedTypes(MeshGeometrySupportedTypes)
		, mNormalMode(NormalMode)
	    , mCreated(false)
	{
	}

	Module::~Module()
	{
	}

	void Module::create()
	{
		mCreated = true;
	}

	void Module::saveCfg(Ogre::String &Data)
	{
		Data += "#Module options\n";
		Data += "Module="+mName+"\n\n";
	}

	bool Module::loadCfg(Ogre::ConfigFile &CfgFile)
	{
		if (CfgFile.getSetting("Module") == mName)
		{
			return true;
		}

		return false;
	}

	float Module::getHeigth(const Ogre::Vector2 &Position)
	{
		return -1;
	}
}}