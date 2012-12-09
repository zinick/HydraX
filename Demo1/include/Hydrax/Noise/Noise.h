/*
--------------------------------------------------------------------------------
This source file is part of Hydrax.
Visit ---

Copyright (C) 2008 Xavier Verguín González <xavierverguin@hotmail.com>
                                           <xavyiy@gmail.com>

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
--------------------------------------------------------------------------------
*/

#ifndef _Hydrax_Noise_H_
#define _Hydrax_Noise_H_

#include "../Prerequisites.h"

namespace Hydrax{ namespace Noise
{
	/** Base noise class, 
	    Override it for create different ways of create water noise.
	 */
	class DllExport Noise
	{
	public:
		/** Constructor
		    @param Name Noise name
		 */
		Noise(const Ogre::String &Name);

		/** Destructor
		 */
		virtual ~Noise();

		/** Create
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
			@return True if is the correct noise config
		 */
		virtual bool loadCfg(Ogre::ConfigFile &CfgFile);

		/** Get noise name
		    @return Noise name
		 */
		inline const Ogre::String& getName() const
		{
			return mName;
		}

		/** Is created() called?
		    @return true if create() have been already called
		 */
		inline const bool& isCreated() const
		{
			return mCreated;
		}

		/** Get the especified x/y noise value
		    @param x X Coord
			@param y Y Coord
			@return Noise value
		 */
		virtual float getValue(const float &x, const float &y) = 0;

	protected:
		/// Module name
		Ogre::String mName;
		/// Is create() called?
		bool mCreated;
	};
}}

#endif