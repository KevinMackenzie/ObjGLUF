#ifndef OBJGLUF_TYPES_H
#define OBJGLUF_TYPES_H

#include "Exports.h"
#include <glm/glm.hpp>
#include <vector>
#include <glad/glad.h>

namespace GLUF {
/*
======================================================================================================================================================================================================
Type Aliases

*/

using Color   = glm::u8vec4;//only accepts numbers from 0 to 255
using Color3f = glm::vec3;
using Color4f = glm::vec4;
using Vec4Array = std::vector<glm::vec4>;
using Vec3Array =  std::vector<glm::vec3>;
using Vec2Array = std::vector<glm::vec2>;
using IndexArray = std::vector<GLuint>;
using AttribLoc = GLuint;

/*
======================================================================================================================================================================================================
OpenGL Basic Data Structures and Operators

Note: these only play a significant role in Gui, but are presented here as basic types and utilities which can be used independently of the Gui

*/


//Rect is supposed to be used where the origin is bottom left
struct OBJGLUF_API Rect {
    union {
        long left, x;
    };
    long top, right;
    union {
        long bottom, y;
    };
};

struct OBJGLUF_API Rectf {
    float left, top, right, bottom;
};

struct OBJGLUF_API Point {
    union {
        long x, width;
    };
    union {
        long y, height;
    };

    Point(long val1, long val2) : x(val1), y(val2) {}

    Point() : x(0L), y(0L) {}
};

inline Point operator/(const Point &pt0, const Point &pt1) {
    return {pt0.x / pt1.x, pt0.y / pt1.y};
}

inline Point operator/(const Point &pt0, const long &f) {
    return {pt0.x / f, pt0.y / f};
}

inline Point operator-(const Point &pt0, const Point &pt1) {
    return {pt0.x - pt1.x, pt0.y - pt1.y};
}

inline bool operator==(const Rect &rc0, const Rect &rc1) {
    return
            (
                    rc0.left == rc1.left && rc0.right == rc1.right &&
                    rc0.top == rc1.top && rc0.bottom == rc1.bottom
            );
}

inline bool operator!=(const Rect &rc0, const Rect &rc1) {
    return !(rc0 == rc1);
}

inline void bFlip(bool &b) {
    b = !b;
}

OBJGLUF_API bool PtInRect(const Rect &rect, const Point &pt);
OBJGLUF_API void SetRectEmpty(Rect &rect);
OBJGLUF_API void SetRect(Rect &rect, long left, long top, long right, long bottom);
OBJGLUF_API void SetRect(Rectf &rect, float left, float top, float right, float bottom);
OBJGLUF_API void OffsetRect(Rect &rect, long x, long y);
OBJGLUF_API void RepositionRect(Rect &rect, long newX, long newY);
OBJGLUF_API long RectHeight(const Rect &rect);
OBJGLUF_API long RectWidth(const Rect &rect);
OBJGLUF_API void InflateRect(Rect &rect, long dx, long dy);//center stays in same spot
OBJGLUF_API void ResizeRect(Rect &rect, long newWidth, long newHeight);//bottom left stays in same spot
OBJGLUF_API bool IntersectRect(const Rect &rect0, const Rect &rect1, Rect &rectIntersect);
OBJGLUF_API Point MultPoints(const Point &pt0, const Point &pt1);

/*
GetVec2FromRect

    Parameters:
        'rect': the rect to retreive from
        'x': true: right; false: left
        'y': true: top; false: bottom

*/
OBJGLUF_API glm::vec2 GetVec2FromRect(const Rect& rect, bool x, bool y);
OBJGLUF_API glm::vec2 GetVec2FromRect(const Rectf& rect, bool x, bool y);

//used for getting vertices from rects 0,0 is bottom left
OBJGLUF_API Point GetPointFromRect(const Rect& rect, bool x, bool y);


OBJGLUF_API Color4f ColorToFloat(const Color& color);//takes 0-255 to 0.0f - 1.0f
OBJGLUF_API Color3f ColorToFloat3(const Color& color);//takes 0-255 to 0.0f - 1.0f

}
#endif //OBJGLUF_TYPES_H
