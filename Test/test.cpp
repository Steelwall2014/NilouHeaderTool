#include "Test.h"

using namespace nilou;

int main()
{
    Base base;
    auto type = base.GetClass();
    auto desc = type->GetTypeDescriptor();
    auto base2 = desc->GetConstructor().Invoke<float>(1.0);
    Base* base3 = std::any_cast<Base*>(base2);
    return 0;
}