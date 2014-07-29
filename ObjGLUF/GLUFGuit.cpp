#include "stdafx.h"
//#include "GLUFGui.h"
#include "ft2build.h"
#include FT_FREETYPE_H

//the freetype library
FT_Library ft;

void GLUFInitGui()
{
	GLUF_ASSERT(FT_Init_FreeType(&ft));
}

class GLUFFont
{
public:
	FT_Face mFontFace;

	operator FT_Face*(){ return &mFontFace; }
};

GLUFFontPtr GLUFLoadFont(unsigned char* rawData, uint64_t rawSize)
{
	GLUFFontPtr ret(new GLUFFont());
	
	FT_Error err = FT_New_Memory_Face(ft, rawData, rawSize, 0, *ret);
	if (err)
	{
		GLUF_ERROR("Error loading font Error code: " + err);
	}
}