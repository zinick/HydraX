/*
--------------------------------------------------------------------------------
This source file is part of Hydrax.
Visit ---

Copyright (C) 2007 Xavier Verguín González <xavierverguin@hotmail.com>
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

#ifndef _Hydrax_Mesh_H_
#define _Hydrax_Mesh_H_

#include "Prerequisites.h"

#include "Help.h"
#include "Structs.h"
#include "Image.h"

namespace Hydrax
{
    /** Class wich contains all funtions/variables related to
        Hydrax water mesh
     */
    class DllExport Mesh
    {
    public:
		/** Mesh type enum
		 */
		enum Type
		{
			SIMPLE_GRID    = 1 << 0,
			IMANTED_GRID   = 1 << 1,
			PROJECTED_GRID = 1 << 2,

			NONE = 0x0000,
			ALL  = 0x001F,
		};

		/** Base Hydrax mesh options class
		 */
		class Options
		{
		public:
			/** Default constructor
			    @param meshSize grid size (X/Z) world space.
				@param type Mesh::Type (Grid type)
			 */
			Options(const Size &meshSize, const Type &meshType)
				: MeshSize(meshSize)
				, MeshType(meshType)
			{
			}

			/// Grid size (X/Z) world space.
			Size MeshSize;
			/// Grid type 
			Type MeshType;
		};

		/** Type::SIMPLE_GRID Option class
		 */
		class SimpleGridOptions : public Options
		{
		public:
			/** Default constructor
			 */
			SimpleGridOptions()
				: Options(Size(256, 256), SIMPLE_GRID)
				, Complexity(64)
			{
			}

			/** Constructor
			    @param meshSize Water size
			    @param complexity N*N mesh grid complexity
			 */
			SimpleGridOptions(const Size  &meshSize,
				              const int   &complexity)
				: Options(meshSize, SIMPLE_GRID)
				, Complexity(complexity)
			{
				if (Complexity > 724)
				{
					Complexity = 724;
				}
			}

			/// N*N mesh grid complexity
			int Complexity;
		};

		/** Type::IMANTED_GRID Option class
		 */
		class ImantedGridOptions : public Options
		{
		public:
			/** Default constructor
			 */
			ImantedGridOptions()
				: Options(Size(256, 256), IMANTED_GRID)
				, Complexity(64)
				, ImanFactor(2)
				, CameraDistance(100)
			{
			}

			/** Constructor
			    @param meshSize Water size
			    @param complexity N*N mesh grid complexity
				@param imanfactor "Power" of the iman
				@param cameradistance Camera distance for recalculate grid LOD
			 */
			ImantedGridOptions(const Size  &meshSize,
				               const int   &complexity,
							   const float &imanfactor,
							   const float &cameradistance)
				: Options(meshSize, IMANTED_GRID)
				, Complexity(complexity)
				, ImanFactor(imanfactor)
				, CameraDistance(cameradistance)
			{
				if (Complexity > 724)
				{
					Complexity = 724;
				}
			}

			/// N*N mesh grid complexity
			int Complexity;
			/// Iman factor
			float ImanFactor;
			/// Camera distance for recalculate grid LOD
			float CameraDistance;
		};

		/** Type::PROJECTED_GRID Option class
		 */
		class ProjectedGridOptions : public Options
		{
		public:
			/** Default constructor
			 */
			ProjectedGridOptions()
				: Options(Size(-1, -1), PROJECTED_GRID)
				, Complexity(128)
			{
			}

			/** Constructor
			    @param complexity N*N mesh projected grid complexity
			 */
			ProjectedGridOptions(const int &complexity)
				: Options(Size(-1,-1), PROJECTED_GRID)
				, Complexity(complexity)
			{
				if (Complexity > 724)
				{
					Complexity = 724;
				}
			}

			/// N*N mesh projected grid complexity
			int Complexity;
		};

        /** Constructor
            @param sm Ogre SceneManager
			@param c Ogre::Camera
         */
		Mesh(Ogre::SceneManager *sm, Ogre::Camera *c);

        /** Destructor
         */
        ~Mesh();

        /** Set options
            @param Options base class pointer
         */
        void setOptions(Options *Options);

        /** Set mesh material
            @param MaterialName The material name
         */
        void setMaterialName(const Ogre::String &MaterialName);

        /** Create our water mesh, geometry, entity, etc...
            @param SceneNode Ogre SceneNode where entity will be attach
            @remarks Call it after setMeshOptions() and setMaterialName()
         */
        void create(Ogre::SceneNode *SceneNode);

        /** Update our mesh grid from a Image ONE_CHANNEL type data
            @param HeigthMap Heigth data [0,1] range
			@return false if Image type isn't correct.
            @remarks Only call it after create() !
			         If you need update mesh with another approach,
					 get the position vertex buffer in your module.
         */
        bool update(const Image &HeigthMap);

		/** For update the projected grid geomtry
		    @param numVer Number of vertices
			@param verArray Vertices array
			@return false if numver of vertices do not correspond.
		 */
		bool updateProjectedGridGeometry(const int &numVer, void* verArray);

		/** Get if a Position point is inside of the grid
		    @param Position World-space point
			@return true if Position point is inside of the grid, else false.
		 */
		bool isPointInGrid(const Ogre::Vector2 &Position);

		/** Get the [0,1] range x/y grid position from a 2D world space x/z point
		    @param Position World-space point
			@return (-1,-1) if the point isn't in the grid.
		 */
		Ogre::Vector2 getGridPosition(const Ogre::Vector2 &Position);

        /** Set water strength(Y axis multiplier)
            @param Strength Strength
         */
        void setStrength(const Ogre::Real &Strength);

        /** Get mesh
            @return Mesh
         */
        inline Ogre::MeshPtr getMesh()
        {
            return mMesh;
        }

        /** Get sub mesh
            @return Sub mesh
         */
        inline Ogre::SubMesh* getSubMesh()
        {
            return mSubMesh;
        }

        /** Get entity
            @return Entity
         */
        inline Ogre::Entity* getEntity()
        {
            return mEntity;
        }

        /** Get options
		    @return Options base class pointer
		 */
		inline Options* getOptions()
		{
			return mOptions;
		}

		/** Get mesh size
		    @return Mesh size
		 */
		inline const Size& getSize() const
		{
			return mOptions->MeshSize;
		}
		 
		/** Get mesh type
		    return Mesh grid type
		 */
		inline const Type& getType() const
		{
			return mOptions->MeshType;
		}

        /** Get number of faces
            @return Number of faces
         */
        inline const int& getNumFaces() const
        {
            return mNumFaces;
        }

        /** Get number of vertices
            @return Number of vertices
         */
        inline const int& getNumVertices() const
        {
            return mNumVertices;
        }

        /** Get material name
            @return Material name
         */
        inline const Ogre::String& getMaterialName() const
        {
            return mMaterialName;
        }

        /** Get strength
            @return Sterngth
         */
        inline const Ogre::Real& getStrength() const
        {
            return mStrength;
        }

		/** Get position vertex buffer
            @return Ogre::HardwareVertexBufferSharedPtr
         */
        inline Ogre::HardwareVertexBufferSharedPtr getPositionHardwareVertexBufferSharedPtr()
        {
            return mPosVertexBuffer;
        }

		/** Get texture coords vertex buffer
            @return Ogre::HardwareVertexBufferSharedPtr
         */
        inline Ogre::HardwareVertexBufferSharedPtr getTextureHardwareVertexBufferSharedPtr()
        {
            return mTexcoordsVertexBuffer;
        }

		/** Get index buffer
		    @return Ogre::HardwareIndexBufferSharedPtr
		 */
		inline Ogre::HardwareIndexBufferSharedPtr getHardwareIndexBufferSharedPtr()
		{
			return mIndexBuffer;
		}

    private:
		/** Creates geometry for Mesh::Type::SIMPLE_GRID
		 */
		void _createSimpleGridGeometry();

		/** Creates geometry for Mesh::Type::PROJECTED_GRID
		 */
		void _createProjectedGridGeometry();

		/** Update our Mesh::Type::SIMPLE_GRID geometry from a Image ONE_CHANNEL type data
            @param HeigthMap Heigth data [0,1] range
			@return false if Image type isn't correct.
            @remarks Only call it after create() !
			         If you need update mesh with another approach,
					 get the position vertex buffer in your module.
         */
        bool _updateSimpleGridGeometry(const Image &HeigthMap);

		/** Update our Mesh::Type::IMANTED_GRID geometry from a Image ONE_CHANNEL type data
            @param HeigthMap Heigth data [0,1] range
			@return false if Image type isn't correct.
            @remarks Only call it after create() !
			         If you need update mesh with another approach,
					 get the position vertex buffer in your module.
         */
        bool _updateImantedGridGeometry(const Image &HeigthMap);

        /// Mesh options
        Options *mOptions;
        /// Ogre::MeshPtr
        Ogre::MeshPtr mMesh;
        /// Ogre::Submesh pointer
        Ogre::SubMesh *mSubMesh;
        /// Ogre::Entity pointer
        Ogre::Entity *mEntity;
        /// Number of faces
        int mNumFaces;
        /// Number of vertices
        int mNumVertices;

        /// Position vertex buffer
        Ogre::HardwareVertexBufferSharedPtr mPosVertexBuffer;
        /// Texture coordinate vertex buffer
        Ogre::HardwareVertexBufferSharedPtr mTexcoordsVertexBuffer;
        /// Index buffer
        Ogre::HardwareIndexBufferSharedPtr  mIndexBuffer;

        /// Water strength(Y axis multiplier)
        Ogre::Real mStrength;
        /// Material name
        Ogre::String mMaterialName;

		/// For last camera x/z position
		Ogre::Vector2 mLastCameraPosition;

        /// Pointer to Ogre::SceneManager
        Ogre::SceneManager *mSceneManager;
		/// Pointer to Ogre::Camera(for LODs)
		Ogre::Camera *mCamera;
    };
}

#endif
