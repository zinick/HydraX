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

#ifndef _Hydrax_TextureManager_H_
#define _Hydrax_TextureManager_H_

#include "Prerequisites.h"

#include "Enums.h"
#include "Structs.h"
#include "Image.h"

namespace Hydrax
{
	class Hydrax;

	/** Class for manager Height and Normal textures.
	 */
	class DllExport TextureManager
	{
	public:
		/** Textures enumeration
		 */
		enum TexturesID
		{
			// Height map
			TEX_HEIGHT_ID = 0,
			// Normal map
			TEX_NORMAL_ID = 1
		};

		/** Constructor
		    @param h Hydrax main pointer
		 */
		TextureManager(Hydrax* h);

		/** Destructor
		 */
		~TextureManager();

		/** Create height and normal map textures
		    @param Size Textures's size
		 */
		void create(const Size &Size);

		/** Update
		    @param Id Texture's ID
			@param Image Update image
			@return false if something fails
			@remarks If you need to update the texture with another way of data,
			         get the Ogre::TexturePtr and modify it directly.
					 Heigth image will be Image::Type::TYPE_ONE_CHANNEL [0,1] range
					 Normal image will be Image::Type::TYPE_RGB [0,255] range
		 */
		inline bool update(const TexturesID &Id, Image &Image)
		{
			if (Id == TEX_HEIGHT_ID)
			{
				return _updateHeigthMap(Image);
			}
			else if (Id == TEX_NORMAL_ID)
			{
				return _updateNormalMap(Image);
			}

			return false;
		}

		/** Destroy textures
		 */
		void destroy();

		/** Get texture
		    @param Id Texture Id ( Height / Normal )
			@return Ogre::TexturePtr
		 */
		inline Ogre::TexturePtr& getTexture(const TexturesID &Id)
		{
			return mTextures[static_cast<int>(Id)];
		}

		/** Get texture's name
		    @param Id Texture Id ( Height / Normal )
			@return Texture's name
		 */
		inline const Ogre::String& getTextureName(const TexturesID &Id) const
		{
			return mTextureNames[static_cast<int>(Id)];
		}

	private:
		/** Create an Ogre::Texture
		    @param Texture Our Ogre::TexturePtr
		    @param Name Texture's name
			@param Size Texture's size
			@return false if there is a problem. 
		 */
		bool _createTexture(Ogre::TexturePtr &Texture, const Ogre::String &Name, const Size &Size);

		/** Get an Ogre::TexturePtr
		    @param Name Texture's name
			@return Our texture
		 */
		Ogre::TexturePtr _getTexture(const Ogre::String &Name);

		/** Update heigth map
		    @param Image Update image
			@return false if something fails
			@remarks Image type will be Image::Type::TYPE_ONE_CHANNEL
		*/
        bool _updateHeigthMap(Image &Image);

		/** Update normal map
		    @param Image Update image
			@return false if something fails
			@remarks Image type will be Image::Type::TYPE_RGB
		*/
        bool _updateNormalMap(Image &Image);

		/// Our Ogre::TexturePtr array
        Ogre::TexturePtr mTextures[2];
		/// Our Ogre::String array for store texture's names
		Ogre::String mTextureNames[2];

        /// Have been created already called?
        bool mCreated;

		/// Hydrax main pointer
		Hydrax *mHydrax;
	};
}

#endif