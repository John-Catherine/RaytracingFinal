# RaytracingFinal

## Example images

<img src="C:\Users\DELL\Desktop\作业\RaytracingFinal\teapot.png" alt="cube" style="zoom: 33%;" />
<img src="C:\Users\DELL\Desktop\作业\RaytracingFinal\rings.png" alt="cube" style="zoom: 33%;" />

## A description of what I implemented

the basic functions of raytracing were implemented.
the intersection is used to calculated the intersection of a ray with a triangle or sphere, which could be apllied to find where the ray hits
calculate the diffuse and specular light by the shading skills which used in assighment 1 to get the shading color 
use the same principle when dealing with shadows and reflections----to figure out if the light ray or reflect ray hit any surfaces
in shadow part: if the light ray hit any surfaces return black color
in reflection part: if the reflection ray of the eye-ray hit anything, the point where eye-ray hit becomes new "eye", the reflection ray and new eye will be used to do a "trace" again
when doing reflection there was a recursive loop between trace and shade, what I could do is to set a threshold value to stop this recursive loop

## Images from during development that show the steps in raytracing development

## Images of some problems

## A description of some problems and how I solved them

