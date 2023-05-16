#pragma once
#include <string>
#include <reflection/Class.h>
#include <reflection/Macros.h>

namespace nilou {


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

        }

    };

    class NCLASS Derived : public Base
    {
        GENERATED_BODY()

    public:

        NPROPERTY()
        std::string DerivedField;

    };

}