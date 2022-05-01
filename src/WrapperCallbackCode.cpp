#include "WrapperCallbackCode.hpp"

std::unordered_map<uintptr_t,
                   std::unique_ptr<WrapperCallbackCode::CallbackBase>>
    WrapperCallbackCode::WrapperCbsData::codes{};
uintptr_t WrapperCallbackCode::WrapperCbsData::code{};
std::mutex WrapperCallbackCode::WrapperCbsData::mtx{};

WrapperCallbackCode::CallbackBase::~CallbackBase() = default;
