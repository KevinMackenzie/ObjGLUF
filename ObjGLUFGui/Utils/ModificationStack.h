#ifndef OBJGLUF_MODIFICATIONSTACK_H
#define OBJGLUF_MODIFICATIONSTACK_H

/*
ModificationStack

    Note:
        -This class is designed to keep track of modifications made to a set of data

    Data Members:
        'mUndoStack': the stack which can be undone
        'mRedoStack': the stack which can be redone
        'mText': the current text as a result to the stack changes
*/

namespace ModificationStackInternal
{
    class Modification
    {
    public:
        virtual void ApplyModificationToString(std::wstring& str) const = 0;
        virtual void RemoveModificationToString(std::wstring& str) const = 0;
    };
    class ModificationAddition : virtual public Modification
    {
    public:
        uint32_t mInsertLocation;
        std::wstring mNewText;

        virtual ModificationAddition& operator=(const ModificationAddition& str);

        virtual void ApplyModificationToString(std::wstring& str) const;
        virtual void RemoveModificationToString(std::wstring& str) const;
    };
    class ModificationRemoval : virtual public Modification
    {
    public:
        uint32_t mStartIndex;
        uint32_t mEndIndex;
        mutable std::wstring mRemovedText;

        virtual ModificationRemoval& operator=(const ModificationRemoval& str);

        virtual void ApplyModificationToString(std::wstring& str) const;
        virtual void RemoveModificationToString(std::wstring& str) const;
    };
    class ModificationRemovalAndAddition : public ModificationAddition, public ModificationRemoval
    {
    public:

        virtual ModificationRemovalAndAddition& operator=(const ModificationRemovalAndAddition& str);
        ModificationRemovalAndAddition(const ModificationRemovalAndAddition& other);
        ModificationRemovalAndAddition();

        virtual void ApplyModificationToString(std::wstring& str) const;
        virtual void RemoveModificationToString(std::wstring& str) const;
    };
    class GenericCompositeModification : public Modification
    {
    protected:
        std::vector<std::shared_ptr<Modification>> mParts;
    public:

        void ClearParts();

        template<typename modType>
        void PushPart(const modType& mod);

        bool Empty() const noexcept{ return mParts.empty(); }

        virtual void ApplyModificationToString(std::wstring& str) const;
        virtual void RemoveModificationToString(std::wstring& str) const;
    };
}
class ModificationStack
{
    ModificationStackInternal::GenericCompositeModification mPartialMod;
    std::stack <ModificationStackInternal::Modification*> mRedoStack;
    std::stack <ModificationStackInternal::Modification*> mUndoStack;
    std::wstring mText;

    void FlattenRedoStack() noexcept;
    void FlattenUndoStack() noexcept;
public:
    ModificationStack(const std::wstring& initialText);

    void PushAddition(const std::wstring& addition, uint32_t loc);
    void PushRemoval(uint32_t startIndex, uint32_t endIndex);
    //void PushRemovalAndAddition(uint32_t startIndex, uint32_t endIndex, const std::wstring& addition);

    template<typename modType>
    void PushPartialModification(const modType& m);
    void ApplyPartialModifications() noexcept;

    void PushPartialAddition(const std::wstring& text, uint32_t loc);
    void PushPartialRemoval(uint32_t startIndex, uint32_t endIndex);

    template<typename modType>
    void PushGeneric(const modType& m);

    void UndoNextItem() noexcept;
    void RedoNextItem() noexcept;

    void FlattenStack() noexcept;

    void SetText(const std::wstring& text) noexcept;

    std::wstring Top() const noexcept{ return mText; }
};
#endif //OBJGLUF_MODIFICATIONSTACK_H
