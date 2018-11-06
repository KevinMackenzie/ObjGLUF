#include "ModificationStack.h"

namespace GLUF {

//--------------------------------------------------------------------------------------
void ModificationStack::FlattenRedoStack() noexcept {
    while (!mRedoStack.empty()) {
        mRedoStack.pop();
    }
}

//--------------------------------------------------------------------------------------
void ModificationStack::FlattenUndoStack() noexcept {
    while (!mUndoStack.empty()) {
        mUndoStack.pop();
    }
}

//--------------------------------------------------------------------------------------
ModificationStack::ModificationStack(const std::wstring &initialText) :
        mText(initialText) {
}

//--------------------------------------------------------------------------------------
void ModificationStack::PushAddition(const std::wstring &text, uint32_t loc) {
    ModificationStackInternal::ModificationAddition m;
    m.mInsertLocation = loc;
    m.mNewText = text;
    PushGeneric(m);
}

//--------------------------------------------------------------------------------------
void ModificationStack::PushRemoval(uint32_t start, uint32_t end) {
    ModificationStackInternal::ModificationRemoval m;
    m.mStartIndex = start;
    m.mEndIndex = end;
    PushGeneric(m);
}

//--------------------------------------------------------------------------------------
/*void ModificationStack::PushRemovalAndAddition(uint32_t startIndex, uint32_t endIndex, const std::wstring& addition)
{
    ModificationStackInternal::ModificationRemovalAndAddition m;
    m.mStartIndex = startIndex;
    m.mInsertLocation = startIndex;
    m.mEndIndex = endIndex;
    m.mNewText = addition;
    PushGeneric(m);
}*/

//--------------------------------------------------------------------------------------
void ModificationStack::ApplyPartialModifications() noexcept {
    if (mPartialMod.Empty())
        return;

    PushGeneric(mPartialMod);
    mPartialMod.ClearParts();
}

//--------------------------------------------------------------------------------------
void ModificationStack::PushPartialAddition(const std::wstring &text, uint32_t loc) {
    ModificationStackInternal::ModificationAddition m;
    m.mInsertLocation = loc;
    m.mNewText = text;
    mPartialMod.PushPart(m);
}

//--------------------------------------------------------------------------------------
void ModificationStack::PushPartialRemoval(uint32_t start, uint32_t end) {
    ModificationStackInternal::ModificationRemoval m;
    m.mStartIndex = start;
    m.mEndIndex = end;
    mPartialMod.PushPart(m);
}

//--------------------------------------------------------------------------------------
void ModificationStack::UndoNextItem() noexcept {
    if (mUndoStack.empty())
        return;

    auto top = mUndoStack.top();
    top->RemoveModificationToString(mText);
    mUndoStack.pop();
    mRedoStack.push(top);
}

//--------------------------------------------------------------------------------------
void ModificationStack::RedoNextItem() noexcept {
    if (mRedoStack.empty())
        return;

    auto top = mRedoStack.top();
    top->ApplyModificationToString(mText);
    mRedoStack.pop();
    mUndoStack.push(top);
}

//--------------------------------------------------------------------------------------
void ModificationStack::FlattenStack() noexcept {
    FlattenUndoStack();
    FlattenRedoStack();
}

//--------------------------------------------------------------------------------------
void ModificationStack::SetText(const std::wstring &text) noexcept {
    mText = text;
    FlattenStack();
}

/*

Subclass functions

*/

//--------------------------------------------------------------------------------------
ModificationStackInternal::ModificationAddition &ModificationStackInternal::ModificationAddition::operator=(const ModificationAddition &str) {
    mInsertLocation = str.mInsertLocation;
    mNewText = str.mNewText;

    return *this;
}

//--------------------------------------------------------------------------------------
void ModificationStackInternal::ModificationAddition::ApplyModificationToString(std::wstring &str) const {
    auto begin = std::begin(str) + std::clamp(mInsertLocation, (uint32_t) 0, static_cast<uint32_t>(str.size()));
    str.insert(begin, std::begin(mNewText), std::end(mNewText));
}

//--------------------------------------------------------------------------------------
void ModificationStackInternal::ModificationAddition::RemoveModificationToString(std::wstring &str) const {
    auto begin = std::begin(str) + std::clamp(mInsertLocation, (uint32_t) 0, static_cast<uint32_t>(str.size()));
    auto end = std::begin(str) + std::clamp(mInsertLocation + static_cast<uint32_t>(mNewText.size()), (uint32_t) 0,
                                            static_cast<uint32_t>(str.size()));

    str.erase(begin, end);
}

//--------------------------------------------------------------------------------------
ModificationStackInternal::ModificationRemoval &ModificationStackInternal::ModificationRemoval::operator=(const ModificationRemoval &str) {
    mStartIndex = str.mStartIndex;
    mEndIndex = str.mEndIndex;
    mRemovedText = str.mRemovedText;

    return *this;
}

//--------------------------------------------------------------------------------------
void ModificationStackInternal::ModificationRemoval::ApplyModificationToString(std::wstring &str) const {
    auto begin = std::begin(str) + std::clamp(mStartIndex, (uint32_t) 0, static_cast<uint32_t>(str.size()));
    auto end = std::begin(str) + std::clamp(mEndIndex, (uint32_t) 0, static_cast<uint32_t>(str.size()));

    mRemovedText.assign(begin, end);
    str.erase(begin, end);
}

//--------------------------------------------------------------------------------------
void ModificationStackInternal::ModificationRemoval::RemoveModificationToString(std::wstring &str) const {
    auto begin = std::begin(str) + std::clamp(mStartIndex, (uint32_t) 0, static_cast<uint32_t>(str.size()));

    str.insert(begin, mRemovedText.begin(), mRemovedText.end());
    mRemovedText.clear();
}

//--------------------------------------------------------------------------------------
void ModificationStackInternal::ModificationRemovalAndAddition::ApplyModificationToString(std::wstring &str) const {
    ModificationRemoval::ApplyModificationToString(str);
    ModificationAddition::ApplyModificationToString(str);
}

//--------------------------------------------------------------------------------------
void ModificationStackInternal::ModificationRemovalAndAddition::RemoveModificationToString(std::wstring &str) const {
    ModificationAddition::RemoveModificationToString(str);
    ModificationRemoval::RemoveModificationToString(str);
}

ModificationStackInternal::ModificationRemovalAndAddition &ModificationStackInternal::ModificationRemovalAndAddition::operator=(const ModificationRemovalAndAddition &str) {
    mStartIndex = str.mStartIndex;
    mEndIndex = str.mEndIndex;
    mInsertLocation = str.mInsertLocation;
    mNewText = str.mNewText;
    mRemovedText = str.mRemovedText;

    return *this;
}

//--------------------------------------------------------------------------------------
ModificationStackInternal::ModificationRemovalAndAddition::ModificationRemovalAndAddition(const ModificationRemovalAndAddition &other)
        : ModificationRemoval(other), ModificationAddition(other) {
}

//--------------------------------------------------------------------------------------
ModificationStackInternal::ModificationRemovalAndAddition::ModificationRemovalAndAddition()
        : ModificationRemoval(), ModificationAddition() {
}


//--------------------------------------------------------------------------------------
void ModificationStackInternal::GenericCompositeModification::ApplyModificationToString(std::wstring &str) const {
    for (auto &it : mParts) {
        it->ApplyModificationToString(str);
    }
}

//--------------------------------------------------------------------------------------
void ModificationStackInternal::GenericCompositeModification::RemoveModificationToString(std::wstring &str) const {
    for (auto it = mParts.crbegin(); it != mParts.crend(); ++it) {
        (*it)->RemoveModificationToString(str);
    }
}

//--------------------------------------------------------------------------------------
void ModificationStackInternal::GenericCompositeModification::ClearParts() {
    mParts.clear();
}

}
