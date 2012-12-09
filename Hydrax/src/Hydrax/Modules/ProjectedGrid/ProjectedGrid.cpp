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

Based on the Projected Grid concept from Claes Johanson thesis:
http://graphics.cs.lth.se/theses/projects/projgrid/
and Ren Cheng Ogre3D implementation:
http://www.cnblogs.com/ArenAK/archive/2007/11/07/951713.html 
--------------------------------------------------------------------------------
*/

#include "ProjectedGrid.h"

#define _def_MaxFarClipDistance 99999

namespace Hydrax{namespace Module
{
	ProjectedGrid::ProjectedGrid(Hydrax *h, Noise::Noise *n, const Ogre::Plane &BasePlane)
		: Module("ProjectedGrid", n, Mesh::Options(mOptions.Complexity, Size(0), Mesh::VT_POS_NORM), MaterialManager::NM_VERTEX)
		, mHydrax(h)
		, mVertices(0)
		, mBasePlane(BasePlane)
		, mNormal(BasePlane.normal)
		, mPos(Ogre::Vector3(0,0,0))
		, mProjectingCamera(0)
		, mRenderingCamera(h->getCamera())
	{
	}

	ProjectedGrid::ProjectedGrid(Hydrax *h, Noise::Noise *n, const Ogre::Plane &BasePlane, const Options &Options)
		: Module("ProjectedGrid", n, Mesh::Options(Options.Complexity, Size(0), Mesh::VT_POS_NORM), MaterialManager::NM_VERTEX)
		, mHydrax(h)
		, mVertices(0)
		, mBasePlane(BasePlane)
		, mNormal(BasePlane.normal)
		, mPos(Ogre::Vector3(0,0,0))
		, mProjectingCamera(0)
		, mRenderingCamera(h->getCamera())
	{
		setOptions(Options);
	}

	ProjectedGrid::~ProjectedGrid()
	{
		if (mVertices)
		{
		    delete [] mVertices;
		}

		if (mProjectingCamera)
		{
			delete mProjectingCamera;
		}

		HydraxLOG(getName() + " destroyed.");
	}

	void ProjectedGrid::setOptions(const Options &Options)
	{
		// Size(0) -> Infinite mesh
		mMeshOptions.MeshSize     = Size(0);
		mMeshOptions.MeshStrength = Options.Strength;

		mHydrax->getMesh()->setOptions(mMeshOptions);
		mHydrax->_setStrength(mOptions.Strength);

		// If create() is called, change only the on-fly parameters
		if (isCreated())
		{
			mOptions.Strength  = Options.Strength;
			mOptions.Elevation = Options.Elevation;
			mOptions.Smooth    = Options.Smooth;

			return;
		} 

		mOptions = Options;
	}

	void ProjectedGrid::create()
	{
		HydraxLOG("Creating " + getName() + " module.");

		Module::create();

		mVertices = new Mesh::POS_NORM_VERTEX[mOptions.Complexity*mOptions.Complexity];	

	    _setDisplacementAmplitude(0.0f);

		HydraxLOG(getName() + " created.");
	}

	void ProjectedGrid::update(const Ogre::Real &timeSinceLastFrame)
	{
		if (!isCreated())
		{
			return;
		}

		Module::update(timeSinceLastFrame);

		if (mLastPosition    != mRenderingCamera->getPosition()    || 
			mLastOrientation != mRenderingCamera->getOrientation() ||
			mOptions.ForceRecalculateGeometry)
		{
			float RenderingFarClipDistance = mRenderingCamera->getFarClipDistance();

		    if (RenderingFarClipDistance > _def_MaxFarClipDistance)
		    {
			    mRenderingCamera->setFarClipDistance(_def_MaxFarClipDistance);
		    }

			mLastMinMax = _getMinMax(&mRange);

		    if (mLastMinMax)
		    {
			    _renderGeometry(mRange,mProjectingCamera->getViewMatrix());

			    mHydrax->getMesh()->updateGeometry(mOptions.Complexity*mOptions.Complexity, mVertices);
		    }

			mRenderingCamera->setFarClipDistance(RenderingFarClipDistance);
		}
		else if (mLastMinMax)
		{
			int i = 0, v, u;

		    for(v=0; v<mOptions.Complexity; v++)
		    {
			    for(u=0; u<mOptions.Complexity; u++)
			    {				
				    mVertices[i].y = -mBasePlane.d + mNoise->getValue(mVertices[i].x, mVertices[i].z)*mOptions.Strength;

				    i++;
			    }
		    }

			_calculeNormals();

			mHydrax->getMesh()->updateGeometry(mOptions.Complexity*mOptions.Complexity, mVertices);
		}

		mLastPosition = mRenderingCamera->getPosition();
		mLastOrientation = mRenderingCamera->getOrientation();
	}

	bool ProjectedGrid::_renderGeometry(const Ogre::Matrix4& m,const Ogre::Matrix4& _viewMat)
	{
		t_corners0 = _calculeWorldPosition(Ogre::Vector2( 0.0f, 0.0f),m,_viewMat);
		t_corners1 = _calculeWorldPosition(Ogre::Vector2(+1.0f, 0.0f),m,_viewMat);
		t_corners2 = _calculeWorldPosition(Ogre::Vector2( 0.0f,+1.0f),m,_viewMat);
		t_corners3 = _calculeWorldPosition(Ogre::Vector2(+1.0f,+1.0f),m,_viewMat);
	
		float du  = 1.0f/(mOptions.Complexity-1),
			  dv  = 1.0f/(mOptions.Complexity-1),
			  u,v = 0.0f,
			  // _1_u = (1.0f-u)
			  _1_u, _1_v = 1.0f,
			  divide;

		Ogre::Vector4 result;

		int i = 0, iv, iu;

		for(iv=0; iv<mOptions.Complexity; iv++)
		{
			u = 0.0f;	
			_1_u = 1.0f;
			for(iu=0; iu<mOptions.Complexity; iu++)
			{				
				result.x = _1_v*(_1_u*t_corners0.x + u*t_corners1.x) + v*(_1_u*t_corners2.x + u*t_corners3.x);				
				result.z = _1_v*(_1_u*t_corners0.z + u*t_corners1.z) + v*(_1_u*t_corners2.z + u*t_corners3.z);				
				result.w = _1_v*(_1_u*t_corners0.w + u*t_corners1.w) + v*(_1_u*t_corners2.w + u*t_corners3.w);				

				divide = 1.0f/result.w;				
				result.x *= divide;
				result.z *= divide;

				mVertices[i].x = result.x;
				mVertices[i].z = result.z;
				mVertices[i].y = -mBasePlane.d + mNoise->getValue(result.x, result.z)*mOptions.Strength;

				i++;
				u += du;
				_1_u = 1.0f-u;
			}
			v += dv;
			_1_v = 1.0f-v;
		}

		// Smooth the heightdata
		if (mOptions.Smooth)
		{
			for(iv=1; iv<(mOptions.Complexity-1); iv++)
			{
				for(iu=1; iu<(mOptions.Complexity-1); iu++)
				{				
					mVertices[iv*mOptions.Complexity + iu].y =	
						0.2f *
					   (mVertices[iv    *mOptions.Complexity + iu    ].y +
						mVertices[iv    *mOptions.Complexity + (iu+1)].y + 
						mVertices[iv    *mOptions.Complexity + (iu-1)].y + 
						mVertices[(iv+1)*mOptions.Complexity + iu    ].y + 
						mVertices[(iv-1)*mOptions.Complexity + iu    ].y);															
				}
			}
		}

		_calculeNormals();

		return true;
	}

	void ProjectedGrid::_calculeNormals()
	{
		int v, u;
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
	}

	// Check the point of intersection with the plane (0,1,0,0) and return the position in homogenous coordinates 
	Ogre::Vector4 ProjectedGrid::_calculeWorldPosition(const Ogre::Vector2 &uv, const Ogre::Matrix4& m, const Ogre::Matrix4& _viewMat)
	{	
		Ogre::Vector4 origin(uv.x,uv.y,-1,1);
		Ogre::Vector4 direction(uv.x,uv.y,1,1);

		origin = m*origin;
		direction = m*direction;

		Ogre::Vector3 _org(origin.x/origin.w,origin.y/origin.w,origin.z/origin.w);
		Ogre::Vector3 _dir(direction.x/direction.w,direction.y/direction.w,direction.z/direction.w);
		_dir -= _org;
		_dir.normalise();

		Ogre::Ray _ray(_org,_dir);
		std::pair<bool,Ogre::Real> _result = _ray.intersects(mBasePlane);
		float l = _result.second;
		Ogre::Vector3 worldPos = _org + _dir*l;
		Ogre::Vector4 _tempVec = _viewMat*Ogre::Vector4(worldPos);
		float _temp = -_tempVec.z/_tempVec.w;
		Ogre::Vector4 retPos(worldPos);
		retPos /= _temp;

		return retPos;
	}

	bool ProjectedGrid::_getMinMax(Ogre::Matrix4 *range)
	{
		_setDisplacementAmplitude(mOptions.Strength);

		float x_min,y_min,x_max,y_max;
		Ogre::Vector3 frustum[8],proj_points[24];

		int i,
			n_points = 0,
			src, dst;

		int cube[] = 
		   {0,1,	0,2,	2,3,	1,3,
		    0,4,	2,6,	3,7,	1,5,
		    4,6,	4,5,	5,7,	6,7};

		Ogre::Vector3 _testLine;
		Ogre::Real _dist;
		Ogre::Ray _ray;

		std::pair<bool,Ogre::Real> _result;

		Ogre::Matrix4 invviewproj = (mRenderingCamera->getProjectionMatrixWithRSDepth()*mRenderingCamera->getViewMatrix()).inverse();
		frustum[0] = invviewproj * Ogre::Vector3(-1,-1,0);
		frustum[1] = invviewproj * Ogre::Vector3(+1,-1,0);
		frustum[2] = invviewproj * Ogre::Vector3(-1,+1,0);
		frustum[3] = invviewproj * Ogre::Vector3(+1,+1,0);
		frustum[4] = invviewproj * Ogre::Vector3(-1,-1,+1);
		frustum[5] = invviewproj * Ogre::Vector3(+1,-1,+1);
		frustum[6] = invviewproj * Ogre::Vector3(-1,+1,+1);
		frustum[7] = invviewproj * Ogre::Vector3(+1,+1,+1);

		// Check intersections with upper_bound and lower_bound	
		for(i=0; i<12; i++)
		{
			src=cube[i*2]; dst=cube[i*2+1];
			_testLine = frustum[dst]-frustum[src];
			_dist = _testLine.normalise();
			_ray = Ogre::Ray(frustum[src], _testLine);
			_result = Ogre::Math::intersects(_ray,mUpperBoundPlane);
			if ((_result.first) && (_result.second<_dist+0.00001))
			{
				proj_points[n_points++] = frustum[src] + _result.second * _testLine;
			}
			_result = Ogre::Math::intersects(_ray,mLowerBoundPlane);
			if ((_result.first) && (_result.second<_dist+0.00001))
			{
				proj_points[n_points++] = frustum[src] + _result.second * _testLine;
			}
		}

		// Check if any of the frustums vertices lie between the upper_bound and lower_bound planes
		for(i=0; i<8; i++)
		{	
			if(mUpperBoundPlane.getDistance(frustum[i])/mLowerBoundPlane.getDistance(frustum[i]) < 0)
			{
				proj_points[n_points++] = frustum[i];
			}		
		}	

		// Create the camera the grid will be projected from
		delete mProjectingCamera;
		mProjectingCamera = new Ogre::Camera("PG_Projecting", NULL);

		// Set rendering camera parameters
		mProjectingCamera->setFrustumOffset(mRenderingCamera->getFrustumOffset());
		mProjectingCamera->setAspectRatio(mRenderingCamera->getAspectRatio());
		mProjectingCamera->setDirection(mRenderingCamera->getRealDirection());
		mProjectingCamera->setFarClipDistance(mRenderingCamera->getFarClipDistance());
		mProjectingCamera->setFOVy(mRenderingCamera->getFOVy());
		mProjectingCamera->setNearClipDistance(mRenderingCamera->getNearClipDistance());
		mProjectingCamera->setOrientation(mRenderingCamera->getRealOrientation());
		mProjectingCamera->setPosition(mRenderingCamera->getRealPosition());

		// Make sure the camera isn't too close to the plane
		float height_in_plane = mBasePlane.getDistance(mProjectingCamera->getRealPosition());
	
		bool keep_it_simple = false,
			 underwater     = false;

		if (height_in_plane < 0.0f) 
		{
			underwater = true;
		}

		if (keep_it_simple)
		{
			mProjectingCamera->setDirection(mRenderingCamera->getRealDirection());
		}
		else
		{
			Ogre::Vector3 aimpoint, aimpoint2;		

			if (height_in_plane < (mOptions.Strength + mOptions.Elevation))
			{					
				if (underwater)
				{
					mProjectingCamera->setPosition(mRenderingCamera->getRealPosition()+Ogre::Vector3(mLowerBoundPlane.normal.x,mLowerBoundPlane.normal.y,mLowerBoundPlane.normal.z)*(mOptions.Strength + mOptions.Elevation - 2*height_in_plane));
				}
				else
				{
					mProjectingCamera->setPosition(mRenderingCamera->getRealPosition()+Ogre::Vector3(mLowerBoundPlane.normal.x,mLowerBoundPlane.normal.y,mLowerBoundPlane.normal.z)*(mOptions.Strength + mOptions.Elevation - height_in_plane));
				}
			} 

			// Aim the projector at the point where the camera view-vector intersects the plane
			// if the camera is aimed away from the plane, mirror it's view-vector against the plane
			if (((mBasePlane.normal).dotProduct(mRenderingCamera->getRealDirection()) < 0.0f) || ((mBasePlane.normal).dotProduct(mRenderingCamera->getRealPosition()) < 0.0f ) )
			{
				_ray = Ogre::Ray(mRenderingCamera->getRealPosition(), mRenderingCamera->getRealDirection());
				_result = Ogre::Math::intersects(_ray,mBasePlane);

				if(_result.first)
				{
					aimpoint = mRenderingCamera->getRealPosition() + _result.second * mRenderingCamera->getRealDirection();
				}
			}
			else
			{
				Ogre::Vector3 flipped = mRenderingCamera->getRealDirection() - 2*mNormal* (mRenderingCamera->getRealDirection()).dotProduct(mNormal);
				flipped.normalise();
				_ray = Ogre::Ray( mRenderingCamera->getRealPosition(), flipped);
				_result = Ogre::Math::intersects(_ray,mBasePlane);

				if(_result.first)
				{
					aimpoint = mRenderingCamera->getRealPosition() + _result.second * flipped;
				}
			}

			// Force the point the camera is looking at in a plane, and have the projector look at it
			// works well against horizon, even when camera is looking upwards
			// doesn't work straight down/up
			float af = fabs((mBasePlane.normal).dotProduct(mRenderingCamera->getRealDirection()));
			aimpoint2 = mRenderingCamera->getRealPosition() + 10.0*mRenderingCamera->getRealDirection();
			aimpoint2 = aimpoint2 - mNormal* (aimpoint2.dotProduct(mNormal));

			// Fade between aimpoint & aimpoint2 depending on view angle
			aimpoint = aimpoint*af + aimpoint2*(1.0f-af);

			mProjectingCamera->setDirection(aimpoint-mProjectingCamera->getRealPosition());
		}

		for(i=0; i<n_points; i++)
		{
			// Project the point onto the surface plane
			proj_points[i] = proj_points[i] - mBasePlane.normal*mBasePlane.getDistance(proj_points[i]);
			proj_points[i] = mProjectingCamera->getViewMatrix() * proj_points[i];
			proj_points[i] = mProjectingCamera->getProjectionMatrixWithRSDepth() * proj_points[i];
		}

		// Get max/min x & y-values to determine how big the "projection window" must be
		if (n_points > 0)
		{
			x_min = proj_points[0].x;
			x_max = proj_points[0].x;
			y_min = proj_points[0].y;
			y_max = proj_points[0].y;

			for(i=1; i<n_points; i++)
			{
				if (proj_points[i].x > x_max) x_max = proj_points[i].x;
				if (proj_points[i].x < x_min) x_min = proj_points[i].x;
				if (proj_points[i].y > y_max) y_max = proj_points[i].y;
				if (proj_points[i].y < y_min) y_min = proj_points[i].y;
			}		

			// Build the packing matrix that spreads the grid across the "projection window"
			Ogre::Matrix4 pack(x_max-x_min,	0,				0,		x_min,
				               0,			y_max-y_min,	0,		y_min,
				               0,			0,				1,		0,	
				               0,			0,				0,		1);

			Ogre::Matrix4 invviewproj = (mProjectingCamera->getProjectionMatrixWithRSDepth()*mProjectingCamera->getViewMatrix()).inverse();
			*range = invviewproj * pack;

			return true;
		}

		return false;
	}

	void ProjectedGrid::_setDisplacementAmplitude(const float &Amplitude)
	{
		mUpperBoundPlane = Ogre::Plane( mNormal, mPos + Amplitude * mNormal);
		mLowerBoundPlane = Ogre::Plane( mNormal, mPos - Amplitude * mNormal);
	}

	float ProjectedGrid::getHeigth(const Ogre::Vector2 &Position)
	{
		return mHydrax->getPosition().y + mNoise->getValue(Position.x, Position.y)*mOptions.Strength;
	}
}}