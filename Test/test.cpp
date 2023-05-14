#include "Test.h"

using namespace nilou;

int main()
{
    Base base;
    auto type = base.GetClass();
    auto desc = type->GetTypeDescriptor();
    auto base2 = desc->GetConstructor().Invoke<int>(1);
    Base* base3 = reinterpret_cast<Base*>(base2);
    return 0;
}