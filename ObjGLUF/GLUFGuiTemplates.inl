#pragma once

namespace GLUF
{

    /*

    GLUFTextHelper Template Functions

    */

    //--------------------------------------------------------------------------------------
    template<class... Types>
    void GLUFTextHelper::DrawFormattedTextLine(const std::wstring& format, Types&... args)
    {
        std::wstring outString;
        RenderText(format, outString, args...);

        DrawTextLine(outString);
    }

    //--------------------------------------------------------------------------------------
    template<class... Types>
    void GLUFTextHelper::DrawFormattedTextLine(const GLUF::GLUFRect& rc, GLUFBitfield flags, const std::wstring& format, Types&... args)
    {
        std::wstring outString;
        RenderText(format, outString, args...);

        DrawTextLine(rc, flags, outString);
    }

    //--------------------------------------------------------------------------------------
    template<class... Types>
    void GLUFTextHelper::RenderText(const std::wstring& format, std::wstring& outString, Types&... args)
    {
        std::wstringstream formatStream;
        formatStream << format;

        std::wstringstream outStringStream;
        RenderText(formatStream, outStringStream, args...);
        outString = outStringStream.str();
    }

    //--------------------------------------------------------------------------------------
    template<typename T1, typename... Types>
    void GLUFTextHelper::RenderText(std::wstringstream& formatStream, std::wstringstream& outString, T1 arg1, Types... args)
    {
        wchar_t ch;
        while (format.get(ch))
        {
            if (ch == '%')
            {
                outString << arg1;
                RenderText(format, outString, args...);
            }
            else
            {
                outString << ch;
            }
        }
    }

    //--------------------------------------------------------------------------------------
    template<typename T1>
    void GLUFTextHelper::RenderText(std::wstringstream& formatStream, std::wstringstream& outString, T1 arg)
    {
        wchar_t ch;
        bool usedArg = false;
        while (format.get(ch))
        {
            if (ch == '%' && !usedArg)
            {
                outString << arg;
                usedArg = true;
            }
            else
            {
                outString << ch;
            }
        }
    }


}