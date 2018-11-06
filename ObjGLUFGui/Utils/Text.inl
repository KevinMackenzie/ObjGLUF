namespace GLUF {

/*

TextHelper Template Functions

*/

//--------------------------------------------------------------------------------------
template<class... Types>
void TextHelper::DrawFormattedTextLine(const std::wstring &format, const Types &... args) noexcept {
    std::wstring outString;
    RenderText(format, outString, args...);

    DrawTextLine(outString);
}

//--------------------------------------------------------------------------------------
template<class... Types>
void TextHelper::DrawFormattedTextLineBase(const Rect &rc, Bitfield flags, const std::wstring &format, const Types &... args) noexcept {
    std::wstring outString;
    RenderText(format, outString, args...);

    DrawTextLineBase(rc, flags, outString);
}

//--------------------------------------------------------------------------------------
template<class... Types>
void TextHelper::RenderText(const std::wstring &format, std::wstring &outString, const Types &... args) noexcept {
    std::wstringstream formatStream;
    formatStream << format;

    std::wstringstream outStringStream;
    RenderText(formatStream, outStringStream, args...);
    outString = outStringStream.str();
}

//--------------------------------------------------------------------------------------
template<typename T1, typename... Types>
void TextHelper::RenderText(std::wstringstream &formatStream, std::wstringstream &outStringStream, const T1 &arg1, const Types &... args) {
    wchar_t ch;
    while (formatStream.get(ch)) {
        if (ch == '%') {
            outStringStream << arg1;
            RenderText(formatStream, outStringStream, args...);
        } else {
            outStringStream << ch;
        }
    }
}

//--------------------------------------------------------------------------------------
template<typename T1>
void TextHelper::RenderText(std::wstringstream &formatStream, std::wstringstream &outStringStream, const T1 &arg) {
    wchar_t ch;
    bool usedArg = false;
    while (formatStream.get(ch)) {
        if (ch == '%' && !usedArg) {
            outStringStream << arg;
            usedArg = true;
        } else {
            outStringStream << ch;
        }
    }
}

}
