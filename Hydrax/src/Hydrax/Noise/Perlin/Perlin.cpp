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

Based on the perlin noise code from Claes Johanson thesis:
http://graphics.cs.lth.se/theses/projects/projgrid/
--------------------------------------------------------------------------------
*/

#include "Perlin.h"

#define _def_PackedNoise true

namespace Hydrax{namespace Noise
{
	Perlin::Perlin()
		: Noise("Perlin")
		, octaves(0)
		, time(0)
		, magnitude(n_dec_magn * 0.085f)
	{
	}

	Perlin::Perlin(const Options &Options)
		: Noise("Perlin")
		, mOptions(Options)
		, octaves(0)
		, time(0)
		, magnitude(n_dec_magn * Options.Scale)
	{
	}

	Perlin::~Perlin()
	{
		if (r_noise)
		{
			delete [] r_noise;
		}

		HydraxLOG(getName() + " destroyed.");
	}

	void Perlin::create()
	{
		if (isCreated())
		{
			return;
		}

		Noise::create();
		_initNoise();
	}

	void Perlin::saveCfg(Ogre::String &Data)
	{
		Noise::saveCfg(Data);
	}

	bool Perlin::loadCfg(Ogre::ConfigFile &CfgFile)
	{
		return Noise::loadCfg(CfgFile);
	}

	void Perlin::update(const Ogre::Real &timeSinceLastFrame)
	{
		time += timeSinceLastFrame*mOptions.Animspeed;
		_calculeNoise();
	}

	float Perlin::getValue(const float &x, const float &y)
	{
		return _getHeigthDual(x,y);
	}

	void Perlin::_initNoise()
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

	void Perlin::_calculeNoise()
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

	int Perlin::_readTexelLinearDual(const int &u, const int &v,const int &o)
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

	float Perlin::_getHeigthDual(float u, float v)
	{	
		// Pointer to the current noise source octave	
		r_noise = p_noise;	

		int ui = u*magnitude,
		    vi = v*magnitude,
			i, 
			value = 0,
			hoct = octaves / n_packsize;

		for(i=0; i<hoct; i++)
		{		
			value += _readTexelLinearDual(ui,vi,0);
			ui = ui << n_packsize;
			vi = vi << n_packsize;
			r_noise += np_size_sq;
		}		

		return static_cast<float>(value)/noise_magnitude;
	}

	int Perlin::_mapSample(const int &u, const int &v, const int &upsamplepower, const int &octave)
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
}}