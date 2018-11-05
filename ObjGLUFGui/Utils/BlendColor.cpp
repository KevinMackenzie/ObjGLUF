#include "BlendColor.h"

/*
======================================================================================================================================================================================================
BlendColor Functions


*/

//--------------------------------------------------------------------------------------
BlendColor::BlendColor()
{
    SetAll({ 255, 255, 255, 255 });
}

//--------------------------------------------------------------------------------------
void BlendColor::Init(const Color& defaultColor, const Color& disabledColor, const Color& hiddenColor)
{
    HighBitColor defColor = static_cast<HighBitColor>(defaultColor);
    for (auto it = mStates.begin(); it != mStates.end(); ++it)
    {
        it->second = defColor;
    }

    mStates[STATE_DISABLED] = static_cast<HighBitColor>(disabledColor);
    mStates[STATE_HIDDEN] = static_cast<HighBitColor>(hiddenColor);
    mCurrentColor = mStates[STATE_HIDDEN];//start hidden
}


//--------------------------------------------------------------------------------------
void BlendColor::Blend(ControlState state, float elapsedTime, float rate)
{
    //this is quite condensed, this basically interpolates from the current state to the destination state based on the time
    //the speed of this transition is a recurisve version of e^kx - 1.0f
    float delta = elapsedTime - mPrevBlendTime;
    HighBitColor col = mStates[state];
    //if (col == Color{ 0, 0, 0, 0 })
    //    return;
    float trans = powf(_E_F, rate * delta) - 1.0f;
    float clamped = glm::clamp(trans, 0.0f, 1.0f);
    HighBitColor old = mCurrentColor;
    mCurrentColor = glm::mix(mCurrentColor, col, clamped);
    mPrevBlendTime = elapsedTime;
    //mCurrentColor = glm::mix(mCurrentColor, mStates[state], glm::clamp(powf(_E_F, rate * elapsedTime) - 1.0f, 0.0f, 1.0f));
}

//--------------------------------------------------------------------------------------
void BlendColor::SetCurrent(const Color& current)
{
    mCurrentColor = current;
}

//--------------------------------------------------------------------------------------
void BlendColor::SetCurrent(ControlState state)
{
    mCurrentColor = mStates[state];
}

//--------------------------------------------------------------------------------------
void BlendColor::SetAll(const Color& color)
{
    for (unsigned int i = STATE_NORMAL; i <= STATE_HIDDEN; ++i)
    {
        mStates[static_cast<ControlState>(i)] = color;
    }

    SetCurrent(color);
}

//--------------------------------------------------------------------------------------
Color BlendColor::GetState(ControlState state) const noexcept
{
    return static_cast<Color>(mStates.at(state));
}

//--------------------------------------------------------------------------------------
void BlendColor::SetState(ControlState state, const Color& col) noexcept
{
    mStates[state] = static_cast<HighBitColor>(col);
}

//--------------------------------------------------------------------------------------
Color BlendColor::GetCurrent() const noexcept
{
    return static_cast<Color>(mCurrentColor);
}


