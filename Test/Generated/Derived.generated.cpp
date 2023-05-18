#include "D:/NilouHeaderTool/Test/Test.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::Derived::StaticClass_ = nullptr;
const NClass *nilou::Derived::GetClass() const 
{ 
    return nilou::Derived::StaticClass(); 
}
const NClass *nilou::Derived::StaticClass()
{
    return nilou::Derived::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::Derived>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::Derived::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::Derived>();
		Mngr.AddField<&nilou::Derived::Buffer>("Buffer");
		Mngr.AddField<&nilou::Derived::DerivedField>("DerivedField");
		Mngr.AddBases<nilou::Derived, nilou::Base>();
;
        nilou::Derived::StaticClass_->Type = Type_of<nilou::Derived>;
        nilou::Derived::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::Derived>);
    }

    static TClassRegistry<nilou::Derived> Dummy;
};
TClassRegistry<nilou::Derived> Dummy = TClassRegistry<nilou::Derived>("nilou::Derived");



void nilou::Derived::Serialize(FArchive& Ar)
{
    nilou::Base::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::Derived";
    nlohmann::json &content = Node["Content"];

    {
        FArchive local_Ar(content["Buffer"], Ar);
        TStaticSerializer<FBinaryBuffer>::Serialize(this->Buffer, local_Ar);
    }
    {
        FArchive local_Ar(content["DerivedField"], Ar);
        TStaticSerializer<std::string>::Serialize(this->DerivedField, local_Ar);
    }
    this->bIsSerializing = false;
}

void nilou::Derived::Deserialize(FArchive& Ar)
{
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    if (content.contains("Buffer"))
    {
        FArchive local_Ar(content["Buffer"], Ar);
        TStaticSerializer<FBinaryBuffer>::Deserialize(this->Buffer, local_Ar);
    }
    if (content.contains("DerivedField"))
    {
        FArchive local_Ar(content["DerivedField"], Ar);
        TStaticSerializer<std::string>::Deserialize(this->DerivedField, local_Ar);
    }
    nilou::Base::Deserialize(Ar);
    this->bIsSerializing = false;
}
