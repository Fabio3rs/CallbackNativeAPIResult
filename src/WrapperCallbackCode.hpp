#pragma once

#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <sys/types.h>
#include <unordered_map>
#include <utility>

namespace WrapperCallbackCode {
class CallbackBase {
  public:
    virtual ~CallbackBase();
};

template <class T, class... Types> class TypedCbWrapper : public CallbackBase {
  public:
    T operator()(Types... args) {
        return callbackObj(std::forward<Types>(args)...);
    }

    std::function<T(Types...)> callbackObj;

    TypedCbWrapper() = default;
    TypedCbWrapper(std::function<T(Types...)> fn)
        : callbackObj(std::move(fn)) {}
};

class WrapperCbsData {

  public:
    static std::unordered_map<uintptr_t, std::unique_ptr<CallbackBase>> codes;
    static std::mutex mtx;
    static uintptr_t code;
};

template <class T, class... Types> class ScopedCallbackMgr {

  public:
    static T callbackWrapper(uintptr_t code, Types... args) {
        std::unique_ptr<CallbackBase> &fn = WrapperCbsData::codes.at(code);
        return dynamic_cast<TypedCbWrapper<T, Types...> *>(fn.get())
            ->callbackObj(std::forward<Types>(args)...);
    }

    [[nodiscard]] uintptr_t getCode() const { return code; }

    ScopedCallbackMgr(std::function<T(Types...)> fn) {
        auto callbackinst = std::make_unique<TypedCbWrapper<T, Types...>>(fn);

        std::lock_guard<std::mutex> lck(WrapperCbsData::mtx);
        code = WrapperCbsData::code++;

        auto base = std::unique_ptr<CallbackBase>(std::move(callbackinst));

        WrapperCbsData::codes.insert(std::make_pair(code, std::move(base)));
    }

    ~ScopedCallbackMgr() {
        std::lock_guard<std::mutex> lck(WrapperCbsData::mtx);
        WrapperCbsData::codes.erase(WrapperCbsData::codes.find(getCode()));
    }

  private:
    uintptr_t code{0};
};

template <class T, class... Types>
class DirectCallbackMgr : public CallbackBase {

  public:
    [[nodiscard]] uintptr_t getCode() const {
        return reinterpret_cast<uintptr_t>(this);
    }

    static T callbackWrapper(uintptr_t code, Types... args) {
        if (code == 0) {
            return {};
        }

        CallbackBase *base = reinterpret_cast<CallbackBase *>(code);
        auto *inst = dynamic_cast<DirectCallbackMgr<T, Types...> *>(base);

        if (inst == nullptr) {
            return {};
        }

        return inst->callbackObj(std::forward<Types>(args)...);
    }

    DirectCallbackMgr(std::function<T(Types...)> fn) : callbackObj(fn) {}

    ~DirectCallbackMgr() override = default;

  private:
    std::function<T(Types...)> callbackObj;
};
} // namespace WrapperCallbackCode
