#include "D:/NilouHeaderTool/Test/Test.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<UClass> Base::StaticClass_ = nullptr;
const UClass *Base::GetClass() 
{ 
    return Base::StaticClass(); 
}
const UClass *Base::StaticClass()
{
    return Base::StaticClass_.get();
}

template<>
struct TClassRegistry<Base>
{
    TClassRegistry(const std::string& InName)
    {
        Base::StaticClass_ = std::make_unique<UClass>();
        reflection::AddClass<Base>("Base")
				   .AddConstructor<>()
				   .AddConstructor<int>()
				   .AddMemberVariable("BaseField", &Base::BaseField)
				   .AddMemberFunction("foo", &Base::foo)
				   .AddDerivedClass("Derived")
;
        Base::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<Base> Dummy;
};
TClassRegistry<Base> Dummy = TClassRegistry<Base>("Base");


