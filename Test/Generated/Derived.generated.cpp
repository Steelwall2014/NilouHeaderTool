#include "D:/NilouHeaderTool/Test/Test.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<UClass> Derived::StaticClass_ = nullptr;
const UClass *Derived::GetClass() 
{ 
    return Derived::StaticClass(); 
}
const UClass *Derived::StaticClass()
{
    return Derived::StaticClass_.get();
}

template<>
struct TClassRegistry<Derived>
{
    TClassRegistry(const std::string& InName)
    {
        Derived::StaticClass_ = std::make_unique<UClass>();
        reflection::AddClass<Derived>("Derived")
				   .AddConstructor<>()
				   .AddMemberVariable("DerivedField", &Derived::DerivedField)
				   .AddParentClass("Base")
;
        Derived::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<Derived> Dummy;
};
TClassRegistry<Derived> Dummy = TClassRegistry<Derived>("Derived");


