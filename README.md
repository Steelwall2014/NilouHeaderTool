# NilouHeaderTool
A tool to generate reflection codes. Reflection library UDRefl needs to be installed.
## Build
To build header tool:
```sh
xmake build -v NilouHeaderTool
```
To build test project:
```sh
xmake build -v Test
```
## Run
The first parameter is the directory of header files. The second parameter is the directory where the generated files will be placed. The third and subsequent parameters are optional, and they are the include directories that will be passes to libclang. E.g.
```sh
NilouHeaderTool D:/NilouHeaderTool/Test D:/NilouHeaderTool/Test/Generated D:/NilouHeaderTool/src/include
```
## How to use
0. Install UDRefl library: https://github.com/Ubpa/UDRefl
1. Include library
```c++
#include <reflection/Class.h>
#include <reflection/Macros.h>
```
2. Create a class called Base
```c++
#include <reflection/Class.h>
#include <reflection/Macros.h>

class Base
{
public:
    Base() { }
    Base(int a): BaseField(a) { }

    int BaseField;

    void foo()
    {
        cout << "foo";
    }
};
```
3. Add marks
```c++
#include <reflection/Class.h>
#include <reflection/Macros.h>

class NCLASS Base
{
    GENERATED_BODY()
public:
    Base() { }
    Base(int a): BaseField(a) { }

    NPROPERTY()
    int BaseField;

    NFUNCTION()
    void foo()
    {
        cout << "foo";
    }
};
```
4. At last, run header tool to generate reflection codes
```sh
NilouHeaderTool D:/NilouHeaderTool/Test D:/NilouHeaderTool/Test/Generated D:/NilouHeaderTool/src/include
```