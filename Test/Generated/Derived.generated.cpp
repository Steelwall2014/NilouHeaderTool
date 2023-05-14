#include "D:/NilouHeaderTool/Test/Test.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

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
        reflection::AddClass<Derived>("Derived")
				   .AddDefaultConstructor()
				   .AddMemberVariable("DerivedField", &Derived::DerivedField)
				   .AddParentClass("Base")
;
        Derived::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<Derived> Dummy;
};
TClassRegistry<Derived> Dummy = TClassRegistry<Derived>("Derived");


