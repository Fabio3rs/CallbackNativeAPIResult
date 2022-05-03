# Utilizando ideias do paradigma funcional para resultado de operações sem usar out-arguments

## Introdução
É comum em C/C++/asm/etc. fazer funções que recebem argumentos de saída em interface externa de APIs para copiar o resultado de uma operação. Para citar um exemplo podemos verificar as seguintes funções de ler o diretório de trabalho atual:

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

Para ambas o usuário precisa passar um argumento de um endereço para um buffer pré alocado e o tamanho do mesmo, mas isso pode criar alguns problemas como alocação em excesso para locais que vão retornar um conteúdo pequeno ou alocação menor que o necessário caso o retorno seja muito grande, além de que em algumas situações pode ser confuso para um programador recém chegado ler o código e ver que é um argumento de saída pois não é óbvio ao olhar de primeira.

Pensando nos problemas acima, considerei que o uso de um callback quando o valor esteja disponível com um ponteiro somente leitura, de maneira a nunca passar endereços de buffers alocados em um local para serem escritos pelo outro local cruzando a barreira da interface da API.

### Exemplo de solução

Declaração:

```cpp
typedef int(*callbackcwd_t)(const char *workingdirectory, size_t maxsize);

int getcwdcb(callbackcwd_t fn);
```

Uso em C/C++:

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


Uso em C++ com lambda:
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

Entretanto tal alteranativa ainda não parece ideal pois considerando o uso em APIs de bibliotecas compartilhadas não tem como usar lambda com capture por exemplo (por não haver a possibilidade de passar para uma raw function pointer), daria então para passar um código gerado antes de chamar que identificaria de onde vem o callback, algo como:

```cpp
typedef int(*callbackcwd_t)(uintptr_t callbackCode, const char *workingdirectory, size_t maxsize);

int getcwdcb(uintptr_t callbackCode, callbackcwd_t fn);
```

Com a elaboração de alguns wrappers podemos chegar a algo parecido com isso:
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

    getcwdcb(cb.getCode(), decltype(cb)::callbackWrapper);

    std::cout << "Supondo que este seja um working directory "
              << WorkingDirectory << std::endl;
}
```

Consistindo então em um método seguro do ponto de vista de buffer overflows para solicitar informações para APIs externas.

Podem haver algumas considerações de perda de desempenho usando vários wrapper calls, mas pode valer a pena dependendo da situação, além de manter a interface da API menos suscetível a bugs de buffer overflow e também uma possível economia de memória considerando que não precisará ser alocado memória previamente e sim só no momento em que receber um ponteiro para a informação a ser copiada.

Uma opção diferente seria enviar um endereço de memória através do UserCode e talvez usar o dynamic_cast para conferir a validade do endereço, um pouco menos seguro, porém provavelmente mais eficiente em termos de ciclos de memória e CPU que a solução anterior pois não possui busca em map e também não possui mutex para garantir acesso de somente um thread ao map por vez:

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

        inicializar(R"({})", cb.getCode(), decltype(cb)::callbackWrapper);
    }

    std::cout << "AAAAA " << SystemStartupResult << std::endl;
}
```

Os exemplos acima tentam resolver a questão de passar o endereço de uma variável que está na stack sem necessidade de static com mutex/atomic extras, podendo então teoricamente operar em multi-threads.


