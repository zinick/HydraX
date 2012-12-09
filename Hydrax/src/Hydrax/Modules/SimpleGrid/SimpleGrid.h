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

#ifndef _Hydrax_Modules_SimpleGrid_H_
#define _Hydrax_Modules_SimpleGrid_H_

#include "../../Prerequisites.h"

#include "../../Hydrax.h"
#include "../../Mesh.h"
#include "../Module.h"

namespace Hydrax{ namespace Module
{
	/** Hydrax simple grid module
	 */
	class DllExport SimpleGrid : public Module
	{
	public:
		/** Struct wich contains Hydrax simple grid module options
		 */
		struct Options
		{
			/// Projected grid complexity (N*N)
			int Complexity;
			/// Size
			Size MeshSize;
			/// Strength
			float Strength;
			/// Smooth
			bool Smooth;

			/** Default constructor
			 */
			Options()
				: Complexity(256)
				, MeshSize(Size(100))
				, Strength(32.5f)
				, Smooth(false)
			{
			}

			/** Constructor
			    @param _Complexity Projected grid complexity
				@param _MeshSize Water mesh size
			 */
			Options(const int &_Complexity, 
				    const Size &_MeshSize)
				: Complexity(_Complexity)
				, MeshSize(_MeshSize)
				, Strength(32.5f)
				, Smooth(false)
			{
			}

			/** Constructor
			    @param _Complexity Projected grid complexity
				@param _MeshSize Water mesh size
				@param _Strength Perlin noise strength
				@param _Smooth Smooth vertex?
			 */
			Options(const int   &_Complexity,
				    const Size  &_MeshSize,
				    const float &_Strength,
					const bool  &_Smooth)
				: Complexity(_Complexity)
				, MeshSize(_MeshSize)
				, Strength(_Strength)
				, Smooth(_Smooth)
			{
			}
		};

		/** Constructor
		    @param h Hydrax manager pointer
			@param n Hydrax noise module
		 */
		SimpleGrid(Hydrax *h, Noise::Noise *n);

		/** Constructor
		    @param h Hydrax manager pointer
			@param n Hydrax noise module
			@param Options Perlin options
		 */
		SimpleGrid(Hydrax *h, Noise::Noise *n, const Options &Options);

		/** Destructor
		 */
        ~SimpleGrid();

		/** Create
		 */
		void create();

		/** Call it each frame
		    @param timeSinceLastFrame Time since last frame(delta)
		 */
		void update(const Ogre::Real &timeSinceLastFrame);

		/** Set options
		    @param Options Options
		 */
		void setOptions(const Options &Options);

		/** Save config
		    @param Data String reference 
		 */
	//	void saveCfg(Ogre::String &Data);

		/** Load config
		    @param CgfFile Ogre::ConfigFile reference 
			@return True if is the correct module config
		 */
	//	bool loadCfg(Ogre::ConfigFile &CfgFile);

		/** Get the current heigth at a especified world-space point
		    @param Position X/Z World position
			@return Heigth at the given position in y-World coordinates, if it's outside of the water return -1
		 */
		float getHeigth(const Ogre::Vector2 &Position);

		/** Get current options
		    @return Current options
		 */
		inline const Options& getOptions() const
		{
			return mOptions;
		}

	private:
		/// Vertex pointer
		Mesh::POS_NORM_VERTEX *mVertices;

		/// Our projected grid options
		Options mOptions;

		/// Our Hydrax pointer
		Hydrax* mHydrax;
	};
}}

#endif