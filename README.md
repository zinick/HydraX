HydraX
======

A water rendering library for Ogre3D

Latest stable version: 0.2

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

Contact:
Xavier verguín González 
xavyiy@gmail.com
xavierverguin@hotmail.com