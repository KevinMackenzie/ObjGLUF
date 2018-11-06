
namespace GLUF {

//--------------------------------------------------------------------------------------
template<typename T>
std::shared_ptr <T> Dialog::GetControl(ControlIndex ID) const {
    // Try to find the control with the given ID
    for (auto it : mControls) {
        if (it.second->GetID() == ID) {
            return std::dynamic_pointer_cast<T>(it.second);
        }
    }

    GLUF_NON_CRITICAL_EXCEPTION(std::invalid_argument("Control ID Not Found"));

    // Not found
    return nullptr;
}
}
