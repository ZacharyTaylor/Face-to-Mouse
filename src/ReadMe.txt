========================================================================
    CONSOLE APPLICATION : Mouse2 Project Overview
========================================================================
This application allows control of the mouse cursor using a webcam as is
outlined in the paper "Using head orientation to control a mouse" by
Zachary Taylor

The application was developed in Visual Studios 2010
It requires Opencv 2.2 to be installed
It also makes use of the boostthreads libray for multithreading

The path to these libraries must be specified in FacetoMouse.h and
FaceTracking.h
The path to two included sound files "beep-3.wav" and "sound95.wav" must
also be specified

Most constants for this program are defined in FacetoMouse.h

Mouse2.cpp
    This is the main application source file.

FacetoMouse.cpp
    This contains all functions that corrospond to the transition 
    between face location and mouse movement

FaceTracking.cpp
    This contains all functions that corrospond to locating the 
    face from the given image


