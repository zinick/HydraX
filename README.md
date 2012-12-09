HydraX
======

A water rendering library for Ogre3D

Latest stable version: 0.3

Changelog
---------

19/04/08 Hydrax 0.3 -----------------------------------------------------------------
New features since 0.2 version:
 - Hydrax interfaze have been totally rewritten(50% done in 0.2v and 100% now):
   * Geometry is now calculated in Hydrax::Module, and changes are uploaded to Hydrax::Mesh class
   * Hydrax::Module have a Hydrax::Noise object to get water heigths
   * Disponible modules:
     - Hydrax::Module::ProjectedGrid
       * Now vertex x/z possition is calculated only when it's needed.
     - Hydrax::Module::SimpleGrid
       * Totally new module, very different to old Perlin module
   * Disponible noise modules:
     - Hydrax::Noise::Perlin
       * Very fast perlin noise generation method

 - Decals support: (To place textures like ship trails, oil, water vegetables, etc on the water)
   * Based on projective decals approach
   * Decals are only rendered when they're in the camera frustum (It saves a lot of fps!)
   * Some instructions:
     - To add a decal:
       Hydrax::Decal* mDecal = mHydrax->getDecalsManager()->add("Texture.png");
     - Place it:
       mDecal->setPosition(Ogre::Vector2);
     - Set decal size:
       mDecal->setSize(Ogre::Vector2);
     - Rotate it:
       mDecal->setOrientation(Ogre::Radian);
     - Set transparency
       mDecal->setTransparency(Ogre::Real);
     - Detele the decal: mHydrax->getDecalsManager()->remove(mDecal->getId());
    
 - Speed enhancements (New modules are faster than olds)
 - LibNoise have been removed from Hydrax.
 - A lot of bugfixes.
 - License changing from GPL to LGPL.

--------------------------------------------------------------------------------------
 

25/03/08 Hydrax 0.2 -----------------------------------------------------------------
New features since 0.1 version:
 - Hydrax TOTALLY rewritten, so, a huge number of Hydrax 0.2 features and options are new features.

 - Modulable interface added, now Hydrax support all kind of different modules to generate 
   the water geometry, heigth, normal maps, etc...
   * New modules can be codded by users, just override the Hydrax::Module class and... code it ;) !
   * Modules availables in 0.2 version. 
     - Infinite ocean(Based on the projected grid concept).
     - Perlin noise water in a defined plane(Not infinite).

 - Material manager class added, principal features:
   * Different shader modes: HLSL and CG (GLSL support will be added in futures versions).
   * Different ways of generate the normal map: 
     - From precomputed texture in the CPU
     - From vertex(precomputed in the CPU)
     - GPU generation(Will be added in future versions)
   * All materials are generated directly by the Hydrax plugin, no more .material(and .cg,.hsls,.glsl) in Hydrax media files.
   * In order to add compatibility, we can add the depth technique to a material when it won't be use by a Ogre::Entity,
     for example: For terrain manager, PLSM2, custom manual objects, etc... 
     void Hydrax::MaterialManager::addDepthTechnique(Ogre::MaterialPtr &Material, const int &Index); 
     For questions and issues post on the forum.

 - Changes in Hydrax components:
   * PLSM2 component, deleted, see void Hydrax::MaterialManager::addDepthTechnique(Ogre::MaterialPtr &Material, const int &Index);
   * FOAM component now looks very realistic ;)

Notes:
 - This version is a transition from 0.1 to 0.3, so there're a lot of changes to do for the next release.
 - Because of the before point, Hydrax Editor won't be released with this version, but yes with 0.3 version.
 - Not use Hydrax::saveCfg(...) and Hydrax::loadCfg(...), wait to 0.3 version!
 - For any question, use Ogre forums.

--------------------------------------------------------------------------------------

Contact:
Xavier verguín González 
xavyiy@gmail.com
xavierverguin@hotmail.com