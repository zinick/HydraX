// ----------------------------------------------------------------------------
// Include the main OGRE header files
// Ogre.h just expands to including lots of individual OGRE header files
// ----------------------------------------------------------------------------
#include <Ogre.h>
// ----------------------------------------------------------------------------
// Include the OGRE example framework
// This includes the classes defined to make getting an OGRE application running
// a lot easier. It automatically sets up all the main objects and allows you to
// just override the bits you want to instead of writing it all from scratch.
// ----------------------------------------------------------------------------
#include <ExampleApplication.h>

// ----------------------------------------------------------------------------
// Include the Hydrax plugin headers
// Main base headers (Hydrax.h) and especific headers (Noise/Water modules)
// ----------------------------------------------------------------------------
#include "Hydrax.h"
#include "Noise/Perlin/Perlin.h"
#include "Modules/ProjectedGrid/ProjectedGrid.h"

#define _def_SkyBoxNum 5
#define _def_PGComplexity 256

Hydrax::Hydrax *mHydrax = 0;

Ogre::String mSkyBoxes[_def_SkyBoxNum] = {
	                         "Sky/ClubTropicana",
                             "Sky/Stormy",
                             "Sky/EarlyMorning",
                             "Sky/Evening",
                             "Sky/Clouds"};

Ogre::Vector3 mSunPosition[_def_SkyBoxNum] = {
	                             Ogre::Vector3(0,10000,-90000),
                                 Ogre::Vector3(0,10000,-90000)/2.5,
                                 Ogre::Vector3(13000,0,120000)/3,
                                 Ogre::Vector3(-50000,-5000,50000),
								 Ogre::Vector3(0,0,0)};

Ogre::Vector3 mSunColor[_def_SkyBoxNum] = {
	                          Ogre::Vector3(1, 0.9, 0.6)/5,
                              Ogre::Vector3(0.75, 0.65, 0.45)/2,
                              Ogre::Vector3(1,0.6,0.4),
                              Ogre::Vector3(1,0.4,0.1),
							  Ogre::Vector3(0,0,0)};

Hydrax::Module::ProjectedGrid::Options mPGOptions[_def_SkyBoxNum] = {
	                          Hydrax::Module::ProjectedGrid::Options(_def_PGComplexity),
	                          Hydrax::Module::ProjectedGrid::Options(_def_PGComplexity, 45.5f, 7.0f, false),
	                          Hydrax::Module::ProjectedGrid::Options(_def_PGComplexity, 32.5f, 7.0f, false),
	                          Hydrax::Module::ProjectedGrid::Options(_def_PGComplexity, 32.5f, 7.0f, false),
	                          Hydrax::Module::ProjectedGrid::Options(_def_PGComplexity, 20.0f, 7.0f, false)};

Hydrax::Noise::Perlin::Options mPerlinOptions[_def_SkyBoxNum] = {
							  Hydrax::Noise::Perlin::Options(8, 0.085f, 0.49f, 1.4f, 1.27),
							  Hydrax::Noise::Perlin::Options(8, 0.085f, 0.49f, 1.4f, 1.27),
							  Hydrax::Noise::Perlin::Options(8, 0.085f, 0.49f, 1.4f, 1.27),
							  Hydrax::Noise::Perlin::Options(8, 0.075f, 0.49f, 1.4f, 1.27),
							  Hydrax::Noise::Perlin::Options(8, 0.085f, 0.49f, 1.4f, 1.27)};

int mCurrentSkyBox = 0;

// ----------------------------------------------------------------------------
// Define the application object
// This is derived from ExampleApplication which is the class OGRE provides to
// make it easier to set up OGRE without rewriting the same code all the time.
// You can override extra methods of ExampleApplication if you want to further
// specialise the setup routine, otherwise the only mandatory override is the
// 'createScene' method which is where you set up your own personal scene.
// ----------------------------------------------------------------------------

class ExampleHydraxDemoListener : public ExampleFrameListener
{
public:
    SceneManager *mSceneMgr;
    Real mKeyBuffer;

    ExampleHydraxDemoListener(RenderWindow* win, Camera* cam, SceneManager *sm)
            : ExampleFrameListener(win,cam)
            , mSceneMgr(sm)
            , mKeyBuffer(-1)
    {
    }

    bool frameStarted(const FrameEvent &e)
    {
        mHydrax->update(e.timeSinceLastFrame);

        mKeyboard->capture();

        if (mKeyboard->isKeyDown(OIS::KC_M) && mKeyBuffer < 0)
        {
			mCurrentSkyBox++;

			if(mCurrentSkyBox > (_def_SkyBoxNum-1))
			{
			    mCurrentSkyBox = 0;
			}

			changeSkyBox();

			mKeyBuffer = 0.5f;
		}

		mKeyBuffer -= e.timeSinceLastFrame;

        return true;
    }

    void changeSkyBox()
    {
        // Change skybox
        mSceneMgr->setSkyBox(true, mSkyBoxes[mCurrentSkyBox], 99999*3, true);

        // Update Hydrax sun position and colour
        mHydrax->setSunPosition(mSunPosition[mCurrentSkyBox]);
        mHydrax->setSunColor(mSunColor[mCurrentSkyBox]);

        // Update light 0 light position and colour
        mSceneMgr->getLight("Light0")->setPosition(mSunPosition[mCurrentSkyBox]);
        mSceneMgr->getLight("Light0")->setSpecularColour(mSunColor[mCurrentSkyBox].x,mSunColor[mCurrentSkyBox].y,mSunColor[mCurrentSkyBox].z);

		// Update perlin noise options
		static_cast<Hydrax::Noise::Perlin*>(mHydrax->getModule()->getNoise())
			->setOptions(mPerlinOptions[mCurrentSkyBox]);

		// Update projected grid options
		static_cast<Hydrax::Module::ProjectedGrid*>(mHydrax->getModule())
			->setOptions(mPGOptions[mCurrentSkyBox]);

        LogManager::getSingleton().logMessage("Skybox " + mSkyBoxes[mCurrentSkyBox] + " selected. ("+Ogre::StringConverter::toString(mCurrentSkyBox+1)+"/"+Ogre::StringConverter::toString(_def_SkyBoxNum)+")");
    }
};

class SampleApp : public ExampleApplication
{
public:
    // Basic constructor
    SampleApp()
    {}

protected:

    // Just override the mandatory create scene method
    void createScene(void)
    {
        // Create the SkyBox
        mSceneMgr->setSkyBox(true, mSkyBoxes[mCurrentSkyBox], 99999*3, true);

		// Set some camera params
        mCamera->setFarClipDistance(99999*6);
		mCamera->setPosition(4897.61,76.8614,4709.9);
		mCamera->setOrientation(Ogre::Quaternion(0.487431, -0.0391184, 0.869485, 0.0697797));

        // Create Hydrax object
		mHydrax = new Hydrax::Hydrax(mSceneMgr, mCamera);

		// Set RTT textures quality
        mHydrax->setRttOptions(
                   Hydrax::RttOptions(// Reflection tex quality
                                      Hydrax::TEX_QUA_1024,
                                      // Refraction tex quality
                                      Hydrax::TEX_QUA_1024,
                                      // Depth tex quality
                                      Hydrax::TEX_QUA_1024));

		// Set components
		mHydrax->setComponents(
            static_cast<Hydrax::HydraxComponent>(Hydrax::HYDRAX_COMPONENT_SUN    |
                                                 Hydrax::HYDRAX_COMPONENT_FOAM   |
                                                 Hydrax::HYDRAX_COMPONENT_DEPTH  |
                                                 Hydrax::HYDRAX_COMPONENT_SMOOTH |
                                                 Hydrax::HYDRAX_COMPONENT_CAUSTICS));

		// Create our projected grid module  
		Hydrax::Module::ProjectedGrid *mModule 
			= new Hydrax::Module::ProjectedGrid(// Hydrax parent pointer
			                                    mHydrax,
												// Noise module
			                                    new Hydrax::Noise::Perlin(/* Default options (8, 0.085f, 0.49f, 1.4f, 1.27f) */),
												// Base plane
			                                    Ogre::Plane(Ogre::Vector3(0,1,0), Ogre::Vector3(0,0,0)),
												// Projected grid options (Can be updated each frame -> setOptions(...))
										        Hydrax::Module::ProjectedGrid::Options(_def_PGComplexity /* See more constructors */));

		// Set our module
		mHydrax->setModule(static_cast<Hydrax::Module::Module*>(mModule));

		// Set our shader mode
		mHydrax->setShaderMode(Hydrax::MaterialManager::SM_HLSL);

        // Create water
        mHydrax->create();

        // Adjust some options
        mHydrax->setPosition(Ogre::Vector3(0,0,0));
        mHydrax->setPlanesError(37.5);
        mHydrax->setDepthLimit(110);
        mHydrax->setSunPosition(mSunPosition[mCurrentSkyBox]);
        mHydrax->setSunColor(mSunColor[mCurrentSkyBox]);
        mHydrax->setNormalDistortion(0.025);
        mHydrax->setDepthColor(Ogre::Vector3(0.04,0.135,0.185));
        mHydrax->setSmoothPower(5);
		mHydrax->setCausticsScale(12);
        mHydrax->setGlobalTransparency(0.1);
        mHydrax->setFullReflectionDistance(99999997952.0);
		mHydrax->setGlobalTransparency(0);
        mHydrax->setPolygonMode(0);

        // Lights
		mSceneMgr->setAmbientLight(ColourValue(1, 1, 1));
		Ogre::Light *mLight = mSceneMgr->createLight("Light0");
		mLight->setPosition(mSunPosition[mCurrentSkyBox]);
		mLight->setDiffuseColour(1, 1, 1);
		mLight->setSpecularColour(mSunColor[mCurrentSkyBox].x,mSunColor[mCurrentSkyBox].y,mSunColor[mCurrentSkyBox].z);

        // Island
        Ogre::Entity* IslandEntity;
		IslandEntity = mSceneMgr->createEntity("Island", "Island.mesh");
		IslandEntity->setMaterialName("Examples/OffsetMapping/Specular");
		Ogre::SceneNode* IslandSceneNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(Ogre::Vector3(5000, -175, 5000));
		IslandSceneNode->setScale(23, 13.5, 23);
		IslandSceneNode->attachObject(IslandEntity);

		// Add frame listener
		mRoot->addFrameListener(new ExampleHydraxDemoListener(mWindow, mCamera, mSceneMgr));
    }
};

// ----------------------------------------------------------------------------
// Main function, just boots the application object
// ----------------------------------------------------------------------------
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char **argv)
#endif
{
    // Create application object
    SampleApp app;

    try
    {
        app.go();
    }
    catch ( Ogre::Exception& e )
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "An exception has occured: " << e.getFullDescription();
#endif
    }

    return 0;
}
