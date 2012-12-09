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

#ifndef _Hydrax_Hydrax_H_
#define _Hydrax_Hydrax_H_

#include "Prerequisites.h"

#include "Enums.h"
#include "Structs.h"
#include "Help.h"
#include "Mesh.h"
#include "Image.h"
#include "MaterialManager.h"
#include "TextureManager.h"
#include "Modules/Module.h"

namespace Hydrax
{
    /** Main Hydrax class. 
	    Hydrax is a plugin for the Ogre3D engine whose aim is rendering realistic water scenes.
		Do not use two instances of the Hydrax class.
     */
    class DllExport Hydrax
    {
    public:
        /** Constructor
            @param sm Ogre SceneManager pointer
            @param c Ogre Camera pointer
         */
        Hydrax(Ogre::SceneManager *sm, Ogre::Camera *c);

        /** Destructor
         */
        ~Hydrax();

        /** Call when all params are set, creates grid water
            and perlin / normal map textures
            @remarks Call when all params are set
         */
        void create();

        /** Call every frame
            @todo Add listener interface
         */
        void update(const Ogre::Real &timeSinceLastFrame);

        /** Returns if the especified component is active
            @param Component Component that we want to check
         */
        bool isComponent(const HydraxComponent &Component);

        /** Set mesh options
            @param Options Mesh::Options base class pointer
            @remarks It can be called after create(), Mesh will be updated
         */
		void setMeshOptions(Mesh::Options *Options);

        /** Set Rtt options
            @param RttOptions Options
            @remarks It can be called after create(), Rtt's will be updated
         */
        void setRttOptions(const RttOptions &RttOptions);

        /** Set Hydrax components
            @param Components Components
            @remarks It can be called after create(), Components will be updated
         */
        void setComponents(const HydraxComponent &Components);

		/** Set Hydrax module
		    @param Module Hydrax module
			@remark Module will be set before call create()
		 */
		void setModule(Module::Module* Module);

        /** Set polygon mode: 1-Solid, 2-Wireframe, 3-Points
            @param Time polygon mode: 1-Solid, 2-Wireframe, 3-Points
         */
        void setPolygonMode(const int &Tipe);

		/** Set shader mode
		    @param ShaderMode Shader mode
		 */
		void setShaderMode(const MaterialManager::ShaderMode &ShaderMode);

        /** Set water position
            @param Position Water position
         */
        void setPosition(const Ogre::Vector3 &Position);

		/** Rotate water and planes
		    @param q const Ogre::Quaternion&
		*/
		void rotate(const Ogre::Quaternion &q);

		/** Save hydrax config to file
		    @param FileName File name
			@remarks If module isn't set, module options won't be saved.
		 */
		void saveCfg(const Ogre::String &FileName);

		/** Load config from file
		    @param FileName File name
			@remarks if module isn't set, or module isn't the same from
			         config file, module options won't be loaded.
		 */
		void loadCfg(const Ogre::String &FileName);

        /** Set clip planes error
            @param PlanesError Clip planes error
         */
        void setPlanesError(const Ogre::Real &PlanesError);

        /** Set water strength
            @param Strength Water strength
         */
        void setStrength(const Ogre::Real &Strength);

        /** Set full reflection distance
            @param FullReflectionDistance Full reflection distance
         */
        void setFullReflectionDistance(const Ogre::Real &FullReflectionDistance);

        /** Set global transparency
            @param GlobalTransparency Global transparency distance
         */
        void setGlobalTransparency(const Ogre::Real &GlobalTransparency);

        /** Set normal distortion
            @param NormalDistortion Normal distortion
            @remarks Value will bi very short, like 0.025
         */
        void setNormalDistortion(const Ogre::Real &NormalDistortion);

        /** Set sun position
            @param SunPosition Sun position
         */
        void setSunPosition(const Ogre::Vector3 &SunPosition);

        /** Set sun strength
            @param SunStrength Sun strength
         */
        void setSunStrength(const Ogre::Real &SunStrength);

        /** Set sun area
            @param SunArea Sun area
         */
        void setSunArea(const Ogre::Real &SunArea);

        /** Set sun color
            @param SunColor Sun color
         */
        void setSunColor(const Ogre::Vector3 &SunColor);

        /** Set foam max distance
            @param FoamMaxDistance Foam max distance
         */
        void setFoamMaxDistance(const Ogre::Real &FoamMaxDistance);

        /** Set foam scale
            @param FoamScale Foam scale
         */
        void setFoamScale(const Ogre::Real &FoamScale);

        /** Set foam start
            @param FoamStart Foam start
         */
        void setFoamStart(const Ogre::Real &FoamStart);

        /** Set foam transparency
            @param FoamTransparency Foam transparency
         */
        void setFoamTransparency(const Ogre::Real &FoamTransparency);

        /** Set depth limit
            @param DepthLimit Depth limit
         */
        void setDepthLimit(const Ogre::Real &DepthLimit);

        /** Set depth color
            @param DepthColor Foam color
         */
        void setDepthColor(const Ogre::Vector3 &DepthColor);

        /** Set smooth power
            @param SmoothPower Smooth power
            @remarks Less values more transition distance, hight values short transition values, 1-50 range(aprox.)
         */
        void setSmoothPower(const Ogre::Real &SmoothPower);

        /** Set caustics scale
            @param CausticsScale Caustics scale
         */
        void setCausticsScale(const Ogre::Real &CausticsScale);

        /** Set caustics power
            @param CausticsPower Caustics power
         */
        void setCausticsPower(const Ogre::Real &CausticsPower);

        /** Set caustics end
            @param CausticsEnd Caustics end
         */
        void setCausticsEnd(const Ogre::Real &CausticsEnd);

        /** Has create() already called?
            @return Has create() already called?
         */
        inline const bool& isCreated() const
        {
            return mCreated;
        }

		/** Get rendering camera
		    @return Ogre::Camera pointer
		 */
		inline Ogre::Camera* getCamera()
		{
			return mCamera;
		}

		/** Get scene manager
		    @return Ogre::SceneManager pointer
		 */
		inline Ogre::SceneManager* getSceneManager()
		{
			return mSceneManager;
		}

		/** Get Hydrax::Mesh
		    @return Hydrax::Mesh pointer
		 */
		inline Mesh* getMesh()
		{
			return mMesh;
		}

		/** Get Hydrax::MaterialManager
		    @return Hydrax::MaterialManager pointer
		 */
		inline MaterialManager* getMaterialManager()
		{
			return mMaterialManager;
		}

		/** Get Hydrax::TextureManager
		    @return Hydrax::TextureManager pointer
		 */
		inline TextureManager* getTextureManager()
		{
			return mTextureManager;
		}

		/** Get our Hydrax::Module::Module
		    @return Hydrax::Module::Module pointer or NULL if Module isn't set.
		 */
		inline Module::Module* getModule()
		{
			return mModule;
		}

        /** Get mesh options
            @return Mesh::Options base class pointer
         */
		inline Mesh::Options *getMeshOptions()
        {
            return mMesh->getOptions();
        }

        /** Get rtt options
            @return Hydrax rtt options
         */
        inline const RttOptions& getRttOptions() const
        {
            return mRttOptions;
        }

        /** Get hydrax components selected
            @return Hydrax components
         */
        inline const HydraxComponent& getComponents() const
        {
            return mComponents;
        }

        /** Get current polygon mode
            @return Current polygon mode: 1-Solid, 2-Wireframe, 3-Points
         */
        inline const int& getPolygonMode() const
        {
            return mPolygonMode;
        }

		/** Get current shader mode
		    @return Current shader mode
		 */
		inline const MaterialManager::ShaderMode& getShaderMode() const
		{
			return mShaderMode;
		}

        /** Get water position
            @return Water position
         */
        inline const Ogre::Vector3& getPosition() const
        {
            return mPosition;
        }

		/** Get the current heigth at a especified world-space point
		    @param Position X/Z World position
			@return Heigth at the given position in y-World coordinates, if it's outside of the water return -1
		 */
		inline float getHeigth(const Ogre::Vector2 &Position)
		{
			if (mModule)
			{
				return mModule->getHeigth(Position);
			}

			return -1;
		}

		/** Get the current heigth at a especified world-space point
		    @param Position X/(Y)/Z World position
			@return Heigth at the given position in y-World coordinates, if it's outside of the water return -1
		 */
		inline float getHeigth(const Ogre::Vector3 &Position)
		{
			return getHeigth(Ogre::Vector2(Position.x, Position.z));
		}

        /** Get water strength
            @return Hydrax water strength
         */
        inline const Ogre::Real& getStrength() const
        {
            return mMesh->getStrength();
        }

        /** Get full reflection distance
            @return Hydrax water full reflection distance
         */
        inline const Ogre::Real& getFullReflectionDistance() const
        {
            return mFullReflectionDistance;
        }

        /** Get global transparency
            @return Hydrax water global transparency
         */
        inline const Ogre::Real& getGlobalTransparency() const
        {
            return mGlobalTransparency;
        }

        /** Get sun position
            @return Sun position
         */
        inline const Ogre::Vector3& getSunPosition() const
        {
            return mSunPosition;
        }

        /** Get normal distortion
            @return Hydrax normal distortion
         */
        inline const Ogre::Real& getNormalDistortion() const
        {
            return mNormalDistortion;
        }

        /** Get water strength
            @return Hydrax water strength
         */
        inline const Ogre::Real& getSunStrength() const
        {
            return mSunStrength;
        }

        /** Get sun area
            @return Sun area
         */
        inline const Ogre::Real& getSunArea() const
        {
            return mSunArea;
        }

        /** Get sun color
            @return Sun color
         */
        inline const Ogre::Vector3& getSunColor() const
        {
            return mSunColor;
        }

        /** Get foam max distance
            @return Foam max distance
         */
        inline const Ogre::Real& getFoamMaxDistance() const
        {
            return mFoamMaxDistance;
        }

        /** Get foam scale
            @return Foam scale
         */
        inline const Ogre::Real& getFoamScale() const
        {
            return mFoamScale;
        }

        /** Get foam start
            @return Foam start
         */
        inline const Ogre::Real& getFoamStart() const
        {
            return mFoamStart;
        }

        /** Get foam transparency
            @return Foam scale
         */
        inline const Ogre::Real& getFoamTransparency() const
        {
            return mFoamTransparency;
        }

        /** Get depth limit
            @return Depth limit
         */
        inline const Ogre::Real& getDepthLimit() const
        {
            return mDepthLimit;
        }

        /** Get depth color
            @return Depth color
         */
        inline const Ogre::Vector3& getDepthColor() const
        {
            return mDepthColor;
        }

        /** Get smooth power
            @return Smooth power
         */
        inline const Ogre::Real& getSmoothPower() const
        {
            return mSmoothPower;
        }

		/** Get caustics scale
            @return Caustics scale
         */
        inline const Ogre::Real& getCausticsScale() const
        {
            return mCausticsScale;
        }

        /** Get caustics power
            @return Caustics power
         */
        inline const Ogre::Real& getCausticsPower() const
        {
            return mCausticsPower;
        }

        /** Get caustics end
            @return Caustics end
         */
        inline const Ogre::Real& getCausticsEnd() const
        {
            return mCausticsEnd;
        }

    private:

    /** Restore water mesh after device listener restored event
	 */
	class DllExport DeviceRestoredListener : public Ogre::RenderSystem::Listener
	    {
		public:
			/// Hydrax manager pointer
            Hydrax* mHydrax;

			/** Event occurred
			    @param eventName Name of the event
				@param parameters Ogre::NameValuePairList pointer
			 */
			void eventOccurred(const Ogre::String& eventName, const Ogre::NameValuePairList *parameters);
		};

    /** Hydrax::CRefractionListener class
     */
    class DllExport CRefractionListener : public Ogre::RenderTargetListener
        {
        public:
            /// Hydrax manager pointer
            Hydrax* mHydrax;

			/// Camera.y - Plane.y difference
		    Ogre::Real mCameraPlaneDiff;

            /** Funtion that is called before the Rtt will render
                @param evt Ogre RenderTargetEvent
                @remarks We've to override it
             */
            void preRenderTargetUpdate(const Ogre::RenderTargetEvent& evt);

            /** Funtion that is called after the Rtt will render
                @param evt Ogre RenderTargetEvent
                @remarks We've to override it
             */
            void postRenderTargetUpdate(const Ogre::RenderTargetEvent& evt);
        };

    /** Hydrax::CReflectionListener class
     */
    class DllExport CReflectionListener : public Ogre::RenderTargetListener
        {
        public:
			/** Hydrax::CReflectionListener::CReflectionQueueListener class
			    Used for avoid near clip plane clipping during the reflection Rtt
			 */
			class CReflectionQueueListener : public Ogre::RenderQueueListener
			{
			public:
				/** Called at the start of the queue
				 */
				void renderQueueStarted(Ogre::uint8 queueGroupId, const Ogre::String &invocation, bool &skipThisInvocation) 
				{
					if ((queueGroupId == Ogre::RENDER_QUEUE_SKIES_EARLY || queueGroupId == Ogre::RENDER_QUEUE_SKIES_LATE) 
						&& mActive)
					{
						mHydrax->getCamera()->disableCustomNearClipPlane();
						Ogre::Root::getSingleton().getRenderSystem()->_setProjectionMatrix(mHydrax->getCamera()->getProjectionMatrixRS()); 
					}
				}

				/** Called on the end of the queue
				 */
				void renderQueueEnded(Ogre::uint8 queueGroupId, const Ogre::String &invocation, bool &skipThisInvocation) 
				{
					if ((queueGroupId == Ogre::RENDER_QUEUE_SKIES_EARLY || queueGroupId == Ogre::RENDER_QUEUE_SKIES_LATE) 
						&& mActive)
					{
						mHydrax->getCamera()->enableCustomNearClipPlane(mHydrax->mReflectionPlane);
						Ogre::Root::getSingleton().getRenderSystem()->_setProjectionMatrix(mHydrax->getCamera()->getProjectionMatrixRS()); 
					}
				}

				/// Hydrax pointer
				Hydrax* mHydrax;
				/// Is the reflection Rtt active?
				bool mActive;
			};

			/// CReflectionQueueListener
			CReflectionQueueListener mCReflectionQueueListener;

            /// Hydrax manager pointer
            Hydrax* mHydrax;

			/// Camera.y - Plane.y difference
		    Ogre::Real mCameraPlaneDiff;

            /** Funtion that is called before the Rtt will render
                @param evt Ogre RenderTargetEvent
                @remarks We've to override it
             */
            void preRenderTargetUpdate(const Ogre::RenderTargetEvent& evt);

            /** Funtion that is called after the Rtt will render
                @param evt Ogre RenderTargetEvent
                @remarks We've to override it
             */
            void postRenderTargetUpdate(const Ogre::RenderTargetEvent& evt);
        };

    /** Hydrax::CDepthListener class
     */
    class DllExport CDepthListener : public Ogre::RenderTargetListener
        {
        public:
            /// Hydrax manager pointer
            Hydrax* mHydrax;

            /// std::string to store entity's original materials name
            std::queue<std::string> mMaterials;

			/// Camera.y - Plane.y difference
		    Ogre::Real mCameraPlaneDiff;

            /** Funtion that is called before the Rtt will render
                @param evt Ogre RenderTargetEvent
                @remarks We've to override it
             */
            void preRenderTargetUpdate(const Ogre::RenderTargetEvent& evt);

            /** Funtion that is called after the Rtt will render
                @param evt Ogre RenderTargetEvent
                @remarks We've to override it
             */
            void postRenderTargetUpdate(const Ogre::RenderTargetEvent& evt);
        };

        /** Update normal map textures
         */
        void _updateNM();

        /** Create all rtt listeners
         */
        void _createRttListeners();

        /** Create/Delete depth rtt listener
            @param Create Create depth rtt listener?
            @param Delete Delete depth rtt listener?
            @remarks Don't call with (Delete = true) if before we haven't call it with (Create = true)
         */
        void _createDepthRttListener(const bool &Create = true, const bool &Delete = false);

        /// Has create() already called?
        bool mCreated;

        /// Rtt options
        RttOptions mRttOptions;
        /// Hydrax components
        HydraxComponent mComponents;
		/// Current shader mode
		MaterialManager::ShaderMode mShaderMode;

		/// Restored device listener
		DeviceRestoredListener mDeviceRestoredListener;

        /// Polygon mode: 1-Solid, 2-Wireframe, 3-Points
        int mPolygonMode;
        /// Water position
        Ogre::Vector3 mPosition;
        /// Planes error, y axis clipplanes displacement
        Ogre::Real mPlanesError;

        /// Full reflection distance param
        Ogre::Real mFullReflectionDistance;
        /// Global transparency param
        Ogre::Real mGlobalTransparency;
        /// Normal distortion param
        Ogre::Real mNormalDistortion;

        /// Sun position
        Ogre::Vector3 mSunPosition;
        /// Sun strength param
        Ogre::Real mSunStrength;
        /// Sun area
        Ogre::Real mSunArea;
        /// Sun color
        Ogre::Vector3 mSunColor;

        /// Foam max distance param
        Ogre::Real mFoamMaxDistance;
        /// Foam scale param
        Ogre::Real mFoamScale;
        /// Foam start param
        Ogre::Real mFoamStart;
        /// Foam transparency param
        Ogre::Real mFoamTransparency;

        /// Depth limit param
        Ogre::Real mDepthLimit;
        /// Depth color param
        Ogre::Vector3 mDepthColor;

        /// Smooth power param
        Ogre::Real mSmoothPower;

        /// Caustics scale param
        Ogre::Real mCausticsScale;
        /// Caustics power
        Ogre::Real mCausticsPower;
        /// Caustics end
        Ogre::Real mCausticsEnd;

        /// Our Hydrax::Mesh pointer
        Mesh *mMesh;
		/// Our Hydrax::MaterialManager
		MaterialManager *mMaterialManager;
		/// Our Hydrax::TextureManager pointer
		TextureManager *mTextureManager;
		/// Our Hydrax::Module::Module pointer
		Module::Module *mModule;

        /// Refraction Plane pointer
        Ogre::MovablePlane* mRefractionPlane;
        /// Reflection Plane pointer
        Ogre::MovablePlane* mReflectionPlane;
        /// Depth Plane pointer
        Ogre::MovablePlane* mDepthPlane;

        /// Refraction texture
        Ogre::TexturePtr mTextureRefraction;
        /// Reflection texture
        Ogre::TexturePtr mTextureReflection;
        /// Depth texture
        Ogre::TexturePtr mTextureDepth;

        /// Refraction listener
        CRefractionListener mRefractionListener;
        /// Reflection listener
        CReflectionListener mReflectionListener;
        /// Depth listener
        CDepthListener mDepthListener;

        /// SceneNode to attach our Hydrax::Mesh entity
        Ogre::SceneNode *mSceneNode;
        /// SceneNode to attach our planes
        Ogre::SceneNode *mPlanesSceneNode;

        /// Pointer to Ogre::SceneManager
        Ogre::SceneManager *mSceneManager;
        /// Pointer to Ogre::Camera
        Ogre::Camera *mCamera;
    };
}

#endif
