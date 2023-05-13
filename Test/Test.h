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

        UPROPERTY()
        int BaseField;

        UFUNCTION()
        void foo()
        {

        }

    };

    class NCLASS Derived : public Base
    {
        GENERATE_BODY()

    public:

        UPROPERTY()
        std::string DerivedField;

    };

}