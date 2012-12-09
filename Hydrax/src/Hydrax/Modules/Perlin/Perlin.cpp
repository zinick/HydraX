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

#include "Perlin.h"

namespace Hydrax{namespace Module
{
	Perlin::Perlin(Hydrax *h)
		: Module("PerlinModule", static_cast<Mesh::Type>(Mesh::SIMPLE_GRID | Mesh::IMANTED_GRID), MaterialManager::NM_TEXTURE)
		, mHydrax(h)
		, mCurrentBlendScroll(0)
	{
		for(int k = 0; k < 3; k++)
		{
			mHeigthMap[k] = 0;
			mNormalMap[k] = 0;
		}
	}

	Perlin::Perlin(Hydrax *h, const Options &Options)
		: Module("PerlinModule", static_cast<Mesh::Type>(Mesh::SIMPLE_GRID | Mesh::IMANTED_GRID), MaterialManager::NM_TEXTURE)
		, mHydrax(h)
		, mCurrentBlendScroll(0)
	{
		for(int k = 0; k < 3; k++)
		{
			mHeigthMap[k] = 0;
			mNormalMap[k] = 0;
		}

		setOptions(Options);
	}

	Perlin::~Perlin()
	{
		for(int k = 0; k < 3; k++)
		{
			if (mHeigthMap[k])
			{
				delete mHeigthMap[k];
			}

			if (mNormalMap[k])
			{
				delete mNormalMap[k];
			}
		}

		HydraxLOG(getName() + " destroyed.");
	}

	void Perlin::setOptions(const Options &Options)
	{
		mOptions = Options;

		mPerlinModule.SetFrequency(mOptions.Frecuency);
        mPerlinModule.SetPersistence(mOptions.Persistence);
        mPerlinModule.SetOctaveCount(mOptions.Octave);
        mPerlinModule.SetLacunarity(mOptions.Lacunarity);

		// If create() is called, recreate textures on the fly
		if (isCreated())
		{
			_createImages();

			// Reload our shader(needed to use new heigth/normal textures in our shader)
			mHydrax->getMaterialManager()->reload(MaterialManager::MAT_WATER);
		}
	}

	void Perlin::create()
	{
		HydraxLOG("Creating " + getName() + " module.");

		Module::create();

		_createImages();

		HydraxLOG(getName() + " created.");
	}

	void Perlin::update(const Ogre::Real &timeSinceLastFrame)
	{
		if (!isCreated())
		{
			return;
		}

		// Update(Blend) maps
	    _blendImages(timeSinceLastFrame);

		// Update Ogre textures
		mHydrax->getTextureManager()->update(TextureManager::TEX_NORMAL_ID, *mNormalMap[2]);

		// Update mesh
		mHydrax->getMesh()->update(*mHeigthMap[2]);
	}

	void Perlin::_createImages()
	{
		HydraxLOG("Creating hydrax texture manager, Textures size: " + Ogre::StringConverter::toString(static_cast<int>(mOptions.TexQuality)));
		mHydrax->getTextureManager()->create(Size(static_cast<int>(mOptions.TexQuality)));
		HydraxLOG("Hydrax texture manager created.");

		for(int k = 0; k < 3; k++)
		{
			if (mHeigthMap[k])
			{
				delete mHeigthMap[k];
			}

			if (mNormalMap[k])
			{
				delete mNormalMap[k];
			}
		}

		for(int k = 0; k < 2; k++)
		{
			mPerlinModule.SetSeed(k);

		    mHeigthMap[k] = _getHeigthMap();
		    mNormalMap[k] = _getNormalMap();
		}

		mHeigthMap[2] = new Image(Size(static_cast<int>(mOptions.TexQuality)), Image::TYPE_ONE_CHANNEL);
		mNormalMap[2] = new Image(Size(static_cast<int>(mOptions.TexQuality)), Image::TYPE_RGB);
	}

	Image* Perlin::_getHeigthMap()
	{
        noise::utils::NoiseMapBuilderPlane constructor;
        noise::utils::NoiseMap noiseMap;

		int MapSize = static_cast<int>(mOptions.TexQuality);

        constructor.SetBounds(-1.0, 1.0, -1.0, 1.0);
        constructor.SetDestSize(MapSize,MapSize);
        constructor.EnableSeamless(true);
        constructor.SetSourceModule(mPerlinModule);
        constructor.SetDestNoiseMap(noiseMap);
        constructor.Build();

		Image* HeigthMap = new Image(Size(MapSize), Image::TYPE_ONE_CHANNEL);

        for (int x = 0; x < MapSize; x++)
        {
            for (int y = 0; y < MapSize; y++)
            {
                // Convert a [-1,1] range double to [0,1] range float
                float To01 = (1+static_cast<float>(noiseMap.GetValue(x, y)))/2;

                if (To01 > 1)
                {
                    To01 = 1;
                }
                else if (To01 < 0)
                {
                    To01 = 0;
                }

				HeigthMap->setValue(x,y,0,To01);
            }
        }

        return HeigthMap;
	}

	Image* Perlin::_getNormalMap()
	{
		noise::utils::NoiseMapBuilderPlane constructor;
        noise::utils::NoiseMap noiseMap;

		int MapSize = static_cast<int>(mOptions.TexQuality);

        constructor.SetBounds(-1.0, 1.0, -1.0, 1.0);
        constructor.SetDestSize(MapSize, MapSize);
        constructor.EnableSeamless(true);
        constructor.SetSourceModule(mPerlinModule);
        constructor.SetDestNoiseMap(noiseMap);
        constructor.Build();

        noise::utils::RendererNormalMap mRenderer;
        noise::utils::Image NImage;
        NImage.SetSize(MapSize, MapSize);
        mRenderer.EnableWrap(true);
        mRenderer.SetBumpHeight(mOptions.NMHeight);
        mRenderer.SetSourceNoiseMap(noiseMap);
        mRenderer.SetDestImage(NImage);
        mRenderer.Render();

		Image* NormalMap = new Image(Size(MapSize), Image::TYPE_RGB);

		for (int x = 0; x < MapSize; x++)
        {
            for (int y = 0; y < MapSize; y++)
            {
				NormalMap->setValue(x,y,0, NImage.GetValue(x,y).red);
				NormalMap->setValue(x,y,1, NImage.GetValue(x,y).green);
				NormalMap->setValue(x,y,2, NImage.GetValue(x,y).blue);
            }
        }

        return NormalMap;
	}

	void Perlin::_blendImages(const Ogre::Real &timeSinceLastFrame)
	{
		int x, y, newY1, newY1a, newY2, newY2a, CurrentBlendScrollInt = static_cast<int>(mCurrentBlendScroll);
        float a, b, c, d, diff, diffb;
		Image::Pixel PixelA, PixelB, PixelAA, PixelBB, p;

        diff  = (mCurrentBlendScroll - CurrentBlendScrollInt)/2; // Divide by 2 to avoid it in: mNoiseBuffer[2].mData[x][y] = (c+d)/2;
        diffb = 0.5 - diff;

		// We asume that mHeigthMap[2] and mNormalMap[2] sizes are equals
		Size MapsSize = mHeigthMap[2]->getSize();

        for (y = 0; y < MapsSize.Height; y ++)
        {
            newY1 = y + CurrentBlendScrollInt;
            if (newY1>MapsSize.Height-1)
            {
                newY1-=MapsSize.Height;
            }

            newY1a = (y+1) + CurrentBlendScrollInt;
            if (newY1a>MapsSize.Height-1)
            {
                newY1a-=MapsSize.Height;
            }

            newY2 = y - CurrentBlendScrollInt;
            if (newY2<0)
            {
                newY2+=MapsSize.Height-1;
            }

            newY2a = (y+1) - CurrentBlendScrollInt;
            if (newY2a<0)
            {
                newY2a+=MapsSize.Height-1;
            }
            if (newY2a>MapsSize.Height-1)
            {
                newY2a-=MapsSize.Height;
            }

            for (x = 0; x < MapsSize.Width; x ++)
            {
				// Todo, try too with x,newY1

				// Heigth maps ---->
                a = mHeigthMap[0]->getValue(newY1,  x, 0); 
                b = mHeigthMap[0]->getValue(newY1a, x, 0); 

                c=a*diffb+ b*diff;

                a = mHeigthMap[1]->getValue(newY2,  x, 0); 
                b = mHeigthMap[1]->getValue(newY2a, x, 0); 

                d=a*diff + b*diffb;

                mHeigthMap[2]->setValue(y, x, 0, (c+d));
				// ----
				
				// Normal maps ---->
				// We don't use getPixel() in order to optimice fps
				PixelA  = Image::Pixel(mNormalMap[0]->getValue(newY2,  x, 0),
					                   mNormalMap[0]->getValue(newY2,  x, 1),
									   mNormalMap[0]->getValue(newY2,  x, 2));

				PixelAA = Image::Pixel(mNormalMap[0]->getValue(newY2a, x, 0),
					                   mNormalMap[0]->getValue(newY2a, x, 1),
									   mNormalMap[0]->getValue(newY2a, x, 2));

				PixelB  = Image::Pixel(mNormalMap[1]->getValue(newY1,  x, 0),
					                   mNormalMap[1]->getValue(newY1,  x, 1),
									   mNormalMap[1]->getValue(newY1,  x, 2));

				PixelBB = Image::Pixel(mNormalMap[1]->getValue(newY1a, x, 0),
					                   mNormalMap[1]->getValue(newY1a, x, 1),
									   mNormalMap[1]->getValue(newY1a, x, 2));

				p = 
				    Image::Pixel(
                        // Red
                        (( PixelB.red   *diffb + PixelBB.red   *diff )
                         +(PixelA.red   *diff  + PixelAA.red   *diffb)),
                        // Green
                        (( PixelB.green *diffb + PixelBB.green *diff )
                         +(PixelA.green *diff  + PixelAA.green *diffb)),
                        // Blue
                        (( PixelB.blue  *diffb + PixelBB.blue  *diff )
                         +(PixelA.blue  *diff  + PixelAA.blue  *diffb)),
                        // Alpha
                        255);

				mNormalMap[2]->setPixel(y,x,p); 
            }
        }

        mCurrentBlendScroll += mOptions.Velocity*timeSinceLastFrame;

        if (mCurrentBlendScroll>MapsSize.Height)
        {
            mCurrentBlendScroll = 0;
        }
	}

	void Perlin::saveCfg(Ogre::String &Data)
	{
		Module::saveCfg(Data);

		Data += "TextureQuality=" +Ogre::StringConverter::toString(static_cast<int>(mOptions.TexQuality))+"\n";
		Data += "Frecuency="      +Ogre::StringConverter::toString(static_cast<Ogre::Real>(mOptions.Frecuency))+"\n";
		Data += "Persistence="    +Ogre::StringConverter::toString(static_cast<int>(mOptions.Persistence))+"\n";
		Data += "Octave="         +Ogre::StringConverter::toString(mOptions.Octave)+"\n";
		Data += "Lacunarity="     +Ogre::StringConverter::toString(static_cast<Ogre::Real>(mOptions.Lacunarity))+"\n";
		Data += "NMHeight="       +Ogre::StringConverter::toString(static_cast<Ogre::Real>(mOptions.NMHeight))+"\n";
		Data += "Velocity="       +Ogre::StringConverter::toString(static_cast<Ogre::Real>(mOptions.Velocity))+"\n";
	}

	bool Perlin::loadCfg(Ogre::ConfigFile &CfgFile)
	{
		if (!Module::loadCfg(CfgFile))
		{
			return false;
		}

		setOptions(
			Options(// Texture quality(Size)
			        static_cast<TextureQuality>(Ogre::StringConverter::parseInt(CfgFile.getSetting("TextureQuality"))),
					// Perlin freq.
					Ogre::StringConverter::parseReal(CfgFile.getSetting("Frecuency")),
					// Persistance
					Ogre::StringConverter::parseReal(CfgFile.getSetting("Persistence")),
					// Octave
					Ogre::StringConverter::parseInt(CfgFile.getSetting("Octave")),
					// Lacunarity
					Ogre::StringConverter::parseReal(CfgFile.getSetting("Lacunarity")),
					// Normal map heigth(power)
					Ogre::StringConverter::parseReal(CfgFile.getSetting("NMHeight")),
					// Heigth map blend velocity
					Ogre::StringConverter::parseReal(CfgFile.getSetting("Velocity"))));

		return true;
	}

	float Perlin::getHeigth(const Ogre::Vector2 &Position)
	{
		if (!mHeigthMap[2])
		{
			return -1;
		}

		Ogre::Vector2 RelativePos = mHydrax->getMesh()->getGridPosition(Position);

		return mHeigthMap[2]->getValueLI(RelativePos.x * mHeigthMap[2]->getSize().Width,
			                             RelativePos.y * mHeigthMap[2]->getSize().Height,
									     0) * mHydrax->getStrength(); 
	}
}}