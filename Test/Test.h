#pragma once
#include <vector>
#include <set>
#include <string>
#include <memory>
#include <reflection/Class.h>
#include <reflection/Macros.h>
#include <json/json.hpp>

namespace nilou {
class Base;

enum class Enum
{
    E1,
    E2
};

class NSTRUCT MyStruct
{
    GENERATED_STRUCT_BODY()

public:

    NPROPERTY()
    std::vector<float> vec;

    NPROPERTY()
    int a = 1;

    NPROPERTY()
    Base* base;

    NPROPERTY()
    std::vector<Enum> e;

};

class Derived;
class NCLASS Base : public NObject
{
    GENERATED_BODY()

public:

    Base() { }

    Base(int a): BaseField(a) { }

    NPROPERTY()
    int BaseField;

    NPROPERTY()
    MyStruct my_struct;

    NPROPERTY()
    std::vector<MyStruct> my_structs;

    NPROPERTY()
    std::shared_ptr<Derived> ref;

    NFUNCTION()
    void foo()
    {

    }

};

class NCLASS Derived : public Base
{
    GENERATED_BODY()

public:

    NPROPERTY()
    std::string DerivedField;

    NPROPERTY()
    std::vector<FBinaryBuffer> Buffer;

};


}
