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

#ifndef _Hydrax_Modules_Perlin_H_
#define _Hydrax_Modules_Perlin_H_

#include "../../Prerequisites.h"

// TODO: Copy it to Modules/Perlin path
#include "../../../noise/noise.h"
#include "../../../noise/noiseutils.h"

#include "../../Hydrax.h"
#include "../Module.h"

namespace Hydrax{ namespace Module
{
	class DllExport Perlin : public Module
	{
	public:
		/** Struct wich contains Hydrax perlin module options
		 */
		struct Options
		{
			/// Our height and normal map texture sizes
			TextureQuality TexQuality;
			/// Perlin noise frecuency value
			double Frecuency;
			/// Perlin noise persistence value
			double Persistence;
			/// Perlin noise octave value
			int Octave;
			/// Perlin noise lacunarity value
			double Lacunarity;
			/// Normal map height value
			double NMHeight;
			/// Velocity factor
			double Velocity;

			/** Default constructor
			 */
			Options()
				: TexQuality(TEX_QUA_128)
				, Frecuency(5)
				, Persistence(0.7)
				, Octave(1)
				, Lacunarity(1)
				, NMHeight(1)
				, Velocity(9.75)
			{
			}

			/** Constructor
			    @param _TextureQuality Height and normal map texture sizes
			    @param _Frecuency Perlin noise frecuency value
			    @param _Persistence Perlin noise persistence value
			    @param _Octave Perlin noise octave value
			    @param _Lacunarity Perlin noise lacunarity value
			    @param _NMHeight Normal map height value
				@param _Velocity Velocity factor
			 */
			Options(const TextureQuality  &_TextureQuality,
					const double          &_Frecuency,
					const double          &_Persistence,
					const int             &_Octave,
					const double          &_Lacunarity,
					const double          &_NMHeight,
					const double          &_Velocity)
				: TexQuality(_TextureQuality)
				, Frecuency(_Frecuency)
				, Persistence(_Persistence)
				, Octave(_Octave)
				, Lacunarity(_Lacunarity)
				, NMHeight(_NMHeight)
				, Velocity(_Velocity)
			{
			}
		};

		/** Constructor
		    @param h Hydrax manager pointer
		 */
		Perlin(Hydrax *h);

		/** Constructor
		    @param h Hydrax manager pointer
			@param Options Perlin options
		 */
		Perlin(Hydrax *h, const Options &Options);

		/** Destructor
		 */
        ~Perlin();

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
		void saveCfg(Ogre::String &Data);

		/** Load config
		    @param CgfFile Ogre::ConfigFile reference 
			@return True if is the correct module config
		 */
		bool loadCfg(Ogre::ConfigFile &CfgFile);

		/** Get the current heigth at a especified world-space point
		    @param Position X/Z World position
			@return Heigth at the given position in y-World coordinates, if it's outside of the water return -1
		 */
		float getHeigth(const Ogre::Vector2 &Position);

	private:
		/** Create heigth/normal maps
		 */
		void _createImages();

		/** Blend images
		    @param timeSinceLastFrame Time since last frame(delta)
		 */
		void _blendImages(const Ogre::Real &timeSinceLastFrame);

		/** Get an Hydrax::Image pointer from actual module height map
		    @return Hydrax::Image*
		 */
		Image* _getHeigthMap();

		/** Get an Hydrax::Image pointer from actual module normal map
		    @return Hydrax::Image*
		 */
		Image* _getNormalMap();

		/// Our perlin noise options
		Options mOptions;
		/// Our noise::module::Perlin for generate data
        noise::module::Perlin mPerlinModule;

		/** For our module we use a simple method:
		    2 Different height map images and their normal map.
			We blend it appling a time decal.
			Third image is for store the resultant image of the blending.

			Images:
			0 - Heigth/Normal image for map 1
			1 - Heigth/Normal image for map 2
			2 - Result of blending images 1 and 2.
		 */
		/// Hydrax::Image Heigth map vector
		Image *mHeigthMap[3];
		/// Hydrax::Image Normal map vector
		Image *mNormalMap[3];

		/// Current blend scroll position
		float mCurrentBlendScroll;

		// Our Hydrax pointer
		Hydrax* mHydrax;
	};
}}

#endif