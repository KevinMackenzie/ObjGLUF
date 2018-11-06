
namespace GLUF {

template<typename modType>
void ModificationStack::PushPartialModification(const modType &m) {
    mPartialMod.PushPart(m);
}


template<typename modType>
void ModificationStack::PushGeneric(const modType &m) {
    ModificationStackInternal::Modification *thisMod = nullptr;
    try {
        thisMod = dynamic_cast<ModificationStackInternal::Modification *>(new modType(m));
        if (thisMod == nullptr)
            throw;
        //*thisMod = m;
    }
    catch (...) {
        //'modType' is not a child of 'Modification'
        return;
    }

    mUndoStack.push(thisMod);

    thisMod->ApplyModificationToString(mText);

    //if something is added to the undo stack, why would the redo stack be relevent anymore?
    FlattenRedoStack();
}

template<typename modType>
void ModificationStackInternal::GenericCompositeModification::PushPart(const modType &m) {
    ModificationStackInternal::Modification *thisMod = nullptr;
    try {
        thisMod = dynamic_cast<ModificationStackInternal::Modification *>(new modType(m));
        if (thisMod == nullptr)
            throw;
        //*thisMod = m;
    }
    catch (...) {
        //'modType' is not a child of 'Modification'
        return;
    }

    mParts.push_back(std::shared_ptr<Modification>(thisMod));
}
}
