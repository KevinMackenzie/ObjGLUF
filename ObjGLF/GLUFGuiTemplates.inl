#pragma once

namespace GLUF
{



    //--------------------------------------------------------------------------------------
    template<typename T>
    std::shared_ptr<T> Dialog::GetControl(ControlIndex ID) const
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

    TextHelper Template Functions

    */

    //--------------------------------------------------------------------------------------
    template<class... Types>
    void TextHelper::DrawFormattedTextLine(const std::wstring& format, const Types&... args) noexcept
    {
        NOEXCEPT_REGION_START

        std::wstring outString;
        RenderText(format, outString, args...);

        DrawTextLine(outString);

        NOEXCEPT_REGION_END
    }

    //--------------------------------------------------------------------------------------
    template<class... Types>
    void TextHelper::DrawFormattedTextLineBase(const Rect& rc, Bitfield flags, const std::wstring& format, const Types&... args) noexcept
    {
        NOEXCEPT_REGION_START

        std::wstring outString;
        RenderText(format, outString, args...);

        DrawTextLineBase(rc, flags, outString);

        NOEXCEPT_REGION_END
    }

    //--------------------------------------------------------------------------------------
    template<class... Types>
    void TextHelper::RenderText(const std::wstring& format, std::wstring& outString, const Types&... args) noexcept
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
    void TextHelper::RenderText(std::wstringstream& formatStream, std::wstringstream& outStringStream, const T1& arg1, const Types&... args)
    {
        wchar_t ch;
        while (formatStream.get(ch))
        {
            if (ch == '%')
            {
                outStringStream << arg1;
                RenderText(formatStream, outStringStream, args...);
            }
            else
            {
                outStringStream << ch;
            }
        }
    }

    //--------------------------------------------------------------------------------------
    template<typename T1>
    void TextHelper::RenderText(std::wstringstream& formatStream, std::wstringstream& outStringStream, const T1& arg)
    {
        wchar_t ch;
        bool usedArg = false;
        while (formatStream.get(ch))
        {
            if (ch == '%' && !usedArg)
            {
                outStringStream << arg;
                usedArg = true;
            }
            else
            {
                outStringStream << ch;
            }
        }
    }


    /*
    
    Control Creation Functions
    
    
    */

    //--------------------------------------------------------------------------------------
    inline std::shared_ptr<Dialog> CreateDialog_()
    {
        return std::shared_ptr<Dialog>(new Dialog());
    }

    //--------------------------------------------------------------------------------------
    inline std::shared_ptr<Static> CreateStatic(Bitfield textFlags, Dialog& dialog)
    {
        return std::shared_ptr<Static>(new Static(textFlags, dialog));
    }

    //--------------------------------------------------------------------------------------
    inline std::shared_ptr<Button> CreateButton(Dialog& dialog)
    {
        return std::shared_ptr<Button>(new Button(dialog));
    }

    //--------------------------------------------------------------------------------------
    inline std::shared_ptr<CheckBox> CreateCheckBox(bool checked, Dialog& dialog)
    {
        return std::shared_ptr<CheckBox>(new CheckBox(checked, dialog));
    }

    //--------------------------------------------------------------------------------------
    inline std::shared_ptr<RadioButton> CreateRadioButton(Dialog& dialog)
    {
        return std::shared_ptr<RadioButton>(new RadioButton(dialog));
    }

    //--------------------------------------------------------------------------------------
    inline std::shared_ptr<ScrollBar> CreateScrollBar(Dialog& dialog)
    {
        return std::shared_ptr<ScrollBar>(new ScrollBar(dialog));
    }

    //--------------------------------------------------------------------------------------
    inline std::shared_ptr<ListBox> CreateListBox(Dialog& dialog)
    {
        return std::shared_ptr<ListBox>(new ListBox(dialog));
    }

    //--------------------------------------------------------------------------------------
    inline std::shared_ptr<ComboBox> CreateComboBox(Dialog& dialog)
    {
        return std::shared_ptr<ComboBox>(new ComboBox(dialog));
    }

    //--------------------------------------------------------------------------------------
    inline std::shared_ptr<Slider> CreateSlider(Dialog& dialog)
    {
        return std::shared_ptr<Slider>(new Slider(dialog));
    }

    //--------------------------------------------------------------------------------------
    inline std::shared_ptr<EditBox> CreateEditBox(Dialog& dialog, bool isMultiline)
    {
        return std::shared_ptr<EditBox>(new EditBox(dialog, isMultiline));
    }

    //--------------------------------------------------------------------------------------
    inline std::shared_ptr<TextHelper> CreateTextHelper(DialogResourceManagerPtr& drm)
    {
        return std::shared_ptr<TextHelper>(new TextHelper(drm));
    }


    template<typename modType>
    void ModificationStack::PushPartialModification(const modType& m)
    {
        mPartialMod.PushPart(m);
    }


    template<typename modType>
    void ModificationStack::PushGeneric(const modType& m)
    {
        ModificationStackInternal::Modification* thisMod = nullptr;
        try
        {
            thisMod = dynamic_cast<ModificationStackInternal::Modification*>(new modType(m));
            if (thisMod == nullptr)
                throw;
            //*thisMod = m;
        }
        catch (...)
        {
            //'modType' is not a child of 'Modification'
            return;
        }

        mUndoStack.push(thisMod);

        thisMod->ApplyModificationToString(mText);

        //if something is added to the undo stack, why would the redo stack be relevent anymore?
        FlattenRedoStack();
    }

    template<typename modType>
    void ModificationStackInternal::GenericCompositeModification::PushPart(const modType& m)
    {
        ModificationStackInternal::Modification* thisMod = nullptr;
        try
        {
            thisMod = dynamic_cast<ModificationStackInternal::Modification*>(new modType(m));
            if (thisMod == nullptr)
                throw;
            //*thisMod = m;
        }
        catch (...)
        {
            //'modType' is not a child of 'Modification'
            return;
        }

        mParts.push_back(std::shared_ptr<Modification>(thisMod));
    }

}