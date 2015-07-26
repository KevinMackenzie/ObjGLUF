#pragma once

namespace GLUF
{



    //--------------------------------------------------------------------------------------
    template<typename T>
    std::shared_ptr<T> GLUFDialog::GetControl(int ID) const
    {
        // Try to find the control with the given ID
        for (auto it : mControls)
        {
            if (it.second->GetID() == ID)
            {
                return std::dynamic_pointer_cast<T>(it.second);
            }
        }

        GLUF_NON_CRITICAL_EXCEPTION(std::invalid_argument("Control ID Not Found"));

        // Not found
        return nullptr;
    }



    /*

    GLUFTextHelper Template Functions

    */

    //--------------------------------------------------------------------------------------
    template<class... Types>
    void GLUFTextHelper::DrawFormattedTextLine(const std::wstring& format, Types&... args) noexcept
    {
        NOEXCEPT_REGION_START

        std::wstring outString;
        RenderText(format, outString, args...);

        DrawTextLine(outString);

        NOEXCEPT_REGION_END
    }

    //--------------------------------------------------------------------------------------
    template<class... Types>
    void GLUFTextHelper::DrawFormattedTextLine(const GLUF::GLUFRect& rc, GLUFBitfield flags, const std::wstring& format, Types&... args) noexcept
    {
        NOEXCEPT_REGION_START

        std::wstring outString;
        RenderText(format, outString, args...);

        DrawTextLine(rc, flags, outString);

        NOEXCEPT_REGION_END
    }

    //--------------------------------------------------------------------------------------
    template<class... Types>
    void GLUFTextHelper::RenderText(const std::wstring& format, std::wstring& outString, Types&... args) noexcept
    {
        NOEXCEPT_REGION_START

        std::wstringstream formatStream;
        formatStream << format;

        std::wstringstream outStringStream;
        RenderText(formatStream, outStringStream, args...);
        outString = outStringStream.str();

        NOEXCEPT_REGION_END
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