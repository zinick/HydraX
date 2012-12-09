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

#include "Hydrax.h"

namespace Hydrax
{

    Hydrax::Hydrax(Ogre::SceneManager *sm, Ogre::Camera *c)
            : mSceneManager(sm)
            , mCamera(c)
            , mCreated(false)
            , mPolygonMode(0)
			, mShaderMode(MaterialManager::SM_HLSL)
            , mPosition(Ogre::Vector3(0,0,0))
            , mPlanesError(0)
            , mFullReflectionDistance(99999997952.0)
            , mGlobalTransparency(0.05)
            , mNormalDistortion(0.09)
            , mSunPosition(Ogre::Vector3(5000,3000,1))
            , mSunStrength(1.75)
            , mSunArea(150)
            , mSunColor(Ogre::Vector3(1,0.75,0.25))
            , mFoamMaxDistance(75000000.0)
            , mFoamScale(0.0075)
            , mFoamStart(0)
            , mFoamTransparency(1)
            , mDepthLimit(0)
            , mDepthColor(Ogre::Vector3(0,0.1,0.172))
            , mSmoothPower(30)
            , mCausticsScale(20)
            , mCausticsPower(15)
            , mCausticsEnd(0.55)
            , mMesh(new Mesh(sm,c))
			, mTextureManager(new TextureManager(this))
			, mMaterialManager(new MaterialManager(this))
			, mModule(0)
            , mSceneNode(0)
            , mRefractionPlane(0)
            , mReflectionPlane(0)
            , mDepthPlane(0)
            , mTextureRefraction(0)
            , mTextureReflection(0)
            , mTextureDepth(0)
            , mPlanesSceneNode(0)
            , mComponents(HYDRAX_COMPONENTS_NONE)
    {
        HydraxLOG("Hydrax object created.");
    }

    Hydrax::~Hydrax()
    {
		if (mModule)
		{
           delete mModule;
		}

        if (mSceneNode)
        {
            mSceneNode->detachAllObjects();
            mSceneNode->getParentSceneNode()->removeAndDestroyChild(mSceneNode->getName());

			delete mTextureManager;
            delete mMesh;
        }

        if (mPlanesSceneNode)
        {
            mPlanesSceneNode->detachAllObjects();
            mPlanesSceneNode->getParentSceneNode()->removeAndDestroyChild(mPlanesSceneNode->getName());

            Ogre::RenderTarget* mRT_TextureRefraction = mTextureRefraction->getBuffer()->getRenderTarget();
            mRT_TextureRefraction->removeAllListeners();
            mRT_TextureRefraction->removeAllViewports();

            Ogre::RenderTarget* mRT_TextureReflection = mTextureReflection->getBuffer()->getRenderTarget();
            mRT_TextureReflection->removeAllListeners();
            mRT_TextureReflection->removeAllViewports();

            if (isComponent(HYDRAX_COMPONENT_DEPTH))
            {
                Ogre::RenderTarget* mRT_TextureDepth = mTextureDepth->getBuffer()->getRenderTarget();
                mRT_TextureDepth->removeAllListeners();
                mRT_TextureDepth->removeAllViewports();

                Ogre::TextureManager::getSingleton().remove("Depth");

                Ogre::MeshManager::getSingleton().remove("DepthClipPlane");
            }

            Ogre::TextureManager::getSingleton().remove("Reflection");
            Ogre::TextureManager::getSingleton().remove("Refraction");

            Ogre::MeshManager::getSingleton().remove("RefractionClipPlane");
            Ogre::MeshManager::getSingleton().remove("ReflectionClipPlane");
        }

        HydraxLOG("Hydrax object removed.");
    }

    void Hydrax::create()
    {
		if (mModule == NULL)
		{
			HydraxLOG("Module isn't set, skipping...");

            return;
		}

        if (isCreated())
        {
            HydraxLOG("Hydrax alredy created, skipping...");

            return;
        }

        HydraxLOG("Creating module.");
        mModule->create();
        HydraxLOG("Module created.");

		HydraxLOG("Creating materials,");
		mMaterialManager->createMaterials(mComponents, MaterialManager::Options(mShaderMode, mModule->getNormalMode()));
		mMesh->setMaterialName(mMaterialManager->getMaterial(MaterialManager::MAT_WATER)->getName());
		mMaterialManager->reload(MaterialManager::MAT_WATER);
		HydraxLOG("Materials created.");

        HydraxLOG("Creating water mesh.");
        mSceneNode = mSceneManager->getRootSceneNode()->createChildSceneNode();
        mMesh->create(mSceneNode);
        HydraxLOG("Water mesh created");

        HydraxLOG("Creating RTListeners.");
        _createRttListeners();
        HydraxLOG("RTListeners created");

		HydraxLOG("Registring device restored listener");
		mDeviceRestoredListener.mHydrax = this;
		Ogre::Root::getSingleton().getRenderSystem()->addListener(&mDeviceRestoredListener);
		HydraxLOG("Device restored listener registred");

        mCreated = true;
    }

	void Hydrax::DeviceRestoredListener::eventOccurred(const Ogre::String& eventName, const Ogre::NameValuePairList *parameters)
	{
		if (eventName == "DeviceRestored")
		{
			// Restore mesh
			HydraxLOG("Restoring water mesh.");
            mHydrax->setMeshOptions( mHydrax->getMeshOptions() );
			HydraxLOG("Water mesh restored.");
		}
	}

    void Hydrax::_createRttListeners()
    {
        if (!isCreated())
        {
            mRefractionPlane = new Ogre::MovablePlane("RefractionPlane");
            mReflectionPlane = new Ogre::MovablePlane("ReflectionPlane");

            mRefractionPlane->d = 0;
            mReflectionPlane->d = 0;

            mRefractionPlane->normal = Ogre::Vector3::NEGATIVE_UNIT_Y;
            mReflectionPlane->normal = Ogre::Vector3::UNIT_Y;

            Ogre::MeshManager::getSingleton().createPlane(
                "RefractionClipPlane",HYDRAX_RESOURCE_GROUP,
                *mRefractionPlane,
                mMesh->getSize().Width,mMesh->getSize().Height,
                10,10,true,1,5,5,Ogre::Vector3::UNIT_Z);

            Ogre::MeshManager::getSingleton().createPlane(
                "ReflectionClipPlane",HYDRAX_RESOURCE_GROUP,
                *mReflectionPlane,
                mMesh->getSize().Width,mMesh->getSize().Height,
                10,10,true,1,5,5,Ogre::Vector3::UNIT_Z);

            mRefractionPlane->setCastShadows(false);
            mReflectionPlane->setCastShadows(false);

            mPlanesSceneNode = mSceneManager->getRootSceneNode()->createChildSceneNode();
            mPlanesSceneNode->attachObject(mRefractionPlane);
            mPlanesSceneNode->attachObject(mReflectionPlane);

            mReflectionListener.mHydrax = this;
            mRefractionListener.mHydrax = this;

			mReflectionListener.mCReflectionQueueListener.mHydrax = this;
			mReflectionListener.mCReflectionQueueListener.mActive = false;
			mSceneManager->addRenderQueueListener(&mReflectionListener.mCReflectionQueueListener);
        }

        Ogre::TextureManager::getSingleton().remove("HydraxReflectionMap");
        Ogre::TextureManager::getSingleton().remove("HydraxRefractionMap");

        mTextureRefraction = Ogre::TextureManager::getSingleton().createManual(
                                 "HydraxRefractionMap",
                                 HYDRAX_RESOURCE_GROUP,
                                 Ogre::TEX_TYPE_2D,
                                 mRttOptions.RefractionQuality, mRttOptions.RefractionQuality, 0,
                                 Ogre::PF_R8G8B8,
                                 Ogre::TU_RENDERTARGET);

        Ogre::RenderTarget* mRT_TextureRefraction = mTextureRefraction->getBuffer()->getRenderTarget();
        {
            Ogre::Viewport *v = mRT_TextureRefraction->addViewport(mCamera);
            v->setClearEveryFrame(true);
            v->setBackgroundColour(Ogre::ColourValue::White);
            v->setOverlaysEnabled(false);

            mRT_TextureRefraction->addListener(&mRefractionListener);
        }

        mTextureReflection = Ogre::TextureManager::getSingleton().createManual(
                                 "HydraxReflectionMap",
                                 HYDRAX_RESOURCE_GROUP,
                                 Ogre::TEX_TYPE_2D,
                                 mRttOptions.ReflectionQuality, mRttOptions.ReflectionQuality, 0,
                                 Ogre::PF_R8G8B8,
                                 Ogre::TU_RENDERTARGET);

        Ogre::RenderTarget* mRT_TextureReflection = mTextureReflection->getBuffer()->getRenderTarget();
        {
            Ogre::Viewport *v = mRT_TextureReflection->addViewport(mCamera);
            v->setClearEveryFrame(true);
			v->setBackgroundColour(Ogre::ColourValue::White);
            v->setOverlaysEnabled(false);

            mRT_TextureReflection->addListener(&mReflectionListener);
        }

        if (isComponent(HYDRAX_COMPONENT_DEPTH))
        {
            _createDepthRttListener();
        }

		getMaterialManager()->reload(MaterialManager::MAT_WATER);
    }

    void Hydrax::_createDepthRttListener(const bool &Create, const bool &Delete)
    {
        if (Create)
        {
            Ogre::TextureManager::getSingleton().remove("HydraxDepthMap");

            if (!isCreated())
            {
                mDepthPlane = new Ogre::MovablePlane("DepthPlane");

                mDepthPlane->d = 0;
                mDepthPlane->normal = Ogre::Vector3::NEGATIVE_UNIT_Y;

                Ogre::MeshManager::getSingleton().createPlane(
                    "DepthClipPlane",HYDRAX_RESOURCE_GROUP,
                    *mDepthPlane,
                    mMesh->getSize().Width,mMesh->getSize().Height,
                    10,10,true,1,5,5,Ogre::Vector3::UNIT_Z);

                mDepthPlane->setCastShadows(false);

                mPlanesSceneNode->attachObject(mDepthPlane);

                mDepthListener.mHydrax = this;
            }

            mTextureDepth = Ogre::TextureManager::getSingleton().createManual(
                                "HydraxDepthMap",
                                HYDRAX_RESOURCE_GROUP,
                                Ogre::TEX_TYPE_2D,
                                mRttOptions.DepthQuality, mRttOptions.DepthQuality, 0,
                                Ogre::PF_R8G8B8,
                                Ogre::TU_RENDERTARGET);

            Ogre::RenderTarget* mRT_TextureDepth = mTextureDepth->getBuffer()->getRenderTarget();
            {
                Ogre::Viewport *v = mRT_TextureDepth->addViewport(mCamera);
                v->setClearEveryFrame(true);
                v->setBackgroundColour(Ogre::ColourValue::Black);
                v->setOverlaysEnabled(false);
                v->setMaterialScheme("HydraxDepth");
                v->setSkiesEnabled(false);

                mRT_TextureDepth->addListener(&mDepthListener);
            }

			getMaterialManager()->reload(MaterialManager::MAT_WATER);
        }
        if (Delete)
        {
            Ogre::RenderTarget* mRT_TextureDepth = mTextureDepth->getBuffer()->getRenderTarget();
            mRT_TextureDepth->removeAllListeners();
            mRT_TextureDepth->removeAllViewports();

            Ogre::TextureManager::getSingleton().remove("HydraxDepthMap");

            Ogre::MeshManager::getSingleton().remove("DepthClipPlane");
        }
    }

    void Hydrax::CReflectionListener::preRenderTargetUpdate(const Ogre::RenderTargetEvent& evt)
    {
		mCReflectionQueueListener.mActive = true;

        mHydrax->mMesh->getEntity()->setVisible(false);
        mHydrax->mReflectionPlane->getParentNode()->translate(0,mHydrax->mPlanesError,0);

		if (mHydrax->mCamera->getPosition().y < mHydrax->mReflectionPlane->getParentNode()->getPosition().y)
		{
			mCameraPlaneDiff = mHydrax->mReflectionPlane->getParentNode()->getPosition().y-mHydrax->mCamera->getPosition().y+0.05;
			mHydrax->mReflectionPlane->getParentNode()->translate(0,-mCameraPlaneDiff,0);
		}
		else
		{
			mCameraPlaneDiff = 0;
		}

        mHydrax->mCamera->enableReflection(mHydrax->mReflectionPlane);
        mHydrax->mCamera->enableCustomNearClipPlane(mHydrax->mReflectionPlane);
    }

    void Hydrax::CReflectionListener::postRenderTargetUpdate(const Ogre::RenderTargetEvent& evt)
    {
        mHydrax->mMesh->getEntity()->setVisible(true);

		if (mCameraPlaneDiff != 0)
		{
			mHydrax->mReflectionPlane->getParentNode()->translate(0,mCameraPlaneDiff,0);
		}

        mHydrax->mReflectionPlane->getParentNode()->translate(0,-mHydrax->mPlanesError,0);

        mHydrax->mCamera->disableReflection();
        mHydrax->mCamera->disableCustomNearClipPlane();

		mCReflectionQueueListener.mActive = false;
    }

    void Hydrax::CRefractionListener::preRenderTargetUpdate(const Ogre::RenderTargetEvent& evt)
    {
        mHydrax->mMesh->getEntity()->setVisible(false);
        mHydrax->mRefractionPlane->getParentNode()->translate(0,mHydrax->mPlanesError,0);

		if (mHydrax->mCamera->getPosition().y < mHydrax->mRefractionPlane->getParentNode()->getPosition().y)
		{
			mCameraPlaneDiff = mHydrax->mRefractionPlane->getParentNode()->getPosition().y-mHydrax->mCamera->getPosition().y+0.05;
			mHydrax->mRefractionPlane->getParentNode()->translate(0,-mCameraPlaneDiff,0);
		}
		else
		{
			mCameraPlaneDiff = 0;
		}

        mHydrax->mCamera->enableCustomNearClipPlane(mHydrax->mRefractionPlane);
    }

    void Hydrax::CRefractionListener::postRenderTargetUpdate(const Ogre::RenderTargetEvent& evt)
    {
        mHydrax->mMesh->getEntity()->setVisible(true);
        mHydrax->mRefractionPlane->getParentNode()->translate(0,-mHydrax->mPlanesError,0);

		if (mCameraPlaneDiff != 0)
		{
			mHydrax->mRefractionPlane->getParentNode()->translate(0,mCameraPlaneDiff,0);
		}

        mHydrax->mCamera->disableCustomNearClipPlane();
    }

    void Hydrax::CDepthListener::preRenderTargetUpdate(const Ogre::RenderTargetEvent& evt)
    {
        mHydrax->mMesh->getEntity()->setVisible(false);

        Ogre::SceneManager::MovableObjectIterator it = mHydrax->mSceneManager->getMovableObjectIterator("Entity");

        Ogre::Entity* cur;
        mMaterials.empty();

        while (it.hasMoreElements())
        {
            cur = dynamic_cast<Ogre::Entity*>(it.peekNextValue());
            mMaterials.push(cur->getSubEntity(0)->getMaterialName());

			cur->getSubEntity(0)->setMaterialName(mHydrax->getMaterialManager()->getMaterial(MaterialManager::MAT_DEPTH)->getName());

            it.moveNext();
        }

        mHydrax->mDepthPlane->getParentNode()->translate(0,mHydrax->mPlanesError,0);

		if (mHydrax->mCamera->getPosition().y < mHydrax->mDepthPlane->getParentNode()->getPosition().y)
		{
			mCameraPlaneDiff = mHydrax->mDepthPlane->getParentNode()->getPosition().y-mHydrax->mCamera->getPosition().y+0.05;
			mHydrax->mDepthPlane->getParentNode()->translate(0,-mCameraPlaneDiff,0);
		}
		else
		{
			mCameraPlaneDiff = 0;
		}

        mHydrax->mCamera->enableCustomNearClipPlane(mHydrax->mDepthPlane);
    }

    void Hydrax::CDepthListener::postRenderTargetUpdate(const Ogre::RenderTargetEvent& evt)
    {
        Ogre::SceneManager::MovableObjectIterator it = mHydrax->mSceneManager->getMovableObjectIterator("Entity");

        Ogre::Entity* cur;

        while (it.hasMoreElements())
        {
            std::string name = mMaterials.front();

            cur = dynamic_cast<Ogre::Entity*>(it.peekNextValue());
            if (Ogre::MaterialManager::getSingleton().resourceExists(name))
                cur->setMaterialName(name);

            mMaterials.pop();
            it.moveNext();
        }

        mHydrax->mMesh->getEntity()->setVisible(true);

        mHydrax->mCamera->disableCustomNearClipPlane();

		if (mCameraPlaneDiff != 0)
		{
			mHydrax->mDepthPlane->getParentNode()->translate(0,mCameraPlaneDiff,0);
		}

        mHydrax->mDepthPlane->getParentNode()->translate(0,-mHydrax->mPlanesError,0);
    }

	void Hydrax::setMeshOptions(Mesh::Options *Options)
    {
		if (mModule)
		{
			if (!mModule->isMeshGeometrySupported(Options->MeshType))
			{
				HydraxLOG("Actual module doesn't support this mesh grid geometry type. Skipping...");
				HydraxLOG("For change mesh/module follow these 3 steps:\n1) Call setModule(static_cast<Hydrax::Module*>(NULL))\n2) Set the new mesh options: setMeshOptions(...)\n3) Set your new module: setModule(...)");

				return;
			}
		}

        if (isCreated())
        {
            Ogre::String MaterialNameTmp = mMesh->getMaterialName();
			Ogre::Real StrengthTmp = mMesh->getStrength();

            HydraxLOG("Updating water mesh.");

            HydraxLOG("Deleting water mesh.");
            delete mMesh;
            HydraxLOG("Water mesh deleted.");

            HydraxLOG("Creating water mesh.");
            mMesh = new Mesh(mSceneManager, mCamera);
            mMesh->setOptions(Options);
            mMesh->setMaterialName(MaterialNameTmp);
            mMesh->create(mSceneNode);
            setPosition(mPosition);
			mMesh->setStrength(StrengthTmp);
			mModule->update(0); // ???
            HydraxLOG("Water mesh created");

            return;
        }

        mMesh->setOptions(Options);
    }

    void Hydrax::setRttOptions(const RttOptions &RttOptions)
    {
        mRttOptions = RttOptions;

        if (isCreated())
        {
            HydraxLOG("Updating Rtt options.");

            Ogre::RenderTarget* mRT_TextureRefraction = mTextureRefraction->getBuffer()->getRenderTarget();
            mRT_TextureRefraction->removeAllListeners();
            mRT_TextureRefraction->removeAllViewports();

            Ogre::RenderTarget* mRT_TextureReflection = mTextureReflection->getBuffer()->getRenderTarget();
            mRT_TextureReflection->removeAllListeners();
            mRT_TextureReflection->removeAllViewports();

            if (isComponent(HYDRAX_COMPONENT_DEPTH))
            {
                Ogre::RenderTarget* mRT_TextureDepth = mTextureDepth->getBuffer()->getRenderTarget();
                mRT_TextureDepth->removeAllListeners();
                mRT_TextureDepth->removeAllViewports();

                Ogre::TextureManager::getSingleton().remove("HydraxDepthMap");
            }

            Ogre::TextureManager::getSingleton().remove("HydraxReflectionMap");
            Ogre::TextureManager::getSingleton().remove("HydraxRefractionMap");

            _createRttListeners();

            HydraxLOG("Rtt options updated.");
        }
    }

    void Hydrax::setPolygonMode(const int &Tipe)
    {
        Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().getByName(mMesh->getMaterialName());

        if (Tipe == 0)
        {
            mat->getTechnique(0)->getPass(0)->setPolygonMode(Ogre::PM_SOLID);
        }
        else if (Tipe == 1)
        {
            mat->getTechnique(0)->getPass(0)->setPolygonMode(Ogre::PM_WIREFRAME);
        }
        else if (Tipe == 2)
        {
            mat->getTechnique(0)->getPass(0)->setPolygonMode(Ogre::PM_POINTS);
        }
    }

	void Hydrax::setShaderMode(const MaterialManager::ShaderMode& ShaderMode)
	{
		mShaderMode = ShaderMode;

		if (isCreated() && mModule)
		{
		    mMaterialManager->createMaterials(mComponents, MaterialManager::Options(mShaderMode, mModule->getNormalMode()));

		    mMesh->setMaterialName(mMaterialManager->getMaterial(MaterialManager::MAT_WATER)->getName());
		    mMaterialManager->reload(MaterialManager::MAT_WATER);
		}
	}

    void Hydrax::update(const Ogre::Real &timeSinceLastFrame)
    {
		if (mModule)
		{
            mModule->update(timeSinceLastFrame);
		}
    }

    void Hydrax::setComponents(const HydraxComponent &Components)
    {
        // Create/Delete depth rtt listeners if it's necesary
        if (isCreated())
        {
            if (isComponent(HYDRAX_COMPONENT_DEPTH))
            {
                if (!(Components & HYDRAX_COMPONENT_DEPTH))
                {
                    _createDepthRttListener(false, true);
                }
            }
            else
            {
                if (Components & HYDRAX_COMPONENT_DEPTH)
                {
                    _createDepthRttListener();
                }
            }
        }

        mComponents = Components;

        if (isComponent(HYDRAX_COMPONENT_SMOOTH) || isComponent(HYDRAX_COMPONENT_CAUSTICS))
        {
            if (!isComponent(HYDRAX_COMPONENT_DEPTH))
            {
                // Delete smooth or caustics components
				HydraxComponent s  = HYDRAX_COMPONENTS_NONE,
			                    f  = HYDRAX_COMPONENTS_NONE;

				if(isComponent(HYDRAX_COMPONENT_SUN))
				{
					s = HYDRAX_COMPONENT_SUN;
				}
				if(isComponent(HYDRAX_COMPONENT_FOAM))
				{
					f = HYDRAX_COMPONENT_FOAM;
				}

				if(isComponent(HYDRAX_COMPONENT_SMOOTH))
				{
					HydraxLOG("Smooth component needs depth component... smooth component desactivated.");
				}
				if(isComponent(HYDRAX_COMPONENT_CAUSTICS))
				{
					HydraxLOG("Caustics component needs depth component... cautics component desactivated.");
				}

		        mComponents = static_cast<HydraxComponent>(s|f);
            }
        }

		if (!isCreated() || !mModule)
		{
			return;
		}

		mMaterialManager->createMaterials(mComponents, MaterialManager::Options(mShaderMode, mModule->getNormalMode()));

		mMesh->setMaterialName(mMaterialManager->getMaterial(MaterialManager::MAT_WATER)->getName());
		mMaterialManager->reload(MaterialManager::MAT_WATER);
    }

	void Hydrax::setModule(Module::Module* Module)
	{
		if (mMesh && Module)
		{
			if (!Module->isMeshGeometrySupported(mMesh->getType()))
			{
				HydraxLOG("Actual mesh grid geometry type doesn't supported by " + Module->getName() + " module. Skipping...");
				HydraxLOG("For change mesh/module follow these 3 steps:\n1) Call setModule(static_cast<Hydrax::Module*>(NULL))\n2) Set the new mesh options: setMeshOptions(...)\n3) Set your new module: setModule(...)");

				return;
			}
		}

		if (mModule)
		{
			if (mModule->getNormalMode() != Module->getNormalMode())
			{
				mMaterialManager->createMaterials(mComponents, MaterialManager::Options(mShaderMode, Module->getNormalMode()));

		        mMesh->setMaterialName(mMaterialManager->getMaterial(MaterialManager::MAT_WATER)->getName());
		        mMaterialManager->reload(MaterialManager::MAT_WATER);
			}

			delete mModule;
		}

		mModule = Module;

		HydraxLOG("Module set.");
	}

    bool Hydrax::isComponent(const HydraxComponent &Component)
    {
        if (mComponents & Component)
        {
            return true;
        }

        if (Component == HYDRAX_COMPONENTS_NONE && mComponents == HYDRAX_COMPONENTS_NONE)
        {
            return true;
        }

        if (Component == HYDRAX_COMPONENTS_ALL && mComponents == HYDRAX_COMPONENTS_ALL)
        {
            return true;
        }

        return false;
    }

	void Hydrax::saveCfg(const Ogre::String &FileName)
	{
		FILE *cfgFile = fopen(FileName.c_str(), "w");

		if (cfgFile)
		{
			Ogre::String tmpStr = "";

			// Hydrax version
			tmpStr += "#Version\n";
			tmpStr += "HydraxVersion="+
				// Major
				Ogre::StringConverter::toString(HYDRAX_VERSION_MAJOR)+"."+
				// Minor
				Ogre::StringConverter::toString(HYDRAX_VERSION_MINOR)+"."+
				// Patch
				Ogre::StringConverter::toString(HYDRAX_VERSION_PATCH)+"\n\n";

		/*	// Mesh field
			tmpStr += "#Mesh\n";
			tmpStr += "MeshSize="+
				// X
				Ogre::StringConverter::toString(mMesh->getSize().Width)+"x"+
				// Z
				Ogre::StringConverter::toString(mMesh->getSize().Height)+"\n";
			tmpStr += "MeshComplexity="+Ogre::StringConverter::toString(mMesh->getComplexity())+"\n\n";
*/
			// Components field
			tmpStr += "#Components\n";
			tmpStr += "ComponentSun="     +Ogre::StringConverter::toString(isComponent(HYDRAX_COMPONENT_SUN     ))+"\n";
			tmpStr += "ComponentFoam="    +Ogre::StringConverter::toString(isComponent(HYDRAX_COMPONENT_FOAM    ))+"\n";
			tmpStr += "ComponentDepth="   +Ogre::StringConverter::toString(isComponent(HYDRAX_COMPONENT_DEPTH   ))+"\n";
			tmpStr += "ComponentSmooth="  +Ogre::StringConverter::toString(isComponent(HYDRAX_COMPONENT_SMOOTH  ))+"\n";
			tmpStr += "ComponentCaustics="+Ogre::StringConverter::toString(isComponent(HYDRAX_COMPONENT_CAUSTICS))+"\n\n";

			// Rtt quality field
			tmpStr += "#Rtt quality\n";
			tmpStr += "RttReflection="+Ogre::StringConverter::toString(static_cast<int>(getRttOptions().ReflectionQuality))+"\n";
			tmpStr += "RttRefraction="+Ogre::StringConverter::toString(static_cast<int>(getRttOptions().RefractionQuality))+"\n";
			tmpStr += "RttDepth="     +Ogre::StringConverter::toString(static_cast<int>(getRttOptions().DepthQuality))     +"\n\n";

			// Params fields
			// Main
			tmpStr += "#Main parameters\n";
			tmpStr += "FullReflectionDistance="+Ogre::StringConverter::toString(getFullReflectionDistance())+"\n";
			tmpStr += "GlobalTransparency="    +Ogre::StringConverter::toString(getGlobalTransparency()    )+"\n";
			tmpStr += "NormalDistortion="      +Ogre::StringConverter::toString(getNormalDistortion()      )+"\n\n";
			// Sun
			if (isComponent(HYDRAX_COMPONENT_SUN))
			{
				tmpStr += "#Sun parameters\n";
				tmpStr += "Sun_Position=" +
					Ogre::StringConverter::toString(getSunPosition().x)+"x"+
					Ogre::StringConverter::toString(getSunPosition().y)+"x"+
					Ogre::StringConverter::toString(getSunPosition().z)+"\n";
				tmpStr += "Sun_Strength="+Ogre::StringConverter::toString(getSunStrength())+"\n";
				tmpStr += "Sun_Area="    +Ogre::StringConverter::toString(getSunArea()    )+"\n";
				tmpStr += "Sun_Color=" +
					Ogre::StringConverter::toString(getSunColor().x)+"x"+
					Ogre::StringConverter::toString(getSunColor().y)+"x"+
					Ogre::StringConverter::toString(getSunColor().z)+"\n\n";
			}
			// Foam
			if (isComponent(HYDRAX_COMPONENT_FOAM))
			{
				tmpStr += "#Foam parameters\n";
				tmpStr += "Foam_MaxDistance=" +Ogre::StringConverter::toString(getFoamMaxDistance() )+"\n";
				tmpStr += "Foam_Scale="       +Ogre::StringConverter::toString(getFoamScale()       )+"\n";
				tmpStr += "Foam_Start="       +Ogre::StringConverter::toString(getFoamStart()       )+"\n";
				tmpStr += "Foam_Transparency="+Ogre::StringConverter::toString(getFoamTransparency())+"\n\n";
			}
			// Depth
			if (isComponent(HYDRAX_COMPONENT_DEPTH))
			{
				tmpStr += "#Depth parameters\n";
				tmpStr += "Depth_Limit=" +Ogre::StringConverter::toString(getDepthLimit())+"\n";
				tmpStr += "Depth_Color=" +
					Ogre::StringConverter::toString(getDepthColor().x)+"x"+
					Ogre::StringConverter::toString(getDepthColor().y)+"x"+
					Ogre::StringConverter::toString(getDepthColor().z)+"\n\n";
			}
			// Smooth
			if (isComponent(HYDRAX_COMPONENT_SMOOTH))
			{
				tmpStr += "#Smooth parameters\n";
				tmpStr += "Smooth_Power=" +Ogre::StringConverter::toString(getSmoothPower())+"\n\n";
			}
			// Caustics
			if (isComponent(HYDRAX_COMPONENT_CAUSTICS))
			{
				tmpStr += "#Caustics parameters\n";
				tmpStr += "Caustics_Scale=" +Ogre::StringConverter::toString(getCausticsScale())+"\n";
				tmpStr += "Caustics_Power=" +Ogre::StringConverter::toString(getCausticsPower())+"\n";
				tmpStr += "Caustics_End="   +Ogre::StringConverter::toString(getCausticsEnd())  +"\n\n";
			}

			// Save module config
			if (mModule)
			{
				mModule->saveCfg(tmpStr);
			}

			fprintf(cfgFile, "%s", tmpStr.c_str());
			fclose(cfgFile);

			try
			{
				Ogre::ResourceGroupManager::getSingleton().removeResourceLocation(FileName, HYDRAX_RESOURCE_GROUP);
			}
			catch(...)
			{
			}

			Ogre::ResourceGroupManager::getSingleton().addResourceLocation(FileName, "FileSystem", HYDRAX_RESOURCE_GROUP);
		}

		HydraxLOG(FileName + " saved.");
	}

	void Hydrax::loadCfg(const Ogre::String &FileName)
	{
		Ogre::ConfigFile CfgFile;

		CfgFile.load(FileName);

		// Check version
		if(CfgFile.getSetting("HydraxVersion") != (
			    // Major
				Ogre::StringConverter::toString(HYDRAX_VERSION_MAJOR)+"."+
				// Minor
				Ogre::StringConverter::toString(HYDRAX_VERSION_MINOR)+"."+
				// Patch
				Ogre::StringConverter::toString(HYDRAX_VERSION_PATCH)))
		{
			HydraxLOG("Config file version doesn't correspond with Hydrax version.");

			return;
		}
/*
		// Load mesh options
		setMeshOptions(
			 MeshOptions(
			    // Size
			    Size(// X
			         Ogre::StringConverter::parseInt(
			              Ogre::StringUtil::split(CfgFile.getSetting("MeshSize"), "x")[0]),
				     // Z
                     Ogre::StringConverter::parseInt(
			              Ogre::StringUtil::split(CfgFile.getSetting("MeshSize"), "x")[1])),
				// Complexity
				Ogre::StringConverter::parseInt(CfgFile.getSetting("MeshComplexity"))));
*/
		// Load components
		HydraxComponent s  = HYDRAX_COMPONENTS_NONE,
			            f  = HYDRAX_COMPONENTS_NONE,
						d  = HYDRAX_COMPONENTS_NONE,
						sm = HYDRAX_COMPONENTS_NONE,
						c  = HYDRAX_COMPONENTS_NONE;

		if (Ogre::StringConverter::parseBool(CfgFile.getSetting("ComponentSun")))
		{
			s = HYDRAX_COMPONENT_SUN;
		}
		if (Ogre::StringConverter::parseBool(CfgFile.getSetting("ComponentFoam")))
		{
			f = HYDRAX_COMPONENT_FOAM;
		}
		if (Ogre::StringConverter::parseBool(CfgFile.getSetting("ComponentDepth")))
		{
			d = HYDRAX_COMPONENT_DEPTH;
		}
		if (Ogre::StringConverter::parseBool(CfgFile.getSetting("ComponentSmooth")))
		{
			sm = HYDRAX_COMPONENT_SMOOTH;
		}
		if (Ogre::StringConverter::parseBool(CfgFile.getSetting("ComponentCaustics")))
		{
			c = HYDRAX_COMPONENT_CAUSTICS;
		}

		setComponents(static_cast<HydraxComponent>(s | f | d | sm | c));

		// Load Rtt options
		setRttOptions(
			RttOptions(
			   // Reflection
			   static_cast<TextureQuality>(Ogre::StringConverter::parseInt(CfgFile.getSetting("RttReflection"))),
			   // Refraction
			   static_cast<TextureQuality>(Ogre::StringConverter::parseInt(CfgFile.getSetting("RttRefraction"))),
			   // Depth
			   static_cast<TextureQuality>(Ogre::StringConverter::parseInt(CfgFile.getSetting("RttDepth")))));

		// Load params options
		// Main
		setFullReflectionDistance(Ogre::StringConverter::parseReal(CfgFile.getSetting("FullReflectionDistance")));
		setGlobalTransparency(Ogre::StringConverter::parseReal(CfgFile.getSetting("GlobalTransparency")));
		setNormalDistortion(Ogre::StringConverter::parseReal(CfgFile.getSetting("NormalDistortion")));
		// Sun
		if (isComponent(HYDRAX_COMPONENT_SUN))
		{
			setSunPosition(
				 Ogre::Vector3(
				     // X
			         Ogre::StringConverter::parseReal(
			              Ogre::StringUtil::split(CfgFile.getSetting("Sun_Position"), "x")[0]),
				     // Y
                     Ogre::StringConverter::parseReal(
			              Ogre::StringUtil::split(CfgFile.getSetting("Sun_Position"), "x")[1]),
					 // Z
                     Ogre::StringConverter::parseReal(
			              Ogre::StringUtil::split(CfgFile.getSetting("Sun_Position"), "x")[2])));

			setSunStrength(Ogre::StringConverter::parseReal(CfgFile.getSetting("Sun_Strength")));
			setSunArea(Ogre::StringConverter::parseReal(CfgFile.getSetting("Sun_Area")));

			setSunColor(
				 Ogre::Vector3(
				     // X
			         Ogre::StringConverter::parseReal(
			              Ogre::StringUtil::split(CfgFile.getSetting("Sun_Color"), "x")[0]),
				     // Y
                     Ogre::StringConverter::parseReal(
			              Ogre::StringUtil::split(CfgFile.getSetting("Sun_Color"), "x")[1]),
					 // Z
                     Ogre::StringConverter::parseReal(
			              Ogre::StringUtil::split(CfgFile.getSetting("Sun_Color"), "x")[2])));
		}
		// Foam
		if (isComponent(HYDRAX_COMPONENT_FOAM))
		{
			setFoamMaxDistance(Ogre::StringConverter::parseReal(CfgFile.getSetting("Foam_MaxDistance")));
			setFoamScale(Ogre::StringConverter::parseReal(CfgFile.getSetting("Foam_Scale")));
			setFoamStart(Ogre::StringConverter::parseReal(CfgFile.getSetting("Foam_Start")));
			setFoamTransparency(Ogre::StringConverter::parseReal(CfgFile.getSetting("Foam_Transparency")));
		}
		// Depth
		if (isComponent(HYDRAX_COMPONENT_DEPTH))
		{
			setDepthLimit(Ogre::StringConverter::parseReal(CfgFile.getSetting("Depth_Limit")));

			setDepthColor(
				 Ogre::Vector3(
				     // X
			         Ogre::StringConverter::parseReal(
			              Ogre::StringUtil::split(CfgFile.getSetting("Depth_Color"), "x")[0]),
				     // Y
                     Ogre::StringConverter::parseReal(
			              Ogre::StringUtil::split(CfgFile.getSetting("Depth_Color"), "x")[1]),
					 // Z
                     Ogre::StringConverter::parseReal(
			              Ogre::StringUtil::split(CfgFile.getSetting("Depth_Color"), "x")[2])));
		}
		// Depth
		if (isComponent(HYDRAX_COMPONENT_SMOOTH))
		{
			setSmoothPower(Ogre::StringConverter::parseReal(CfgFile.getSetting("Smooth_Power")));
		}
		// Caustics
		if (isComponent(HYDRAX_COMPONENT_CAUSTICS))
		{
			setCausticsScale(Ogre::StringConverter::parseReal(CfgFile.getSetting("Caustics_Scale")));
			setCausticsPower(Ogre::StringConverter::parseReal(CfgFile.getSetting("Caustics_Power")));
			setCausticsEnd(Ogre::StringConverter::parseReal(CfgFile.getSetting("Caustics_End")));
		}

		// Load module config
		if (mModule)
		{
			mModule->loadCfg(CfgFile);
		}

		HydraxLOG(FileName + " loaded.");
	}

    void Hydrax::setPosition(const Ogre::Vector3 &Position)
    {
        mPosition = Position;

		if (isComponent(HYDRAX_COMPONENT_DEPTH))
		{
		    mMaterialManager->setGpuProgramParameter(
			    MaterialManager::GPUP_VERTEX, MaterialManager::MAT_DEPTH,
			    "uPlaneYPos", Position.y);
		}

		if (isComponent(HYDRAX_COMPONENT_FOAM))
		{
		    mMaterialManager->setGpuProgramParameter(
			    MaterialManager::GPUP_FRAGMENT, MaterialManager::MAT_WATER,
			    "uFoamRange", Ogre::Vector2(Position.y, mMesh->getStrength()));
		}

        mSceneNode->setPosition(Position);
        mPlanesSceneNode->setPosition(Position);
    }

	void Hydrax::rotate(const Ogre::Quaternion &q)
	{
		mSceneNode->rotate(q);
		mPlanesSceneNode->rotate(q);
	}

    void Hydrax::setPlanesError(const Ogre::Real &PlanesError)
    {
        mPlanesError = PlanesError;
    }

    void Hydrax::setStrength(const Ogre::Real &Strength)
    {
        mMesh->setStrength(Strength);

		if (isComponent(HYDRAX_COMPONENT_FOAM))
		{
		    mMaterialManager->setGpuProgramParameter(
			    MaterialManager::GPUP_FRAGMENT, MaterialManager::MAT_WATER,
			    "uFoamRange", Ogre::Vector2(mPosition.y, Strength));
		}
    }

    void Hydrax::setFullReflectionDistance(const Ogre::Real &FullReflectionDistance)
    {
        mFullReflectionDistance = FullReflectionDistance;

		mMaterialManager->setGpuProgramParameter(
			MaterialManager::GPUP_FRAGMENT, MaterialManager::MAT_WATER,
			"uFullReflectionDistance", FullReflectionDistance);
    }

    void Hydrax::setGlobalTransparency(const Ogre::Real &GlobalTransparency)
    {
        mGlobalTransparency = GlobalTransparency;

		mMaterialManager->setGpuProgramParameter(
			MaterialManager::GPUP_FRAGMENT, MaterialManager::MAT_WATER,
			"uGlobalTransparency", GlobalTransparency);
    }

    void Hydrax::setNormalDistortion(const Ogre::Real &NormalDistortion)
    {
        mNormalDistortion = NormalDistortion;

		mMaterialManager->setGpuProgramParameter(
			MaterialManager::GPUP_FRAGMENT, MaterialManager::MAT_WATER,
			"uNormalDistortion", NormalDistortion);
    }

    void Hydrax::setSunPosition(const Ogre::Vector3 &SunPosition)
    {
        if (!isComponent(HYDRAX_COMPONENT_SUN))
        {
            return;
        }

        mSunPosition = SunPosition;

		mMaterialManager->setGpuProgramParameter(
			MaterialManager::GPUP_FRAGMENT, MaterialManager::MAT_WATER,
			"uSunPosition", SunPosition);
    }

    void Hydrax::setSunStrength(const Ogre::Real &SunStrength)
    {
        if (!isComponent(HYDRAX_COMPONENT_SUN))
        {
            return;
        }

        mSunStrength = SunStrength;

		mMaterialManager->setGpuProgramParameter(
			MaterialManager::GPUP_FRAGMENT, MaterialManager::MAT_WATER,
			"uSunStrength", SunStrength);
    }

    void Hydrax::setSunArea(const Ogre::Real &SunArea)
    {
        if (!isComponent(HYDRAX_COMPONENT_SUN))
        {
            return;
        }

        mSunArea = SunArea;

		mMaterialManager->setGpuProgramParameter(
			MaterialManager::GPUP_FRAGMENT, MaterialManager::MAT_WATER,
			"uSunArea", SunArea);
    }

	void Hydrax::setSunColor(const Ogre::Vector3 &SunColor)
    {
        if (!isComponent(HYDRAX_COMPONENT_SUN))
        {
            return;
        }

        mSunColor = SunColor;

		mMaterialManager->setGpuProgramParameter(
			MaterialManager::GPUP_FRAGMENT, MaterialManager::MAT_WATER,
			"uSunColor", SunColor);
    }

    void Hydrax::setFoamMaxDistance(const Ogre::Real &FoamMaxDistance)
    {
        if (!isComponent(HYDRAX_COMPONENT_FOAM))
        {
            return;
        }

        mFoamMaxDistance = FoamMaxDistance;

		mMaterialManager->setGpuProgramParameter(
			MaterialManager::GPUP_FRAGMENT, MaterialManager::MAT_WATER,
			"uFoamMaxDistance", FoamMaxDistance);
    }

    void Hydrax::setFoamScale(const Ogre::Real &FoamScale)
    {
        if (!isComponent(HYDRAX_COMPONENT_FOAM))
        {
            return;
        }

        mFoamScale = FoamScale;

		mMaterialManager->setGpuProgramParameter(
			MaterialManager::GPUP_FRAGMENT, MaterialManager::MAT_WATER,
			"uFoamScale", FoamScale);
    }

    void Hydrax::setFoamStart(const Ogre::Real &FoamStart)
    {
        if (!isComponent(HYDRAX_COMPONENT_FOAM))
        {
            return;
        }

        mFoamStart = FoamStart;

		mMaterialManager->setGpuProgramParameter(
			MaterialManager::GPUP_FRAGMENT, MaterialManager::MAT_WATER,
			"uFoamStart", FoamStart);
    }

    void Hydrax::setFoamTransparency(const Ogre::Real &FoamTransparency)
    {
        if (!isComponent(HYDRAX_COMPONENT_FOAM))
        {
            return;
        }

        mFoamTransparency = FoamTransparency;

		mMaterialManager->setGpuProgramParameter(
			MaterialManager::GPUP_FRAGMENT, MaterialManager::MAT_WATER,
			"uFoamTransparency", FoamTransparency);
    }

    void Hydrax::setDepthColor(const Ogre::Vector3 &DepthColor)
    {
        if (!isComponent(HYDRAX_COMPONENT_DEPTH))
        {
            return;
        }

        mDepthColor = DepthColor;

		mMaterialManager->setGpuProgramParameter(
			MaterialManager::GPUP_FRAGMENT, MaterialManager::MAT_WATER,
			"uDepthColor", DepthColor);
    }

    void Hydrax::setDepthLimit(const Ogre::Real &DepthLimit)
    {
        if (!isComponent(HYDRAX_COMPONENT_DEPTH))
        {
            return;
        }

		mDepthLimit = DepthLimit;

        if (mDepthLimit <= 0)
        {
            mDepthLimit = 1;
        }

		mMaterialManager->setGpuProgramParameter(
			MaterialManager::GPUP_FRAGMENT, MaterialManager::MAT_DEPTH,
			"uDepthLimit", 1/mDepthLimit);
    }

    void Hydrax::setSmoothPower(const Ogre::Real &SmoothPower)
    {
        if (!isComponent(HYDRAX_COMPONENT_SMOOTH))
        {
            return;
        }

        mSmoothPower = SmoothPower;

		mMaterialManager->setGpuProgramParameter(
			MaterialManager::GPUP_FRAGMENT, MaterialManager::MAT_WATER,
			"uSmoothPower", SmoothPower);
    }

    void Hydrax::setCausticsScale(const Ogre::Real &CausticsScale)
    {
        if (!isComponent(HYDRAX_COMPONENT_CAUSTICS))
        {
            return;
        }

        mCausticsScale = CausticsScale;

		mMaterialManager->setGpuProgramParameter(
			MaterialManager::GPUP_FRAGMENT, MaterialManager::MAT_DEPTH,
			"uCausticsScale", CausticsScale);
    }

    void Hydrax::setCausticsPower(const Ogre::Real &CausticsPower)
    {
        if (!isComponent(HYDRAX_COMPONENT_CAUSTICS))
        {
            return;
        }

        mCausticsPower = CausticsPower;

		mMaterialManager->setGpuProgramParameter(
			MaterialManager::GPUP_FRAGMENT, MaterialManager::MAT_WATER,
			"uCausticsPower", CausticsPower);
    }

    void Hydrax::setCausticsEnd(const Ogre::Real &CausticsEnd)
    {
        if (!isComponent(HYDRAX_COMPONENT_CAUSTICS))
        {
            return;
        }

        mCausticsEnd = CausticsEnd;

		mMaterialManager->setGpuProgramParameter(
			MaterialManager::GPUP_FRAGMENT, MaterialManager::MAT_WATER,
			"uCausticsEnd", CausticsEnd);
    }
}
