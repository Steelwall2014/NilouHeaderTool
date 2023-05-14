#include "Test.h"

using namespace nilou;

int main()
{
    Base base;
    auto type = base.GetClass();
    auto desc = type->GetTypeDescriptor();
    auto base2 = desc->GetDefaultConstructor().Invoke();
    Base* base3 = reinterpret_cast<Base*>(base2);
    auto derived = desc->GetDefaultConstructor().Invoke();
    Derived* derived2 = reinterpret_cast<Derived*>(derived);
    return 0;
}