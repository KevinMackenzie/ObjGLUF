#pragma once

namespace GLUF {







/*

Control Creation Functions


*/

//--------------------------------------------------------------------------------------
inline std::shared_ptr<Dialog> CreateDialog_() {
    return std::shared_ptr<Dialog>(new Dialog());
}

//--------------------------------------------------------------------------------------
inline std::shared_ptr<Static> CreateStatic(Bitfield textFlags, Dialog &dialog) {
    return std::shared_ptr<Static>(new Static(textFlags, dialog));
}

//--------------------------------------------------------------------------------------
inline std::shared_ptr<Button> CreateButton(Dialog &dialog) {
    return std::shared_ptr<Button>(new Button(dialog));
}

//--------------------------------------------------------------------------------------
inline std::shared_ptr<CheckBox> CreateCheckBox(bool checked, Dialog &dialog) {
    return std::shared_ptr<CheckBox>(new CheckBox(checked, dialog));
}

//--------------------------------------------------------------------------------------
inline std::shared_ptr<RadioButton> CreateRadioButton(Dialog &dialog) {
    return std::shared_ptr<RadioButton>(new RadioButton(dialog));
}

//--------------------------------------------------------------------------------------
inline std::shared_ptr<ScrollBar> CreateScrollBar(Dialog &dialog) {
    return std::shared_ptr<ScrollBar>(new ScrollBar(dialog));
}

//--------------------------------------------------------------------------------------
inline std::shared_ptr<ListBox> CreateListBox(Dialog &dialog) {
    return std::shared_ptr<ListBox>(new ListBox(dialog));
}

//--------------------------------------------------------------------------------------
inline std::shared_ptr<ComboBox> CreateComboBox(Dialog &dialog) {
    return std::shared_ptr<ComboBox>(new ComboBox(dialog));
}

//--------------------------------------------------------------------------------------
inline std::shared_ptr<Slider> CreateSlider(Dialog &dialog) {
    return std::shared_ptr<Slider>(new Slider(dialog));
}

//--------------------------------------------------------------------------------------
inline std::shared_ptr<EditBox> CreateEditBox(Dialog &dialog, bool isMultiline) {
    return std::shared_ptr<EditBox>(new EditBox(dialog, isMultiline));
}

//--------------------------------------------------------------------------------------
inline std::shared_ptr<TextHelper> CreateTextHelper(DialogResourceManagerPtr &drm) {
    return std::shared_ptr<TextHelper>(new TextHelper(drm));
}


}