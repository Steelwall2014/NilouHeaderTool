#include "D:/NilouHeaderTool/Test/Test.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> Derived::StaticClass_ = nullptr;
const NClass *Derived::GetClass() const 
{ 
    return Derived::StaticClass(); 
}
const NClass *Derived::StaticClass()
{
    return Derived::StaticClass_.get();
}

template<>
struct TClassRegistry<Derived>
{
    TClassRegistry(const std::string& InName)
    {
        Derived::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<Derived>();
		Mngr.AddField<&Derived::DerivedField>("DerivedField");
		Mngr.AddBases<Derived, Base>();
;
        Derived::StaticClass_->Type = Type_of<Derived>;
        Derived::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<Derived>);
    }

    static TClassRegistry<Derived> Dummy;
};
TClassRegistry<Derived> Dummy = TClassRegistry<Derived>("Derived");


