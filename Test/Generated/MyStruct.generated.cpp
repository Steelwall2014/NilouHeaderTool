#include "D:/NilouHeaderTool/Test/Test.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::MyStruct::StaticClass_ = nullptr;
const NClass *nilou::MyStruct::GetClass() const 
{ 
    return nilou::MyStruct::StaticClass(); 
}
const NClass *nilou::MyStruct::StaticClass()
{
    return nilou::MyStruct::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::MyStruct>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::MyStruct::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::MyStruct>();
		Mngr.AddField<&nilou::MyStruct::a>("a");
		Mngr.AddField<&nilou::MyStruct::arr>("arr");
		Mngr.AddField<&nilou::MyStruct::base>("base");
		Mngr.AddField<&nilou::MyStruct::m>("m");
		Mngr.AddField<&nilou::MyStruct::s>("s");
		Mngr.AddField<&nilou::MyStruct::vec>("vec");
;
        nilou::MyStruct::StaticClass_->Type = Type_of<nilou::MyStruct>;
        nilou::MyStruct::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::MyStruct>);
    }

    static TClassRegistry<nilou::MyStruct> Dummy;
};
TClassRegistry<nilou::MyStruct> Dummy = TClassRegistry<nilou::MyStruct>("nilou::MyStruct");



void nilou::MyStruct::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["a"], Ar);
        TStaticSerializer<decltype(this->a)>::Serialize(this->a, local_Ar);
    }
    {
        FArchive local_Ar(content["arr"], Ar);
        TStaticSerializer<decltype(this->arr)>::Serialize(this->arr, local_Ar);
    }
    {
        FArchive local_Ar(content["base"], Ar);
        TStaticSerializer<decltype(this->base)>::Serialize(this->base, local_Ar);
    }
    {
        FArchive local_Ar(content["m"], Ar);
        TStaticSerializer<decltype(this->m)>::Serialize(this->m, local_Ar);
    }
    {
        FArchive local_Ar(content["s"], Ar);
        TStaticSerializer<decltype(this->s)>::Serialize(this->s, local_Ar);
    }
    {
        FArchive local_Ar(content["vec"], Ar);
        TStaticSerializer<decltype(this->vec)>::Serialize(this->vec, local_Ar);
    }
}

void nilou::MyStruct::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("a"))
    {
        FArchive local_Ar(content["a"], Ar);
        TStaticSerializer<decltype(this->a)>::Deserialize(this->a, local_Ar);
    }
    if (content.contains("arr"))
    {
        FArchive local_Ar(content["arr"], Ar);
        TStaticSerializer<decltype(this->arr)>::Deserialize(this->arr, local_Ar);
    }
    if (content.contains("base"))
    {
        FArchive local_Ar(content["base"], Ar);
        TStaticSerializer<decltype(this->base)>::Deserialize(this->base, local_Ar);
    }
    if (content.contains("m"))
    {
        FArchive local_Ar(content["m"], Ar);
        TStaticSerializer<decltype(this->m)>::Deserialize(this->m, local_Ar);
    }
    if (content.contains("s"))
    {
        FArchive local_Ar(content["s"], Ar);
        TStaticSerializer<decltype(this->s)>::Deserialize(this->s, local_Ar);
    }
    if (content.contains("vec"))
    {
        FArchive local_Ar(content["vec"], Ar);
        TStaticSerializer<decltype(this->vec)>::Deserialize(this->vec, local_Ar);
    }
    
}
