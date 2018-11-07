#include "Types.h"

namespace GLUF {
/*
======================================================================================================================================================================================================
OpenGL Basic Data Structures and Operators

*/

//--------------------------------------------------------------------------------------
bool PtInRect(const Rect &rect, const Point &pt) {
    //for the first comparison, it is impossible for both statements to be false,
    //because if the y is greater than the top, it is automatically greater than the bottom, and vise versa
    return (pt.y >= rect.bottom && pt.y <= rect.top) &&
           (pt.x <= rect.right && pt.x >= rect.left);
}

//--------------------------------------------------------------------------------------
void SetRectEmpty(Rect &rect) {
    rect.top = rect.bottom = rect.left = rect.right = 0;
}

//--------------------------------------------------------------------------------------
void SetRect(Rect &rect, long left, long top, long right, long bottom) {
    rect.top = top;
    rect.bottom = bottom;
    rect.left = left;
    rect.right = right;
}

//--------------------------------------------------------------------------------------
void SetRect(Rectf &rect, float left, float top, float right, float bottom) {
    rect.top = top;
    rect.bottom = bottom;
    rect.left = left;
    rect.right = right;
}

//--------------------------------------------------------------------------------------
void OffsetRect(Rect &rect, long x, long y) {
    rect.top += y;
    rect.bottom += y;
    rect.left += x;
    rect.right += x;
}

//--------------------------------------------------------------------------------------
void RepositionRect(Rect &rect, long newX, long newY) {
    long deltaX = newX - rect.left;
    long deltaY = newY - rect.bottom;

    rect.left = newX;
    rect.right += deltaX;
    rect.bottom = newY;
    rect.top += deltaY;
}

//--------------------------------------------------------------------------------------
long RectHeight(const Rect &rect) {
    return rect.top - rect.bottom;
}

//--------------------------------------------------------------------------------------
long RectWidth(const Rect &rect) {
    return rect.right - rect.left;
}


//--------------------------------------------------------------------------------------
void InflateRect(Rect &rect, long dx, long dy) {
    long dx2 = dx / 2;
    long dy2 = dy / 2;
    rect.left -= dx2;
    rect.right += dx2;//remember to have opposites

    rect.top += dy2;
    rect.bottom -= dy2;
}

//--------------------------------------------------------------------------------------
void ResizeRect(Rect &rect, long newWidth, long newHeight) {
    rect.top = rect.bottom + newHeight;
    rect.right = rect.left + newWidth;
}

//--------------------------------------------------------------------------------------
bool IntersectRect(const Rect &rect0, const Rect &rect1, Rect &rectIntersect) {

    //Left
    if (rect0.left > rect1.left) {
        rectIntersect.left = rect0.left;
    } else {
        rectIntersect.left = rect1.left;
    }

    //Right
    if (rect0.right < rect1.right) {
        rectIntersect.right = rect0.right;
    } else {
        rectIntersect.right = rect1.right;
    }


    //Top
    if (rect0.top < rect1.top) {
        rectIntersect.top = rect0.top;
    } else {
        rectIntersect.top = rect1.top;
    }

    //Bottom
    if (rect0.bottom > rect1.bottom) {
        rectIntersect.bottom = rect0.bottom;
    } else {
        rectIntersect.bottom = rect1.bottom;
    }

    //this will ONLY happen if the do NOT intersect
    if (rectIntersect.left > rectIntersect.right || rectIntersect.top < rectIntersect.bottom) {
        SetRectEmpty(rectIntersect);
        return false;
    }

    return true;
}


/*
======================================================================================================================================================================================================
Datatype Conversion Functions

*/

//--------------------------------------------------------------------------------------
Color4f ColorToFloat(const Color &color) {
    Color4f col;
    col.x = glm::clamp((float) color.x / 255.0f, 0.0f, 1.0f);
    col.y = glm::clamp((float) color.y / 255.0f, 0.0f, 1.0f);
    col.z = glm::clamp((float) color.z / 255.0f, 0.0f, 1.0f);
    col.w = glm::clamp((float) color.w / 255.0f, 0.0f, 1.0f);
    return col;
}

//--------------------------------------------------------------------------------------
Color3f ColorToFloat3(const Color &color) {
    Color3f col;
    col.x = glm::clamp((float) color.x / 255.0f, 0.0f, 1.0f);
    col.y = glm::clamp((float) color.y / 255.0f, 0.0f, 1.0f);
    col.z = glm::clamp((float) color.z / 255.0f, 0.0f, 1.0f);
    return col;
}

//--------------------------------------------------------------------------------------
Point MultPoints(const Point &pt0, const Point &pt1) {
    Point retPt;
    retPt.x = pt1.x * pt0.x;
    retPt.y = pt1.y * pt1.y;

    return retPt;
}


//--------------------------------------------------------------------------------------
glm::vec2 GetVec2FromRect(const Rect &rect, bool x, bool y) {
    if (x)
        if (y)
            return glm::vec2(rect.right, rect.top);
        else
            return glm::vec2(rect.right, rect.bottom);
    else if (y)
        return glm::vec2(rect.left, rect.top);
    else
        return glm::vec2(rect.left, rect.bottom);
}

//--------------------------------------------------------------------------------------
glm::vec2 GetVec2FromRect(const Rectf &rect, bool x, bool y) {
    if (x)
        if (y)
            return glm::vec2(rect.right, rect.top);
        else
            return glm::vec2(rect.right, rect.bottom);
    else if (y)
        return glm::vec2(rect.left, rect.top);
    else
        return glm::vec2(rect.left, rect.bottom);
}

}

