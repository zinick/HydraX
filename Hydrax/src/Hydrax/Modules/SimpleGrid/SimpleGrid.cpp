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

#include "SimpleGrid.h"

namespace Hydrax{namespace Module
{
	SimpleGrid::SimpleGrid(Hydrax *h, Noise::Noise *n)
		: Module("SimpleGrid", n, Mesh::Options(mOptions.Complexity, Size(mOptions.MeshSize), Mesh::VT_POS_NORM), MaterialManager::NM_VERTEX)
		, mHydrax(h)
		, mVertices(0)
	{
	}

	SimpleGrid::SimpleGrid(Hydrax *h, Noise::Noise *n, const Options &Options)
		: Module("SimpleGrid", n, Mesh::Options(Options.Complexity, Size(Options.MeshSize), Mesh::VT_POS_NORM), MaterialManager::NM_VERTEX)
		, mHydrax(h)
		, mVertices(0)
	{
		setOptions(Options);
	}

	SimpleGrid::~SimpleGrid()
	{
		if (mVertices)
		{
		    delete [] mVertices;
		}

		HydraxLOG(getName() + " destroyed.");
	}

	void SimpleGrid::setOptions(const Options &Options)
	{
		mMeshOptions.MeshSize     = Options.MeshSize;
		mMeshOptions.MeshStrength = Options.Strength;

		mHydrax->getMesh()->setOptions(mMeshOptions);
		mHydrax->_setStrength(mOptions.Strength);

		// If create() is called, change only the on-fly parameters
		if (isCreated())
		{
			mOptions.MeshSize  = Options.MeshSize;
			mOptions.Strength  = Options.Strength;
			mOptions.Smooth    = Options.Smooth;

			int v, u;
			for(v=0; v<mOptions.Complexity; v++)
			{
				for(u=0; u<mOptions.Complexity; u++)
				{
					mVertices[v*mOptions.Complexity + u].x  = (static_cast<float>(v)/(mOptions.Complexity-1)) * mOptions.MeshSize.Width;
					mVertices[v*mOptions.Complexity + u].z  = (static_cast<float>(u)/(mOptions.Complexity-1)) * mOptions.MeshSize.Height;
				}
			}	

			return;
		} 

		mOptions = Options;
	}

	void SimpleGrid::create()
	{
		HydraxLOG("Creating " + getName() + " module.");

		Module::create();

		mVertices = new Mesh::POS_NORM_VERTEX[mOptions.Complexity*mOptions.Complexity];	

		int v, u;
		for(v=0; v<mOptions.Complexity; v++)
		{
			for(u=0; u<mOptions.Complexity; u++)
			{
				mVertices[v*mOptions.Complexity + u].x  = (static_cast<float>(v)/(mOptions.Complexity-1)) * mOptions.MeshSize.Width;
				mVertices[v*mOptions.Complexity + u].z  = (static_cast<float>(u)/(mOptions.Complexity-1)) * mOptions.MeshSize.Height;
			}
		}	

		HydraxLOG(getName() + " created.");
	}

	void SimpleGrid::update(const Ogre::Real &timeSinceLastFrame)
	{
		if (!isCreated())
		{
			return;
		}

		Module::update(timeSinceLastFrame);

		// Update heigths
		int i = 0, v, u;
		float v_;

		for(v=0; v<mOptions.Complexity; v++)
		{
			v_ = (static_cast<float>(v)/(mOptions.Complexity-1)) * mOptions.MeshSize.Width;

			for(u=0; u<mOptions.Complexity; u++)
		    {
			    mVertices[i].y  = 
				    mNoise->getValue(v_,
					                 (static_cast<float>(u)/(mOptions.Complexity-1)) * mOptions.MeshSize.Height) * mOptions.Strength;
				i++;
			}
		}

		// Smooth the heightdata
		if (mOptions.Smooth)
		{
			for(v=1; v<(mOptions.Complexity-1); v++)
			{
				for(u=1; u<(mOptions.Complexity-1); u++)
				{				
					mVertices[v*mOptions.Complexity + u].y =	
						0.2f *
					   (mVertices[v    *mOptions.Complexity + u    ].y +
						mVertices[v    *mOptions.Complexity + (u+1)].y + 
						mVertices[v    *mOptions.Complexity + (u-1)].y + 
						mVertices[(v+1)*mOptions.Complexity + u    ].y + 
						mVertices[(v-1)*mOptions.Complexity + u    ].y);															
				}
			}
		}
		
		// Update normals
		Ogre::Vector3 vec1, vec2, normal;

		for(v=1; v<(mOptions.Complexity-1); v++)
		{
			for(u=1; u<(mOptions.Complexity-1); u++)
			{
				vec1 = Ogre::Vector3(
					mVertices[v*mOptions.Complexity + u + 1].x-mVertices[v*mOptions.Complexity + u - 1].x,
					mVertices[v*mOptions.Complexity + u + 1].y-mVertices[v*mOptions.Complexity + u - 1].y, 
					mVertices[v*mOptions.Complexity + u + 1].z-mVertices[v*mOptions.Complexity + u - 1].z);

				vec2 = Ogre::Vector3(
					mVertices[(v+1)*mOptions.Complexity + u].x - mVertices[(v-1)*mOptions.Complexity + u].x,
					mVertices[(v+1)*mOptions.Complexity + u].y - mVertices[(v-1)*mOptions.Complexity + u].y,
					mVertices[(v+1)*mOptions.Complexity + u].z - mVertices[(v-1)*mOptions.Complexity + u].z);

				normal = vec2.crossProduct(vec1);

				mVertices[v*mOptions.Complexity + u].nx = normal.x;
				mVertices[v*mOptions.Complexity + u].ny = normal.y;
				mVertices[v*mOptions.Complexity + u].nz = normal.z;
			}
		}

		// Upload geometry changes
		mHydrax->getMesh()->updateGeometry(mOptions.Complexity*mOptions.Complexity, mVertices);
	}

	float SimpleGrid::getHeigth(const Ogre::Vector2 &Position)
	{
		Ogre::Vector2 RelativePos = mHydrax->getMesh()->getGridPosition(Position);

		RelativePos.x *= mOptions.MeshSize.Width;
		RelativePos.y *= mOptions.MeshSize.Height;

		return mHydrax->getPosition().y + mNoise->getValue(RelativePos.x, RelativePos.y)*mOptions.Strength;
	}
}}