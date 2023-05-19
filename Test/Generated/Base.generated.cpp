#include "D:/NilouHeaderTool/Test/Test.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::Base::StaticClass_ = nullptr;
const NClass *nilou::Base::GetClass() const 
{ 
    return nilou::Base::StaticClass(); 
}
const NClass *nilou::Base::StaticClass()
{
    return nilou::Base::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::Base>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::Base::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::Base>();
		Mngr.AddField<&nilou::Base::BaseField>("BaseField");
		Mngr.AddField<&nilou::Base::my_struct>("my_struct");
		Mngr.AddField<&nilou::Base::my_structs>("my_structs");
		Mngr.AddField<&nilou::Base::ref>("ref");
		Mngr.AddMethod<&nilou::Base::foo>("foo");
		Mngr.AddBases<nilou::Base, NObject>();
;
        nilou::Base::StaticClass_->Type = Type_of<nilou::Base>;
        nilou::Base::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::Base>);
    }

    static TClassRegistry<nilou::Base> Dummy;
};
TClassRegistry<nilou::Base> Dummy = TClassRegistry<nilou::Base>("nilou::Base");



void nilou::Base::Serialize(FArchive& Ar)
{
    NObject::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::Base";
    nlohmann::json &content = Node["Content"];

    {
        FArchive local_Ar(content["BaseField"], Ar);
        TStaticSerializer<decltype(this->BaseField)>::Serialize(this->BaseField, local_Ar);
    }
    {
        FArchive local_Ar(content["my_struct"], Ar);
        this->my_struct.Serialize(local_Ar);
    }
    {
        FArchive local_Ar(content["my_structs"], Ar);
        TStaticSerializer<decltype(this->my_structs)>::Serialize(this->my_structs, local_Ar);
    }
    {
        FArchive local_Ar(content["ref"], Ar);
        TStaticSerializer<decltype(this->ref)>::Serialize(this->ref, local_Ar);
    }
    this->bIsSerializing = false;
}

void nilou::Base::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    if (content.contains("BaseField"))
    {
        FArchive local_Ar(content["BaseField"], Ar);
        TStaticSerializer<decltype(this->BaseField)>::Deserialize(this->BaseField, local_Ar);
    }
    if (content.contains("my_struct"))
    {
        FArchive local_Ar(content["my_struct"], Ar);
        this->my_struct.Deserialize(local_Ar);
    }
    if (content.contains("my_structs"))
    {
        FArchive local_Ar(content["my_structs"], Ar);
        TStaticSerializer<decltype(this->my_structs)>::Deserialize(this->my_structs, local_Ar);
    }
    if (content.contains("ref"))
    {
        FArchive local_Ar(content["ref"], Ar);
        TStaticSerializer<decltype(this->ref)>::Deserialize(this->ref, local_Ar);
    }
    NObject::Deserialize(Ar);
}
