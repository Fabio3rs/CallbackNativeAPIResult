#include "WrapperCallbackCode.hpp"
#include <cstring>
#include <functional>
#include <string>
#include <string_view>

/*typedef int (*callbackcwd_t)(uintptr_t code, const char *workingdirectory,
size_t maxsize);

int getcwdcb(callbackcwd_t fn) { return 0; }

static std::string WorkingDirectory;

int main() {
    if (getcwdcb([](const char *workingdirectory, size_t maxsize) {
            size_t wkdirLen = maxsize == 0 ? strlen(workingdirectory)
                                           : strnlen(workingdirectory, maxsize);
            WorkingDirectory.assign(workingdirectory, wkdirLen);
            return 0;
        }) == 0) {
    }
}*/

typedef int (*callbackcwd_t)(uintptr_t code, const char *workingdirectory,
                             size_t maxsize);

/*
Simulates the idea of an working directory function that sends the result with a
callback
*/
int getcwdcb(uintptr_t code, callbackcwd_t fn) { return fn(code, "/home", 0); }

typedef int (*systemStartupCallback_t)(uintptr_t code, const char *json,
                                       size_t maxsize);

int inicializar(uintptr_t code, systemStartupCallback_t fn,
                const char *config) {
    std::cout << config << std::endl;
    return fn(code, R"({ "success": true })", 0);
}

static void exampleWorkingDirectory() {
    using namespace WrapperCallbackCode;
    std::string WorkingDirectory;

    {
        ScopedCallbackMgr<int, const char *, size_t> cb(
            [&WorkingDirectory](const char *workingdirectory, size_t maxsize) {
                size_t wkdirLen = maxsize == 0
                                      ? strlen(workingdirectory)
                                      : strnlen(workingdirectory, maxsize);
                WorkingDirectory.assign(workingdirectory, wkdirLen);
                return 0;
            });

        getcwdcb(cb.getCode(), decltype(cb)::callbackWrapper);
    }

    std::cout << "Supondo que este seja um working directory "
              << WorkingDirectory << std::endl;
}

static void exampleScopedCallback() {
    std::cout << __func__ << std::endl;
    using namespace WrapperCallbackCode;
    std::string SystemStartupResult;

    {
        ScopedCallbackMgr cb((std::function<int(const char *, size_t)>(
            [&SystemStartupResult](const char *json, size_t maxsize) {
                size_t wkdirLen =
                    maxsize == 0 ? strlen(json) : strnlen(json, maxsize);
                SystemStartupResult.assign(json, wkdirLen);
                return 0;
            })));

        inicializar(cb.getCode(), decltype(cb)::callbackWrapper, R"({})");
    }

    std::cout << "AAAAA " << SystemStartupResult << std::endl;
}

static void exampleDirectCallback() {
    std::cout << __func__ << std::endl;
    using namespace WrapperCallbackCode;
    std::string SystemStartupResult;

    {
        DirectCallbackMgr<int, const char *, size_t> cb(
            [&SystemStartupResult](const char *json, size_t maxsize) {
                size_t wkdirLen =
                    maxsize == 0 ? strlen(json) : strnlen(json, maxsize);
                SystemStartupResult.assign(json, wkdirLen);
                return 0;
            });

        inicializar(cb.getCode(), decltype(cb)::callbackWrapper, R"({})");
    }

    std::cout << "AAAAA " << SystemStartupResult << std::endl;
}

int main() {
    exampleWorkingDirectory();
    exampleScopedCallback();
    exampleDirectCallback();
}
