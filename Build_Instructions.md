# Introduction #

This page describes the building instructions of mcore3d on various platforms.

Currently mcore3d supports the following platforms:
  * Win32

# Building on Win32 #

## Prerequisites ##
  * Windows XP/Vista/7
  * Visual Studio 2008 Standard Edition or above with C++ and C# installed
  * Graphics card which supports OpenGL 2.1 or above

## Building Steps ##
  1. Append the mcore3d's 'Bin' directory to the system's 'PATH' environment variable
  1. Open '3Party/3Party.sln' to build all the necessary third party libraries.
  1. Open 'Test/Test.sln' and simply build and run the test projects.
  1. In order to run all the rendering tests, you need to download extra content 'Scene.7z' from the 'Downloads' tab and unzip the content to 'Test/RenderTest/Media'