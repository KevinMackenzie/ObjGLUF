#ifndef OBJGLUF_BLENDCOLOR_H
#define OBJGLUF_BLENDCOLOR_H

/*
BlendColor

    Note:
        Used to modulate colors for different control states to provide a responsive GUI experience
        Internally, Colors are represented as signed 16 bit integer vectors, because issues arise when using
            'glm::mix' with unsigned values

    Data Members:
        'mStates': all of the different control states which can exist
        'mCurrentColor': the current color of the blend state

*/
using HighBitColor = glm::i16vec4;
using ColorStateMap = std::map < ControlState, HighBitColor >;

struct BlendColor
{
    float mPrevBlendTime = 0.0f;
    ColorStateMap       mStates;
    HighBitColor        mCurrentColor;
public:

    BlendColor();

    /*
    Init

        Parameters:
            'defaultColor': the default color state
            'disabledColor': the disabled color state
            'hiddenColor': the hidden color state

    */
    void        Init(const Color& defaultColor, const Color& disabledColor = { 128, 128, 128, 200 }, const Color& hiddenColor = { 255, 255, 255, 0 });

    /*
    Blend

        Parameters:
            'state': the state to blend from the current to
            'elapsedTime': the time elapsed since the begin of the blend period
            'rate': how quickly to blend between states

        Note:
            this is designed to be called every update cycle to provide a smooth blend animation

    */
    void        Blend(ControlState state, float elapsedTime, float rate = 5.0f);

    /*
    SetCurrent

        Parameters:
            'current': the color to set the current state
            'state': the state to set the current state to

    */
    void        SetCurrent(const Color& current);
    void        SetCurrent(ControlState state);

    /*
    SetAll

        Parameters:
            'color': the color to set all of the states to; used for static elements

    */
    void        SetAll(const Color& color);

    Color GetState(ControlState state) const noexcept;
    void        SetState(ControlState state, const Color& col) noexcept;
    Color GetCurrent() const noexcept;
};
#endif //OBJGLUF_BLENDCOLOR_H
