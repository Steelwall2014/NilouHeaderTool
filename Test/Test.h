#pragma once
#include <string>
#include <reflection/TypeDescriptor.h>
#include <reflection/Registry.h>
#include <reflection/Class.h>
#include <reflection/Macros.h>

namespace nilou {


    class NCLASS Base
    {
        GENERATE_BODY()

    public:

        Base() { }

        Base(int a): BaseField(a) { }

        NPROPERTY()
        int BaseField;

        NFUNCTION()
        void foo()
        {

        }

    };

    class NCLASS Derived : public Base
    {
        GENERATE_BODY()

    public:

        NPROPERTY()
        std::string DerivedField;

    };

}