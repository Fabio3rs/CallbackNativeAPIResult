#include <cstdint>
#include <cstring>
#include <functional>
#include <string>
#include <string_view>

/*typedef int (*callbackcwdcode_t)(const char *workingdirectory, size_t maxsize,
                                 uintptr_t callbackCode);

int getcwdcb(callbackcwdcode_t fn, uintptr_t callbackCode) { return 0; }

int cbCodeWrapper(const char *workingdirectory, size_t maxsize,
                  uintptr_t callbackCode);

uintptr_t registerCb(std::function<int(const char *, size_t, uintptr_t)> cb);

std::string getWorkingDirectory() {
    std::string WorkingDirectory;

    getcwdcb(
        cbCodeWrapper,
        registerCb([&WorkingDirectory](const char *workingdirectory,
                                       size_t maxsize, uintptr_t callbackCode) {
            size_t wkdirLen = maxsize == 0 ? strlen(workingdirectory)
                                           : strnlen(workingdirectory, maxsize);
            WorkingDirectory.assign(workingdirectory, wkdirLen);
            return 0;
        }));

    return WorkingDirectory;
}
*/