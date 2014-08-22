#version __VERSION__ 

#if __VERSION__ > 420
#define TEXTURE_2D texture
#define TEXTURE_3D texture
#else
#define TEXTURE_2D texture2D
#define TEXTURE_3D texture3D