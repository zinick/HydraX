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

#include "Mesh.h"
#include "Modules/PerlinPG/PerlinPG.h"

namespace Hydrax
{
	Mesh::Mesh(Ogre::SceneManager *sm, Ogre::Camera *c)
            : mSceneManager(sm)
			, mCamera(c)
			, mOptions(static_cast<Options*>(new SimpleGridOptions()))
            , mStrength(20)
            , mMesh(0)
            , mSubMesh(0)
            , mEntity(0)
            , mNumFaces(0)
            , mNumVertices(0)
            , mPosVertexBuffer(0)
            , mTexcoordsVertexBuffer(0)
            , mIndexBuffer(0)
            , mMaterialName("_NULL_")
			, mLastCameraPosition(Ogre::Vector2::ZERO)
    {
    }

    Mesh::~Mesh()
    {
        Ogre::MeshManager::getSingleton().remove("HydraxMesh");

        mSceneManager->destroyEntity(mEntity);
    }

    void Mesh::setOptions(Options *Options)
    {
		mOptions = Options;
    }

    void Mesh::setMaterialName(const Ogre::String &MaterialName)
    {
        mMaterialName = MaterialName;

        if (mEntity)
        {
            mEntity->setMaterialName(mMaterialName);
        }
    }

	void Mesh::create(Ogre::SceneNode *SceneNode)
	{
		// Create mesh and submesh
        mMesh = Ogre::MeshManager::getSingleton().createManual("HydraxMesh",
                HYDRAX_RESOURCE_GROUP);
        mSubMesh = mMesh->createSubMesh();
        mSubMesh->useSharedVertices = false;

		switch(mOptions->MeshType)
		{
		    case SIMPLE_GRID:
			case IMANTED_GRID:
			{
				_createSimpleGridGeometry();
			}
			break;

			case PROJECTED_GRID:
			{
				_createProjectedGridGeometry();
			}
			break;
		}

		// End mesh creation
        Ogre::AxisAlignedBox meshBounds(0,0,0,
			                            mOptions->MeshSize.Width, mStrength, mOptions->MeshSize.Height);

		if (getType() == PROJECTED_GRID)
		{
			meshBounds = Ogre::AxisAlignedBox(-1000000,-1000000,-1000000,
		                                       1000000, 1000000, 1000000);
		}

        mMesh->_setBounds(meshBounds);
        mMesh->load();
        mMesh->touch();

        mEntity = mSceneManager->createEntity("HydraxMeshEnt", "HydraxMesh");
        mEntity->setMaterialName(mMaterialName);
        mEntity->setRenderQueueGroup(Ogre::RENDER_QUEUE_9);
        Ogre::MaterialPtr m = Ogre::MaterialManager::getSingleton().getByName(mMaterialName);
        m->setReceiveShadows(false);
        SceneNode->attachObject(mEntity);

		if (getType() == PROJECTED_GRID)
		{
			SceneNode->setPosition(0,0,0);
		}
		else
		{
			SceneNode->setPosition(-mOptions->MeshSize.Width/2,0,-mOptions->MeshSize.Height/2);
		}
	}

    void Mesh::_createSimpleGridGeometry()
    {
		SimpleGridOptions* SGOptions = static_cast<SimpleGridOptions*>(mOptions);

        int x,y;

		mNumFaces    = 2 * SGOptions->Complexity * SGOptions->Complexity;
        mNumVertices = (SGOptions->Complexity + 1) * (SGOptions->Complexity + 1) ;

        // Vertex buffers
        mSubMesh->vertexData = new Ogre::VertexData();
        mSubMesh->vertexData->vertexStart = 0;
        mSubMesh->vertexData->vertexCount = mNumVertices;

        Ogre::VertexDeclaration*   vdecl = mSubMesh->vertexData->vertexDeclaration;
        Ogre::VertexBufferBinding* vbind = mSubMesh->vertexData->vertexBufferBinding;

        vdecl->addElement(0, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
        vdecl->addElement(1, 0, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);

        // --- Position vertices ---
        // Create vertex buffer
        mPosVertexBuffer =
            Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
                3*sizeof(float),
                mNumVertices,
                Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

        // Set bingind
        vbind->setBinding(0, mPosVertexBuffer);

        // Lock vertex buffer
        float *posVertexBuffer =
            (float*)mPosVertexBuffer->
            lock(0,
                 mNumVertices,
                 Ogre::HardwareBuffer::HBL_DISCARD);

        // Set values
        for (y=0;y<=SGOptions->Complexity;y++)
        {
            for (x=0;x<=SGOptions->Complexity;x++)
            {
                int numPoint = y*(SGOptions->Complexity+1) + x ;

                posVertexBuffer[3*numPoint+0] = (float)(x) / (float)(SGOptions->Complexity) * (float) SGOptions->MeshSize.Width;
                posVertexBuffer[3*numPoint+1] = 0;
                posVertexBuffer[3*numPoint+2] = (float)(y) / (float)(SGOptions->Complexity) * (float) SGOptions->MeshSize.Height;
            }
        }

        // Unlock the buffer
        mPosVertexBuffer->unlock();

        // --- Texcoords ---
        // Create vertex buffer
		if (getType() == IMANTED_GRID)
		{
           mTexcoordsVertexBuffer =
               Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
                   2*sizeof(float),
                   mNumVertices,
                   Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
		}
		else
		{
		    mTexcoordsVertexBuffer =
               Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
                   2*sizeof(float),
                   mNumVertices,
				   Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
		}

        // Set binding
        vbind->setBinding(1, mTexcoordsVertexBuffer);

        // Lock our vertices
        float *texcoordsBufData =
            (float*)mTexcoordsVertexBuffer->
            lock(0,
                 mNumVertices,
                 Ogre::HardwareBuffer::HBL_DISCARD);
        // Asign values
        for (y=0;y<=SGOptions->Complexity;y++)
        {
            for (x=0;x<=SGOptions->Complexity;x++)
            {
                texcoordsBufData[2*(y*(SGOptions->Complexity+1)+x)+0] = (float)y / SGOptions->Complexity ;
                texcoordsBufData[2*(y*(SGOptions->Complexity+1)+x)+1] = (float)x / SGOptions->Complexity ;
            }
        }

        // Unlock
        mTexcoordsVertexBuffer->unlock();

        // --- Index buffer ---
        // Find what we need, 16 or 32 bit buffer
        bool is32Bits = false;
        Ogre::HardwareIndexBuffer::IndexType IT_BIT = Ogre::HardwareIndexBuffer::IT_16BIT;

        if (SGOptions->Complexity > 255)
        {
            is32Bits = true;
            IT_BIT = Ogre::HardwareIndexBuffer::IT_32BIT;
        }

        // Create index buffer
        mIndexBuffer =
            Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
                IT_BIT,
                3*mNumFaces,
                Ogre::HardwareBuffer::HBU_STATIC, true);

        // Lock
        unsigned short *faceVertexIndices16Bits;
        unsigned long  *faceVertexIndices32Bits;

        if (!is32Bits)
        {
            // 16 Bit buffer
            faceVertexIndices16Bits =
                (unsigned short*)mIndexBuffer->
                lock(0,
                     mNumFaces*3*2,
                     Ogre::HardwareBuffer::HBL_DISCARD);
        }
        else
        {
            // 32 bits buffer
            faceVertexIndices32Bits =
                (unsigned long*)mIndexBuffer->
                lock(0,
                     mNumFaces*3*2,
                     Ogre::HardwareBuffer::HBL_DISCARD);
        }

        // Set values
        unsigned short *twoface16Bits;
        unsigned long  *twoface32Bits;

        for (y=0 ; y<SGOptions->Complexity ; y++)
        {
            for (int x=0 ; x<SGOptions->Complexity ; x++)
            {
                int p0 = y*(SGOptions->Complexity+1) + x ;
                int p1 = y*(SGOptions->Complexity+1) + x + 1 ;
                int p2 = (y+1)*(SGOptions->Complexity+1) + x ;
                int p3 = (y+1)*(SGOptions->Complexity+1) + x + 1 ;

                if (!is32Bits)
                {
                    // 16 bits buffer
                    twoface16Bits = faceVertexIndices16Bits + (y*SGOptions->Complexity+x)*2*3;

                    // First triangle
                    twoface16Bits[0]=p2;
                    twoface16Bits[1]=p1;
                    twoface16Bits[2]=p0;
                    // Second triangle
                    twoface16Bits[3]=p2;
                    twoface16Bits[4]=p3;
                    twoface16Bits[5]=p1;
                }
                else
                {
                    // 32 bits buffer
                    twoface32Bits = faceVertexIndices32Bits + (y*SGOptions->Complexity+x)*2*3;

                    // First triangle
                    twoface32Bits[0]=p2;
                    twoface32Bits[1]=p1;
                    twoface32Bits[2]=p0;
                    // Second triangle
                    twoface32Bits[3]=p2;
                    twoface32Bits[4]=p3;
                    twoface32Bits[5]=p1;
                }
            }
        }

        // Unlock
        mIndexBuffer->unlock();

        // Set index buffer for this submesh
        mSubMesh->indexData->indexBuffer = mIndexBuffer;
        mSubMesh->indexData->indexStart = 0;
        mSubMesh->indexData->indexCount = 3*mNumFaces;
    }

	void Mesh::_createProjectedGridGeometry()
	{
		ProjectedGridOptions *PGOptions = static_cast<ProjectedGridOptions*>(mOptions);

		int numVertices = PGOptions->Complexity*PGOptions->Complexity;
		int numEle = 6 * (PGOptions->Complexity-1)*(PGOptions->Complexity-1);

		// Vertex buffers
		mSubMesh->vertexData = new Ogre::VertexData();
		mSubMesh->vertexData->vertexStart = 0;
		mSubMesh->vertexData->vertexCount = numVertices;

		Ogre::VertexDeclaration* vdecl = mSubMesh->vertexData->vertexDeclaration;
		Ogre::VertexBufferBinding* vbind = mSubMesh->vertexData->vertexBufferBinding;

		size_t offset = 0;
		vdecl->addElement(0, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
		offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
		vdecl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);
		offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
		vdecl->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);

		// Prepare buffer for positions - todo: first attempt, slow
		mPosVertexBuffer =
			Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
			sizeof(SOFTWARESURFACEVERTEX),
			numVertices,
			Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);
		vbind->setBinding(0, mPosVertexBuffer);

		unsigned int *indexbuffer = new unsigned int[numEle];
		int i = 0;
		{
			for(int v=0; v<PGOptions->Complexity-1; v++){
				for(int u=0; u<PGOptions->Complexity-1; u++){
					// face 1 |/
					indexbuffer[i++]	= v*PGOptions->Complexity + u;
					indexbuffer[i++]	= v*PGOptions->Complexity + u + 1;
					indexbuffer[i++]	= (v+1)*PGOptions->Complexity + u;

					// face 2 /|
					indexbuffer[i++]	= (v+1)*PGOptions->Complexity + u;
					indexbuffer[i++]	= v*PGOptions->Complexity + u + 1;
					indexbuffer[i++]	= (v+1)*PGOptions->Complexity + u + 1;
				}
			}
		}

		// Prepare buffer for indices
		mIndexBuffer =
			Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
			Ogre::HardwareIndexBuffer::IT_32BIT,
			numEle,
			Ogre::HardwareBuffer::HBU_STATIC, true);

		mIndexBuffer->writeData(0,
			mIndexBuffer->getSizeInBytes(), // size
			indexbuffer,        // source
			true); // discard?

		delete []indexbuffer;

		// Set index buffer for this submesh
		mSubMesh->indexData->indexBuffer = mIndexBuffer;
		mSubMesh->indexData->indexStart = 0;
		mSubMesh->indexData->indexCount = numEle;
	}

	bool Mesh::update(const Image &HeigthMap)
	{
		if (HeigthMap.getType() != Image::TYPE_ONE_CHANNEL)
		{
			HydraxLOG("Error in Mesh::update, Image type isn't correct.");

			return false;
		}

		switch(mOptions->MeshType)
		{
		    case SIMPLE_GRID:
			{
				return _updateSimpleGridGeometry(HeigthMap);
			}
			break;

			case IMANTED_GRID:
			{
				return _updateImantedGridGeometry(HeigthMap);
			}
		}

		return false;
	}

    bool Mesh::_updateSimpleGridGeometry(const Image &HeigthMap)
    {
		SimpleGridOptions* SGOptions = static_cast<SimpleGridOptions*>(mOptions);

        float *posVertexBuffer =
            (float*)mPosVertexBuffer->
            lock(0,
                 mNumVertices,
                 Ogre::HardwareBuffer::HBL_NORMAL);

        int x, y, numPoint;
		float xcoord, ycoord;

		// y[1,...n-1] for set all borders to 0 heigth.
		for (y=1;y<=SGOptions->Complexity-1;y++)
        {
			ycoord = ((float)y/SGOptions->Complexity)*HeigthMap.getSize().Height;

            for (x=1;x<=SGOptions->Complexity-1;x++)
            {
				xcoord = ((float)x/SGOptions->Complexity)*HeigthMap.getSize().Width;

                numPoint = y*(SGOptions->Complexity+1) + x ;

				if (SGOptions->Complexity > HeigthMap.getSize().Width)
				{
                    posVertexBuffer[3*numPoint+1] = HeigthMap.getValueLI(xcoord,ycoord,0)*mStrength;
				}
				else
				{
					posVertexBuffer[3*numPoint+1] = HeigthMap.getValue(xcoord,ycoord,0)*mStrength;
				}
            }
        }

        mPosVertexBuffer->unlock();

		return true;
    }

	bool Mesh::_updateImantedGridGeometry(const Image &HeigthMap)
    {
		ImantedGridOptions* IGOptions = static_cast<ImantedGridOptions*>(mOptions);
		Ogre::Vector2 CurrentCameraPosition = Ogre::Vector2(mCamera->getPosition().x, mCamera->getPosition().z);
		Ogre::Vector2 CurrentPosition = getGridPosition(CurrentCameraPosition);

		int x, y, numPoint;
		float xcoord, ycoord;

		float *posVertexBuffer =
			(float*)mPosVertexBuffer->
			lock(0,
			mNumVertices,
			Ogre::HardwareBuffer::HBL_NORMAL);

		if((mLastCameraPosition-CurrentCameraPosition).length() > IGOptions->CameraDistance && CurrentPosition != Ogre::Vector2(-1,-1))
		{
			float *texcoordsBufData =
                (float*)mTexcoordsVertexBuffer->
                   lock(0,
                        mNumVertices,
						Ogre::HardwareBuffer::HBL_NORMAL);

			mLastCameraPosition = CurrentCameraPosition;
			
			Ogre::Vector2 OriginToCamera, MatrixPoint, NewMatrixPoint;
			float OriginToCameraLength, ProjectionLength, Transformer;

			for (y=1;y<=IGOptions->Complexity-1;y++)
			{
				for (x=1;x<=IGOptions->Complexity-1;x++)
				{
					numPoint = y*(IGOptions->Complexity+1) + x;

                    MatrixPoint = Ogre::Vector2(
						(float)(x) / (float)(IGOptions->Complexity),
						(float)(y) / (float)(IGOptions->Complexity));

					OriginToCamera = MatrixPoint-CurrentPosition;

					Ogre::Vector2 segmentoOrigenProjeccion = OriginToCamera.normalisedCopy()*200;
					
					Ogre::Vector2 Corner;
					Ogre::Vector2 CornerF;
					Ogre::Vector2 Dest = Ogre::Vector2::ZERO;

					for (int k = 0;k<2;k++)
					{
						Corner = Ogre::Vector2(k, k);

						CornerF = Ogre::Vector2(0, 1);
						Dest = Math::intersectionOfTwoLines(Corner,CornerF,segmentoOrigenProjeccion,MatrixPoint);
						if(Dest != Ogre::Vector2::ZERO)
						{
							break;
						}
						CornerF = Ogre::Vector2(1, 0);
						Dest = Math::intersectionOfTwoLines(Corner,CornerF,segmentoOrigenProjeccion,MatrixPoint);
						if(Dest != Ogre::Vector2::ZERO)
						{
							break;
						}
					}

                    ProjectionLength = (Dest-CurrentPosition).length();

					OriginToCameraLength = OriginToCamera.length();

					Transformer = Ogre::Math::Pow(OriginToCameraLength,IGOptions->ImanFactor)/Ogre::Math::Pow(ProjectionLength,IGOptions->ImanFactor-1);

					Ogre::Vector2 CameraToOriginUnitario = OriginToCamera.normalisedCopy();

					NewMatrixPoint = CameraToOriginUnitario*Transformer;
                    NewMatrixPoint += CurrentPosition;
                    NewMatrixPoint *= Ogre::Vector2(IGOptions->MeshSize.Width, IGOptions->MeshSize.Height);

					xcoord = (NewMatrixPoint.x/IGOptions->MeshSize.Width) * HeigthMap.getSize().Width;
					ycoord = (NewMatrixPoint.y/IGOptions->MeshSize.Height)* HeigthMap.getSize().Height;

					posVertexBuffer[3*numPoint+0] = NewMatrixPoint.x;
				    posVertexBuffer[3*numPoint+1] = HeigthMap.getValueLI(xcoord,ycoord,0)*mStrength;
					posVertexBuffer[3*numPoint+2] = NewMatrixPoint.y;

					// Inverted
					texcoordsBufData[2*numPoint+0] = NewMatrixPoint.y / IGOptions->MeshSize.Height ;
					texcoordsBufData[2*numPoint+1] = NewMatrixPoint.x / IGOptions->MeshSize.Width;
				}
			}
		
			mTexcoordsVertexBuffer->unlock();
		}
		else
		{
			for (y=1;y<=IGOptions->Complexity-1;y++)
			{
                for (x=1;x<=IGOptions->Complexity-1;x++)
                {
					numPoint = y*(IGOptions->Complexity+1) + x;

					xcoord = (posVertexBuffer[3*numPoint+0]/IGOptions->MeshSize.Width) *HeigthMap.getSize().Width ;
					ycoord = (posVertexBuffer[3*numPoint+2]/IGOptions->MeshSize.Height)*HeigthMap.getSize().Height;

					posVertexBuffer[3*numPoint+1] = HeigthMap.getValueLI(xcoord,ycoord,0)*mStrength;
				}
			}
		}

		mPosVertexBuffer->unlock();

		return true;
    }

	bool Mesh::updateProjectedGridGeometry(const int &numVer, void* verArray)
	{
		int MeshComplexity = static_cast<ProjectedGridOptions*>(mOptions)->Complexity;

		if (numVer != MeshComplexity*MeshComplexity)
		{
			return false;
		}

		if (verArray)
		{
		    mPosVertexBuffer->
			    writeData(0,
		        mPosVertexBuffer->getSizeInBytes(), // size
	            verArray,            // source
		        true); // discard?
		}

		return true;
	}

	bool Mesh::isPointInGrid(const Ogre::Vector2 &Position)
	{
		Ogre::AxisAlignedBox WordMeshBox = mEntity->getWorldBoundingBox();

		// Get our mesh grid rectangle:
		// c-----------d
		// |           |
		// |           |
		// |           |
		// a-----------b
		Ogre::Vector3
			a = WordMeshBox.getCorner(Ogre::AxisAlignedBox::FAR_LEFT_BOTTOM),
			b = WordMeshBox.getCorner(Ogre::AxisAlignedBox::FAR_RIGHT_BOTTOM),
			c = WordMeshBox.getCorner(Ogre::AxisAlignedBox::NEAR_RIGHT_BOTTOM),
			d = WordMeshBox.getCorner(Ogre::AxisAlignedBox::NEAR_LEFT_BOTTOM);

		// Transform all corners to Ogre::Vector2 array
		Ogre::Vector2 Corners2D[4] =
		   {Ogre::Vector2(a.x, a.z), 
		    Ogre::Vector2(b.x, b.z), 
		    Ogre::Vector2(c.x, c.z), 
		    Ogre::Vector2(d.x, d.z)};

		// Determinate if Position is into our rectangle, we use a line intersection detection
		// because our mesh rectangle can be rotated, if the number of collisions with the four
		// segments AB, BC, CD, DA is one, the Position point is into the rectangle, else(if number 
		// of collisions are 0 or 2, the Position point is outside the rectangle.
		int NumberOfCollisions = 0;
		// Find a point wich isn't be inside the rectangle
		Ogre::Vector2 DestPoint = Corners2D[0] + (Corners2D[1]-Corners2D[0])*2;
		for (int k = 0; k < 3; k++)
		{
			if (Math::intersectionOfTwoLines(Corners2D[k], Corners2D[k+1], Position, DestPoint) != Ogre::Vector2::ZERO)
			{
				NumberOfCollisions ++;
			}

			if (k == 2)
			{
				if (Math::intersectionOfTwoLines(Corners2D[3], Corners2D[0], Position, DestPoint) != Ogre::Vector2::ZERO)
			    {
				    NumberOfCollisions ++;
			    }
			}
		}
		if (NumberOfCollisions == 1)
		{
			return true;
		}

		return false;
	}

	Ogre::Vector2 Mesh::getGridPosition(const Ogre::Vector2 &Position)
	{
		if (getType() == PROJECTED_GRID)
		{
			return Position;
		}

		if (!isPointInGrid(Position))
		{
			return Ogre::Vector2(-1,-1);
		}

		Ogre::AxisAlignedBox WordMeshBox = mEntity->getWorldBoundingBox();

		// Get our mesh grid rectangle: (Only a,b,c corners)
		// c
		// |           
		// |           
		// |           
		// a-----------b
		Ogre::Vector3
			a = WordMeshBox.getCorner(Ogre::AxisAlignedBox::FAR_LEFT_BOTTOM),
			b = WordMeshBox.getCorner(Ogre::AxisAlignedBox::FAR_RIGHT_BOTTOM),
			c = WordMeshBox.getCorner(Ogre::AxisAlignedBox::NEAR_LEFT_BOTTOM);

		// Transform all corners to Ogre::Vector2 array
		Ogre::Vector2 Corners2D[3] =
		   {Ogre::Vector2(a.x, a.z), 
		    Ogre::Vector2(b.x, b.z), 
		    Ogre::Vector2(c.x, c.z)};

		// Get segments AB and AC
		Ogre::Vector2 AB = Corners2D[1]-Corners2D[0],
			          AC = Corners2D[2]-Corners2D[0];

		// Find the X/Y position projecting the Position point to AB and AC segments.
		Ogre::Vector2 XProjectedPoint = Position-AC,
			          YProjectedPoint = Position-AB;

		// Fint the intersections points
		Ogre::Vector2 XPoint = Math::intersectionOfTwoLines(Corners2D[0],Corners2D[1],Position,XProjectedPoint),
			          YPoint = Math::intersectionOfTwoLines(Corners2D[0],Corners2D[2],Position,YProjectedPoint);
		
		// Find lengths
		Ogre::Real ABLength = AB.length(),
			       ACLength = AC.length(),
				   XLength  = (XPoint-Corners2D[0]).length(),
				   YLength  = (YPoint-Corners2D[0]).length();

		// Find final x/y grid positions in [0,1] range
		Ogre::Real XFinal = XLength / ABLength,
			       YFinal = YLength / ACLength;

		return Ogre::Vector2(XFinal,YFinal);
	}

    void Mesh::setStrength(const Ogre::Real &Strength)
    {
		mStrength = Strength;

		if (getType() == PROJECTED_GRID)
		{
			return;
		}

        Ogre::AxisAlignedBox meshBounds(0,0,0,
		                             	mOptions->MeshSize.Width, mStrength, mOptions->MeshSize.Height);

        mMesh->_setBounds (meshBounds, false);
    }
}
