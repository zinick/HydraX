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

#ifndef _Hydrax_Modules_PerlinPG_H_
#define _Hydrax_Modules_PerlinPG_H_

#include "../../Prerequisites.h"

#include "../../Hydrax.h"
#include "../Module.h"

#define n_bits				5
#define n_size				(1<<(n_bits-1))
#define n_size_m1			(n_size - 1)
#define n_size_sq			(n_size*n_size)
#define n_size_sq_m1		(n_size_sq - 1)

#define n_packsize			4

#define np_bits				(n_bits+n_packsize-1)
#define np_size				(1<<(np_bits-1))
#define np_size_m1			(np_size-1)
#define np_size_sq			(np_size*np_size)
#define np_size_sq_m1		(np_size_sq-1)

#define n_dec_bits			12
#define n_dec_magn			4096
#define n_dec_magn_m1		4095

#define max_octaves			32

#define noise_frames		256
#define noise_frames_m1		(noise_frames-1)

#define noise_decimalbits	15
#define noise_magnitude		(1<<(noise_decimalbits-1))

#define scale_decimalbits	15
#define scale_magnitude		(1<<(scale_decimalbits-1))

struct SOFTWARESURFACEVERTEX
{
	float x,y,z;
	float nx,ny,nz;
	float tu,tv;
};

namespace Hydrax{ namespace Module
{
	class DllExport PerlinPG : public Module
	{
	public:
		/** Struct wich contains Hydrax perlin projected grid module options
		 */
		struct Options
		{
			/// Projected grid complexity (N*N)
			int Complexity;
			/// Strength
			float Strength;
			/// Displace
			bool Displace;
			/// Octaves
			int Octaves;
			/// Scale
			float Scale;
			/// Falloff
			float Falloff;
			/// Animspeed
			float Animspeed;
			/// Timemulti
			float Timemulti;
			/// Elevation 
			float Elevation;
			/// Smooth
			bool Smooth;

			/** Default constructor
			 */
			Options()
				: Complexity(256)
				, Strength(32.5f)
				, Displace(true)
				, Octaves(8)
				, Scale(0.085f)
				, Falloff(0.49f)
				, Animspeed(1.4f)
				, Timemulti(1.27f)
				, Elevation(7.0f)
				, Smooth(false)
			{
			}

			/** Constructor
			    @param _Complexity Projected grid complexity
			 */
			Options(const int &_Complexity)
				: Complexity(_Complexity)
				, Strength(32.5f)
				, Displace(true)
				, Octaves(8)
				, Scale(0.085f)
				, Falloff(0.49f)
				, Animspeed(1.4f)
				, Timemulti(1.27f)
				, Elevation(7.0f)
				, Smooth(false)
			{
			}

			/** Constructor
			    @param _Complexity Projected grid complexity
				@param _Strength Perlin noise strength
				@param _Displace Displace vertex?
				@param _Octaves Perlin noise octaves
				@param _Scale Noise scale
				@param _Falloff Noise fall off
				@param _Animspeed Animation speed
				@param _Timemulti Timemulti
				@param _Elevation Elevation
				@param _Smooth Smooth vertex?
			 */
			Options(const int   &_Complexity,
				    const float &_Strength,
					const bool  &_Displace,
					const int   &_Octaves,
					const float &_Scale,
					const float &_Falloff,
					const float &_Animspeed,
					const float &_Timemulti,
					const float &_Elevation,
					const bool  &_Smooth)
				: Complexity(_Complexity)
				, Strength(_Strength)
				, Displace(_Displace)
				, Octaves(_Octaves)
				, Scale(_Scale)
				, Falloff(_Falloff)
				, Animspeed(_Animspeed)
				, Timemulti(_Timemulti)
				, Elevation(_Elevation)
				, Smooth(_Smooth)
			{
			}
		};

		/** Constructor
		    @param h Hydrax manager pointer
			@param BasePlane Noise base plane
		 */
		PerlinPG(Hydrax *h, const Ogre::Plane &BasePlane);

		/** Constructor
		    @param h Hydrax manager pointer
			@param BasePlane Noise base plane
			@param Options Perlin options
		 */
		PerlinPG(Hydrax *h, const Ogre::Plane &BasePlane, const Options &Options);

		/** Destructor
		 */
        ~PerlinPG();

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
		/** Initialize noise
		 */
		void _initNoise();

		/** Calcule noise
		 */
		void _calculeNoise();

		/** Calcule current normals
		 */
		void _calculeNormals();

		/** Read texel linear dual
		    @param u u
			@param v v
			@param o Octave
			@return int
		 */
	    int _readTexelLinearDual(const int &u, const int &v, const int &o);

		/** Read texel linear
		    @param u u
			@param v v
			@return Heigth
		 */
		float _getHeigthDual(int u, int v);

		/** Map sample
		    @param u u
			@param v v
			@param level Level
			@param octave Octave
			@return Map sample
		 */
		int _mapSample(const int &u, const int &v, const int &upsamplepower, const int &octave);

		/** Render geometry
		    @param m Range
			@param _viewMat View matrix
			@return true if it's sucesfful
		 */
		bool _renderGeometry(const Ogre::Matrix4& m,const Ogre::Matrix4& _viewMat);

		/** Calcule world position
		    @param uv uv
			@param m Range
			@param _viewMat View matrix
			@return The position in homogenous coordinates 
		 */
		Ogre::Vector4 _calculeWorldPosition(const Ogre::Vector2 &uv, const Ogre::Matrix4& m,const Ogre::Matrix4& _viewMat);
	
		/** Get min/max
		    @param range Range
			@return true if it's in min/max
		 */
	    bool _getMinMax(Ogre::Matrix4 *range);

		/** Set displacement amplitude
		    @param Amplitude Amplitude to set
		 */
		void _setDisplacementAmplitude(const float &Amplitude);

		/// Vertex pointer
		SOFTWARESURFACEVERTEX *vertices;

		/// Noise variables
		int noise[n_size_sq*noise_frames];
		int o_noise[n_size_sq*max_octaves];
		int p_noise[np_size_sq*(max_octaves>>(n_packsize-1))];	
		int *r_noise;
		int octaves;

		/// Time
		double time;

		/// For corners
		Ogre::Vector4 t_corners0,t_corners1,t_corners2,t_corners3;

		/// Range matrix
		Ogre::Matrix4 mRange;

		/// Planes
	    Ogre::Plane	mBasePlane, 
			        mUpperBoundPlane, 
					mLowerBoundPlane;

		/// Cameras
	    Ogre::Camera *mProjectingCamera,	// The camera that does the actual projection
		             *mRenderingCamera;		// The camera whose frustum the projection is created for

		/// Normal and position
	    Ogre::Vector3 mNormal, mPos;

		/// Our perlin noise projected grid options
		Options mOptions;

		/// Our Hydrax pointer
		Hydrax* mHydrax;
	};
}}

#endif