# assignment-5-final-project-firefly
assignment-5-final-project-firefly created by GitHub Classroom

## Initial Proposal
[Graphics Final Project Proposal.pdf](https://github.com/nyu-cs-gy-6533-fall-2022/assignment-5-final-project-firefly/files/10201571/Graphics.Final.Project.Proposal.1.pdf) <br />
The initial goal of the project was inspired by artistic webGL applications found on Shadertoy. I started off by wanting to create a procedural terrain with instances of fireflies wandering the terrain. However over the course of the project, the project direction had to be pivoted to ensure progress during the learning process.


## Setup
To start off the project, I did the setup of the project by cleaning up and refactoring the code from prior assignments. 

## Free Camera Movement
After cleaning up and removing previous tasks I started off by implementing camera movement and smooth camera rotation. 
![wavesstarry-cam](https://user-images.githubusercontent.com/37173256/206891613-36c8141b-01be-4386-9826-92729aae4e71.gif)

## Low Poly Plane Generation
For the procedural generation, I first statically implemented a plane where the Y-axis vertex is decided by the perlin noise value at the plane's X and Y vertex values. I was aiming for a retro, low-poly aesthetic so the illumination was calculated in the vertex shader using the 'flat' shader variable type which overwrote normal interpolation using the value from just one of the triangles' vertices. This gave the terrain per-face normals for each triangle, giving the hard edges between each polygon. 
<br />
<br />
However with the standard triangle meshing for the plane, all of the triangles formed quads in the same pattern making the texture of the terrain very repetitive. In order to break up this monotony in texture, I reorganized the plane's index buffer to have triangle alternate patterns. Because I wanted the low-poly aesthetic using the 'flat' variable type, throughout the rest of the development process I was testing the different visuals between which triangle vertex's values the 'flat' variable would take. In order to maintain a smooth aesthetic, I opted to determine the terrain color in the fragment shader which decided the coloring of the terrain based on the Y-axis value of the mesh's position.


The following are some early plane generations with different color schemes resembling (sunny day, blue night, no color modification): <br />
![image](https://user-images.githubusercontent.com/37173256/206885746-d229a64e-ef29-4da1-a17b-dfe19dfd476c.png)

![image](https://user-images.githubusercontent.com/37173256/206885751-1fa55ad9-40e7-42ea-97b1-23454308da9f.png)

![image](https://user-images.githubusercontent.com/37173256/206885755-13cebc79-6def-4608-baa8-9d60980167cd.png)

## Snow Instancing
In the initial project plan I wanted to create instances of fireflies that roamed above the terrain, but I was having difficulties getting textures working (which later turned out to be an incompatibility in data structures between libraries) so I couldn't get height maps for which the other objects to interact with so the instances were pivoted to falling snow.
<br />
<br />
To imitate the snow flakes falling, each snow flake is a small sphere object. However, because the snow was a separate mesh and coloring style I had to create a new VAO, new buffers, and shader programs to render the snow. After the snow objects were able to be rendered alongside the terrain VAO, I instanced the snow flakes and passed along an vec3 offset value uniform array for each of the snow flake instances to be able to have an independent position. To calculate the offsets to pass in the offset uniform array, each piece of snow also had a vector value attached to it to track and smooth out the random movement vector that it was moving each frame.

Combining all of the above mentioned steps and a falling constant speed in the vector, the following .gif is an example shot:
<br />
![eea2ec9e082143546790f2aab5cd269a95819018f5944392facc75a620dbde65](https://user-images.githubusercontent.com/37173256/206885757-190fab09-588b-4726-9c57-65f1f697c3a0.gif)


## Multiple moving lights with attenuation
In the spirit of the original project plan of fireflies, I decided to make each of the snowflakes its own light source which would impact the terrain. In order to track the light positioning to influence the terrain, I sent in the snow positions as a Uniform array to the terrain shaders. Using this I computed the attenuation (light falloff) from each snowflake to its respective terrain coordinates. Attenuation was necessary as without any lighting falloff the different light sources would be immediately overwhelming and make the image an unintelligible white blob. However, even with attenuation the sheer number of snowflakes in the scene made the lights compounding and very bright, making the tweaking of the constant/ linear/ exponential attenuation attributes tedious.


![45663744856d14a01dfb0b940c627e4f9700ff2eb2452801a0fbf0ca3a29cdfc](https://user-images.githubusercontent.com/37173256/206885764-982c0347-b2ca-44c1-afe7-c0a4476dbfa3.gif)

## Heightmap Texture
After getting some help learning and restructuring my texture code, I was able to generate the heightmap texture from perlin noise which I used for the Y-axis position values of the terrain. This heightmap texture allowed me to easily offset the terrain, giving access to a new dynamic moving view of the scene. The height map also enabled me to finally acces the Y-axis values of the terrain from the snow objects, enabling the falling snow to rest in place at the terrain's elevation level.

![wavesstarry-restsnow](https://user-images.githubusercontent.com/37173256/206893737-b2396368-40a8-4ada-83c4-5753734e2093.gif)

## Star Bloom
Imitating the technique for implementing Bloom, I used the deferred shading pipeline to implement a 'star bloom' effect. From earlier, each snow flake has their alpha value decreased based on distance from camera effectively reducing the value or 'brightness' of the snowflake. This means that the bloom effect is only active for snowflakes that are within a certain distance threshold from the camera.
<br />
<br />
The following .gifs are a couple examples of how the 'star bloom' effect would look and how farther snowflakes wouldn't activate the effect:
![e7bda613beb8194c5096ab0367673dc0579d55a3ed9c433da5181834ec5d1429](https://user-images.githubusercontent.com/37173256/206885767-88762f86-eaf9-41c0-b96a-e2e05851eda6.gif)

![441c502e2a400a9fbd3e243fd0adcf48c3accbcc8a085be3d9305fe6bc65db91](https://user-images.githubusercontent.com/37173256/206885768-bf174a36-de94-4dfe-9d09-feef500041c2.gif)

## orbiting camera mode
Implemented an extra 'orbit' mode to add no input movement. To toggle between orbit mode and control camera mode, use 'P' and 'O' respectively.
<br />
<br />
![wavesstarry-freqvarorbit](https://user-images.githubusercontent.com/37173256/206887785-ac4058ee-dcf9-42d1-8c86-e24f3af194c6.gif)

## distance based rendering
An additional feature to save on performance and mimic distance, added alpha blending modes so that snow alpha decreases with distance from the camera and at a certain point stops rendering entirely. Additionally the alpha for the terrain also decreases exponentially based on distance from the camera to match with the snow.

![wavesstarry-distance](https://user-images.githubusercontent.com/37173256/206893967-3077777a-8b80-4117-94b8-785314de38fe.gif)

additionally, I found that the GL helper classes provided in class code was limited in handling all of the rendering pipeline specifically the Tesselation Control Shader and Tesselation Evaluation Shader, so I made some additions to the helper files to add a ```Program.tessInit()```
