#include "D:/NilouHeaderTool/Test/Test.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

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
        Mngr.RegisterType<Base>();
		Mngr.AddConstructor<Base>();
		Mngr.AddConstructor<Base, int>();
		Mngr.AddField<&Base::BaseField>("BaseField");
		Mngr.AddMethod<&Base::foo>("foo");
;
        Base::StaticClass_->Type = Type_of<Base>;
        Base::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<Base>);
    }

    static TClassRegistry<Base> Dummy;
};
TClassRegistry<Base> Dummy = TClassRegistry<Base>("Base");


