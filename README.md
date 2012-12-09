HydraX
======

A water rendering library for Ogre3D

Latest stable version: 0.5.1

Introduction
------------

Hydrax is an add-on for Ogre which aim is to provide an easy-use library in order to render pretty water scenes.

One of the primary design goals is to be fully configurable. All effects - such as depth-based distortions, smooth transitions, foam, caustics and underwater god rays - can be modified in real-time.

Real-time configuration also extends to things such as RTT's texture quality and HydraX's geometry and noise modules options. You can change between different modules (eg: geometry and noise) on-fly.

Main features
--------------
 
* Realistic fresnel refraction and reflection effects
* Intelligent depth effects based on depth maps
* Smooth transitions effects: Smooth transitions with all objects in the scene using the depth map
* Caustics effects
* Sunlight effects (Colour, Position, Sun strength, area, etc...)
* Full underwater effects like underwater reflections, god rays...
* Foam effects
* Water editor
* Vertex and GPU Normals generation
* Decals support (to place any kind of texture on the water, like ship trails, water vegetables, etc)
* Modulable interface (Geometry and noise modules)
* HLSL and CG support (and GLSL in future versions) 

Hydrax has a modular interface wich allows any kind of water geometry. At present there are three geometry and two noise modules available as follows:

* simple grid geometry
* radial grid geometry
* infinite ocean geometry (based on the projected grid concept)

* perlin noise
* FFT noise

Credits
-------

The original creator of HydraX is xaviyiy, AKA Xavier Verguín González (xavyiy@gmail.com). The official repository for his work can be found here: http://modclub.rigsofrods.com/xavi/

The project appears to be abandoned by xaviyiy as of 14th Deceomber 2010. For support, it is recommended to refer to the HydraX section of the Ogre3D forum:

http://www.ogre3d.org/addonforums/viewforum.php?f=20

Screenshots
-----------

http://www.ogre3d.org/tikiwiki/img/wiki_up/Hydrax_04_5_.png
http://www.ogre3d.org/tikiwiki/img/wiki_up/Hydrax_04_2.png
http://www.ogre3d.org/tikiwiki/img/wiki_up/Hydrax_04_1_.png

http://www.ogre3d.org/tikiwiki/img/wiki_up/Hydrax_04_3_.png
http://www.ogre3d.org/tikiwiki/img/wiki_up/Hydrax_04_4_.png

Videos
------

HydraX with SkyX:
http://www.youtube.com/watch?v=FL02s_MGIQc