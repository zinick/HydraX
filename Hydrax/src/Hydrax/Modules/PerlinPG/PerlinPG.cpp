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

Based on the Projected Grid concept from Claes Johanson thesis:
http://graphics.cs.lth.se/theses/projects/projgrid/
and Ren Cheng Ogre3D implementation:
http://www.cnblogs.com/ArenAK/archive/2007/11/07/951713.html 
--------------------------------------------------------------------------------
*/

#include "PerlinPG.h"

#define _def_PackedNoise        true
#define _def_MaxFarClipDistance 99999

namespace Hydrax{namespace Module
{
	PerlinPG::PerlinPG(Hydrax *h, const Ogre::Plane &BasePlane)
		: Module("PerlinPGModule", Mesh::PROJECTED_GRID, MaterialManager::NM_VERTEX)
		, mHydrax(h)
		, mBasePlane(BasePlane)
		, mNormal(BasePlane.normal)
		, mPos(Ogre::Vector3(0,0,0))
		, mProjectingCamera(0)
		, mRenderingCamera(h->getCamera())
		, octaves(0)
		, time(0)
	{
	}

	PerlinPG::PerlinPG(Hydrax *h, const Ogre::Plane &BasePlane, const Options &Options)
		: Module("PerlinPGModule", Mesh::PROJECTED_GRID, MaterialManager::NM_VERTEX)
		, mHydrax(h)
		, mBasePlane(BasePlane)
		, mNormal(BasePlane.normal)
		, mPos(Ogre::Vector3(0,0,0))
		, mProjectingCamera(0)
		, mRenderingCamera(h->getCamera())
		, octaves(0)
		, time(0)
	{
		setOptions(Options);
	}

	PerlinPG::~PerlinPG()
	{
		if (vertices)
		{
		    delete [] vertices;
		}

		if (mProjectingCamera)
		{
			delete mProjectingCamera;
		}

		if (r_noise)
		{
			delete [] r_noise;
		}

		HydraxLOG(getName() + " destroyed.");
	}

	void PerlinPG::setOptions(const Options &Options)
	{
		mHydrax->setStrength(mOptions.Strength);

		// If create() is called, change only the on-fly parameters
		if (isCreated())
		{
			mOptions.Strength  = Options.Strength;
			mOptions.Displace  = Options.Displace;
			mOptions.Scale     = Options.Scale;
			mOptions.Falloff   = Options.Falloff;
			mOptions.Animspeed = Options.Animspeed;
			mOptions.Timemulti = Options.Timemulti;
			mOptions.Elevation = Options.Elevation;
			mOptions.Smooth    = Options.Smooth;

			return;
		} 

		mOptions = Options;
	}

	void PerlinPG::create()
	{
		HydraxLOG("Creating " + getName() + " module.");

		Module::create();

		vertices = new SOFTWARESURFACEVERTEX[mOptions.Complexity*mOptions.Complexity];	

		int v, u;
		for(v=0; v<mOptions.Complexity; v++)
		{
			for(u=0; u<mOptions.Complexity; u++)
			{
				vertices[v*mOptions.Complexity + u].nx = 0.0f;
				vertices[v*mOptions.Complexity + u].ny = 1.0f;
				vertices[v*mOptions.Complexity + u].nz = 0.0f;
				vertices[v*mOptions.Complexity + u].tu = static_cast<float>(u)/(mOptions.Complexity-1);
				vertices[v*mOptions.Complexity + u].tv = static_cast<float>(v)/(mOptions.Complexity-1);
			}
		}	

		_initNoise();	
	    _setDisplacementAmplitude(0.0f);

		HydraxLOG(getName() + " created.");
	}

	void PerlinPG::update(const Ogre::Real &timeSinceLastFrame)
	{
		if (!isCreated())
		{
			return;
		}

		time += timeSinceLastFrame*mOptions.Animspeed;

		float RenderingFarClipDistance = mRenderingCamera->getFarClipDistance();
		if (RenderingFarClipDistance > _def_MaxFarClipDistance)
		{
			mRenderingCamera->setFarClipDistance(_def_MaxFarClipDistance);
		}

		if (_getMinMax(&mRange))
		{
			_renderGeometry(mRange,mProjectingCamera->getViewMatrix());

			mHydrax->getMesh()->updateProjectedGridGeometry(mOptions.Complexity*mOptions.Complexity, vertices);
		}

		mRenderingCamera->setFarClipDistance(RenderingFarClipDistance);
	}

	void PerlinPG::_initNoise()
	{	
		// Create noise (uniform)
		float tempnoise[n_size_sq*noise_frames], temp;

		int i, frame, v, u, 
            v0, v1, v2, u0, u1, u2, f;

		for(i=0; i<(n_size_sq*noise_frames); i++)
		{
			temp = static_cast<float>(rand())/RAND_MAX;		
			tempnoise[i] = 4*(temp - 0.5f);	
		}	

		for(frame=0; frame<noise_frames; frame++)
		{
			for(v=0; v<n_size; v++)
			{
				for(u=0; u<n_size; u++)
				{	
					v0 = ((v-1)&n_size_m1)*n_size;
					v1 = v*n_size;
					v2 = ((v+1)&n_size_m1)*n_size;
					u0 = ((u-1)&n_size_m1);
					u1 = u;
					u2 = ((u+1)&n_size_m1);				
					f  = frame*n_size_sq;

					temp = (1.0f/14.0f) *
					   (tempnoise[f + v0 + u0] +      tempnoise[f + v0 + u1] + tempnoise[f + v0 + u2] +
						tempnoise[f + v1 + u0] + 6.0f*tempnoise[f + v1 + u1] + tempnoise[f + v1 + u2] +
						tempnoise[f + v2 + u0] +      tempnoise[f + v2 + u1] + tempnoise[f + v2 + u2]);

					noise[frame*n_size_sq + v*n_size + u] = noise_magnitude*temp;
				}
			}
		}	
	}

	void PerlinPG::_calculeNoise()
	{
        #define min(a,b) (((a) < (b)) ? (a) : (b))

		int i, o, v, u,
			multitable[max_octaves],
			amount[3],
			iImage;

		unsigned int image[3];

		float sum = 0.0f,
			  f_multitable[max_octaves];

		double dImage, fraction;

		octaves = min(mOptions.Octaves, max_octaves);		

		// calculate the strength of each octave
		for(i=0; i<octaves; i++)
		{
			f_multitable[i] = powf(mOptions.Falloff,1.0f*i);
			sum += f_multitable[i];
		}

		for(i=0; i<octaves; i++)
		{
			f_multitable[i] /= sum;
		}
		
		for(i=0; i<octaves; i++)
		{
			multitable[i] = scale_magnitude*f_multitable[i];
		}
	
		double r_timemulti = 1.0;
		const float PI_3 = Ogre::Math::PI/3;

		for(o=0; o<octaves; o++)
		{		
			fraction = modf(time*r_timemulti,&dImage);
			iImage = static_cast<int>(dImage);

			amount[0] = scale_magnitude*f_multitable[o]*(pow(sin((fraction+2)*PI_3),2)/1.5);
			amount[1] = scale_magnitude*f_multitable[o]*(pow(sin((fraction+1)*PI_3),2)/1.5);
			amount[2] = scale_magnitude*f_multitable[o]*(pow(sin((fraction  )*PI_3),2)/1.5);

			image[0] = (iImage  ) & noise_frames_m1;
			image[1] = (iImage+1) & noise_frames_m1;
			image[2] = (iImage+2) & noise_frames_m1;
			
			for (i=0; i<n_size_sq; i++)
			{
			    o_noise[i + n_size_sq*o] = (	
				   ((amount[0] * noise[i + n_size_sq * image[0]])>>scale_decimalbits) + 
				   ((amount[1] * noise[i + n_size_sq * image[1]])>>scale_decimalbits) + 
				   ((amount[2] * noise[i + n_size_sq * image[2]])>>scale_decimalbits));
			}
			
			r_timemulti *= mOptions.Timemulti;
		}

		if(_def_PackedNoise)
		{
			int octavepack = 0;
			for(o=0; o<octaves; o+=n_packsize)
			{
				for(v=0; v<np_size; v++)
				{
					for(u=0; u<np_size; u++)
					{
						p_noise[v*np_size+u+octavepack*np_size_sq]  = o_noise[(o+3)*n_size_sq + (v&n_size_m1)*n_size + (u&n_size_m1)];
						p_noise[v*np_size+u+octavepack*np_size_sq] += _mapSample( u, v, 3, o);
						p_noise[v*np_size+u+octavepack*np_size_sq] += _mapSample( u, v, 2, o+1);
						p_noise[v*np_size+u+octavepack*np_size_sq] += _mapSample( u, v, 1, o+2);		
					}
				}

				octavepack++;
			}
		}
	}

	bool PerlinPG::_renderGeometry(const Ogre::Matrix4& m,const Ogre::Matrix4& _viewMat)
	{
		_calculeNoise();

		t_corners0 = _calculeWorldPosition(Ogre::Vector2( 0.0f, 0.0f),m,_viewMat);
		t_corners1 = _calculeWorldPosition(Ogre::Vector2(+1.0f, 0.0f),m,_viewMat);
		t_corners2 = _calculeWorldPosition(Ogre::Vector2( 0.0f,+1.0f),m,_viewMat);
		t_corners3 = _calculeWorldPosition(Ogre::Vector2(+1.0f,+1.0f),m,_viewMat);
	
		float magnitude = n_dec_magn * mOptions.Scale,
			  du  = 1.0f/(mOptions.Complexity-1),
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

				vertices[i].x = result.x;
				vertices[i].z = result.z;
				vertices[i].y = -mBasePlane.d + _getHeigthDual(magnitude*result.x, magnitude*result.z);

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
			for(iv=1; v<(mOptions.Complexity-1); iv++)
			{
				for(iu=1; u<(mOptions.Complexity-1); iu++)
				{				
					vertices[iv*mOptions.Complexity + iu].y =	
						0.2f *
					   (vertices[iv    *mOptions.Complexity + iu    ].y +
						vertices[iv    *mOptions.Complexity + (iu+1)].y + 
						vertices[iv    *mOptions.Complexity + (iu-1)].y + 
						vertices[(iv+1)*mOptions.Complexity + iu    ].y + 
						vertices[(iv-1)*mOptions.Complexity + iu    ].y);															
				}
			}
		}

		if (!mOptions.Displace)
		{
			// Reset height to 0
			for(i=0; i<(mOptions.Complexity*mOptions.Complexity); i++)
			{
				vertices[i].y = 0;
			}
		}

		_calculeNormals();

		return true;
	}

	void PerlinPG::_calculeNormals()
	{
		int v, u;
		Ogre::Vector3 vec1, vec2, normal;

		for(v=1; v<(mOptions.Complexity-1); v++)
		{
			for(u=1; u<(mOptions.Complexity-1); u++)
			{
				vec1 = Ogre::Vector3(
					vertices[v*mOptions.Complexity + u + 1].x-vertices[v*mOptions.Complexity + u - 1].x,
					vertices[v*mOptions.Complexity + u + 1].y-vertices[v*mOptions.Complexity + u - 1].y, 
					vertices[v*mOptions.Complexity + u + 1].z-vertices[v*mOptions.Complexity + u - 1].z);

				vec2 = Ogre::Vector3(
					vertices[(v+1)*mOptions.Complexity + u].x - vertices[(v-1)*mOptions.Complexity + u].x,
					vertices[(v+1)*mOptions.Complexity + u].y - vertices[(v-1)*mOptions.Complexity + u].y,
					vertices[(v+1)*mOptions.Complexity + u].z - vertices[(v-1)*mOptions.Complexity + u].z);

				normal = vec2.crossProduct(vec1);

				vertices[v*mOptions.Complexity + u].nx = normal.x;
				vertices[v*mOptions.Complexity + u].ny = normal.y;
				vertices[v*mOptions.Complexity + u].nz = normal.z;
			}
		}
	}

	// Check the point of intersection with the plane (0,1,0,0) and return the position in homogenous coordinates 
	Ogre::Vector4 PerlinPG::_calculeWorldPosition(const Ogre::Vector2 &uv, const Ogre::Matrix4& m, const Ogre::Matrix4& _viewMat)
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

	int PerlinPG::_readTexelLinearDual(const int &u, const int &v,const int &o)
	{
		int iu, iup, iv, ivp, fu, fv,
			ut01, ut23, ut;

		iu = (u>>n_dec_bits)&np_size_m1;
		iv = ((v>>n_dec_bits)&np_size_m1)*np_size;

		iup = ((u>>n_dec_bits) + 1)&np_size_m1;
		ivp = (((v>>n_dec_bits) + 1)&np_size_m1)*np_size;

		fu = u & n_dec_magn_m1;
		fv = v & n_dec_magn_m1;

		ut01 = ((n_dec_magn-fu)*r_noise[iv + iu] + fu*r_noise[iv + iup])>>n_dec_bits;
		ut23 = ((n_dec_magn-fu)*r_noise[ivp + iu] + fu*r_noise[ivp + iup])>>n_dec_bits;
		ut = ((n_dec_magn-fv)*ut01 + fv*ut23) >> n_dec_bits;

		return ut;
	}

	float PerlinPG::_getHeigthDual(int u, int v)
	{	
		// Pointer to the current noise source octave	
		r_noise = p_noise;	

		int i, 
			value = 0,
			hoct = octaves / n_packsize;

		for(i=0; i<hoct; i++)
		{		
			value += _readTexelLinearDual(u,v,0);
			u = u << n_packsize;
			v = v << n_packsize;
			r_noise += np_size_sq;
		}		

		return value*mOptions.Strength/noise_magnitude;
	}

	int PerlinPG::_mapSample(const int &u, const int &v, const int &upsamplepower, const int &octave)
	{
		int magnitude = 1<<upsamplepower,

		    pu = u >> upsamplepower,
		    pv = v >> upsamplepower,

		    fu = u & (magnitude-1),
		    fv = v & (magnitude-1),

		    fu_m = magnitude - fu,
		    fv_m = magnitude - fv,

		    o = fu_m*fv_m*o_noise[octave*n_size_sq + ((pv)  &n_size_m1)*n_size + ((pu)  &n_size_m1)] +
			    fu*  fv_m*o_noise[octave*n_size_sq + ((pv)  &n_size_m1)*n_size + ((pu+1)&n_size_m1)] +
			    fu_m*fv*  o_noise[octave*n_size_sq + ((pv+1)&n_size_m1)*n_size + ((pu)  &n_size_m1)] +
			    fu*  fv*  o_noise[octave*n_size_sq + ((pv+1)&n_size_m1)*n_size + ((pu+1)&n_size_m1)];

		return o >> (upsamplepower+upsamplepower);
	}

	bool PerlinPG::_getMinMax(Ogre::Matrix4 *range)
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

	void PerlinPG::_setDisplacementAmplitude(const float &Amplitude)
	{
		mUpperBoundPlane = Ogre::Plane( mNormal, mPos + Amplitude * mNormal);
		mLowerBoundPlane = Ogre::Plane( mNormal, mPos - Amplitude * mNormal);
	}

	float PerlinPG::getHeigth(const Ogre::Vector2 &Position)
	{
		float magnitude = n_dec_magn * mOptions.Scale;

		return mHydrax->getPosition().y + _getHeigthDual(magnitude*Position.x, magnitude*Position.y);
	}
}}