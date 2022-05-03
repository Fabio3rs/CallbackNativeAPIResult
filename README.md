# Using functional paradigm ideas to possible make C/C++ shared library string returns safer without out-arguments

## Introduction
It's common in C/C++/asm/etc to make functions with out-arguments in shared libraries to get a string result of an operation. For example we can analyze the get working directory function's prototype:

WinAPI:
```cpp
DWORD GetCurrentDirectory(
  [in]  DWORD  nBufferLength,
  [out] LPTSTR lpBuffer
);
```

POSIX:
```cpp
char *getcwd(char *buf, size_t size);
```

In both cases the user [programmer] needs to pass an argument of a buffer address pre-allocated with a fixed size and it's size, but this can create some problems like:
- Excess of allocation for read small values
- Lack of allocation for read bigger values
- Difficult to read the code line intent at first since looks like something entering the function but the data is comming from the function, the intent is not obvious at the first look.

The allocation size is fixed and hard to predict/choose a ideal size since the return can vary between 1 byte to thousands of bytes (for example the get current working directory function).

When programming a new interface that needs to output a string or something that requires a buffer some problems like buffer overflow can occour, thinking about that I was testing to use callbacks in shared libraries interfaces (.SO/.DLL) sending const pointer values with a know size to the callback function copy.

### Example of solution

Declaration:

```cpp
typedef int(*callbackcwd_t)(const char *workingdirectory, size_t maxsize);

int getcwdcb(callbackcwd_t fn);
```

Use C/C++:

```cpp

char *workingdirectory = NULL;

/****/


int callbackDiretorioAtual(const char *workingdirectory, size_t maxsize) {
    size_t wkdirLen = maxsize == 0? strlen(workingdirectory) :  strnlen(workingdirectory, maxsize);
    if (workingdirectory) free(workingdirectory);

    workingdirectory = malloc(wkdirLen + 1);
    strncpy(workingdirectory, workingdirectory, wkdirLen);
    workingdirectory[wkdirLen] = '\0';
    return 0;
}

/*****/


    if (getcwdcb(callbackDiretorioAtual) == 0)
    {
    
    }
```


Use in C++ with lambda:
```cpp
std::string WorkingDirectory;

/****/
    if (getcwdcb([](const char *workingdirectory, size_t maxsize) {
            size_t wkdirLen = maxsize == 0 ? strlen(workingdirectory)
                                           : strnlen(workingdirectory, maxsize);
            WorkingDirectory.assign(workingdirectory, wkdirLen);
            return 0;
        }) == 0) {
    }
```

However this method can not be the ideal since shared library APIs we probably can't use lambda with capture since they can not be casted to a raw function pointer. We can try design the function that receives a code and re-transmites to the callback. Storing the object in the heap and pass a integer code to the function, when the callback calls we can verify the code and calls the original lambda.
Something like this:

```cpp
typedef int(*callbackcwd_t)(uintptr_t callbackCode, const char *workingdirectory, size_t maxsize);

int getcwdcb(uintptr_t callbackCode, callbackcwd_t fn);
```

Designing some wrappers we can get something like this:
```cpp
typedef int (*callbackcwd_t)(uintptr_t code, const char *workingdirectory,
                             size_t maxsize);

int getcwdcb(uintptr_t code, callbackcwd_t fn) { return fn(code, "/home", 0); }

int main() {
    using namespace WrapperCallbackCode;
    std::string WorkingDirectory;

    ScopedCallbackMgr cb((std::function<int(const char *, size_t)>(
        [&WorkingDirectory](const char *workingdirectory, size_t maxsize) {
            size_t wkdirLen = maxsize == 0 ? strlen(workingdirectory)
                                           : strnlen(workingdirectory, maxsize);
            WorkingDirectory.assign(workingdirectory, wkdirLen);
            return 0;
        })));

    inicializar(cb.getCode(), decltype(cb)::callbackWrapper, R"({})");

    std::cout << "Supose this can be a current working directory "
              << WorkingDirectory << std::endl;
}
```

This theoretically can be used in multi-thread applications.

Can be some considerations of loss of performance using various call wrappers, but for the memory security sandpoint can be worth.

A different option can be to send the memory address casted a uintptr_t through the UserCode and cast it back in some callback wrapper, a bit less secure since the it's hader to check the pointer validity, maybe using the dynamic_cast to add a layer of validation of the address type. Can be faster than the previous solution since it not uses a map and neither a mutex, and maybe only the std::function allocates in the heap. In thesis it continues compatible with multi-threading and probably faster since it has no mutex.

```cpp
static void exampleDirectCallback() {
    std::cout << __func__ << std::endl;
    using namespace WrapperCallbackCode;
    std::string SystemStartupResult;

    {
        DirectCallbackMgr cb((std::function<int(const char *, size_t)>(
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
```

