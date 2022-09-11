# RaytracingFinal

## Example images

<img src="\teapot.png" alt="cube" style="zoom: 33%;" />
<img src="RaytracingFinal\rings.png" alt="cube" style="zoom: 33%;" />
![teapot]("C:\Users\DELL\Desktop\作业\RaytracingFinal\rings.png"）

## A description of what I implemented

the basic functions of raytracing were implemented.
the intersection is used to calculated the intersection of a ray with a triangle or sphere, which could be apllied to find where the ray hits
calculate the diffuse and specular light by the shading skills which used in assighment 1 to get the shading color 
use the same principle when dealing with shadows and reflections----to figure out if the light ray or reflect ray hit any surfaces
in shadow part: if the light ray hit any surfaces return black color
in reflection part: if the reflection ray of the eye-ray hit anything, the point where eye-ray hit becomes new "eye", the reflection ray and new eye will be used to do a "trace" again
when doing reflection there was a recursive loop between trace and shade, what I could do is to set a threshold value to stop this recursive loop

## some problems and how I solved them

