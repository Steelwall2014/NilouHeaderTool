#include "D:/NilouHeaderTool/Test/Test.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> Base::StaticClass_ = nullptr;
const NClass *Base::GetClass() const 
{ 
    return Base::StaticClass(); 
}
const NClass *Base::StaticClass()
{
    return Base::StaticClass_.get();
}

template<>
struct TClassRegistry<Base>
{
    TClassRegistry(const std::string& InName)
    {
        Base::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<Base>("Base")
				   .AddDefaultConstructor()
				   .AddMemberVariable("BaseField", &Base::BaseField)
				   .AddMemberFunction("foo", &Base::foo)
				   .AddDerivedClass("Derived")
;
        Base::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<Base> Dummy;
};
TClassRegistry<Base> Dummy = TClassRegistry<Base>("Base");


