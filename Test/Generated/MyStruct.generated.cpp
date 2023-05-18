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
		Mngr.AddField<&nilou::MyStruct::base>("base");
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
        TStaticSerializer<int>::Serialize(this->a, local_Ar);
    }
    {
        FArchive local_Ar(content["base"], Ar);
        TStaticSerializer<nilou::Base *>::Serialize(this->base, local_Ar);
    }
    {
        FArchive local_Ar(content["vec"], Ar);
        TStaticSerializer<std::vector<float>>::Serialize(this->vec, local_Ar);
    }
}

void nilou::MyStruct::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("a"))
    {
        FArchive local_Ar(content["a"], Ar);
        TStaticSerializer<int>::Deserialize(this->a, local_Ar);
    }
    if (content.contains("base"))
    {
        FArchive local_Ar(content["base"], Ar);
        TStaticSerializer<nilou::Base *>::Deserialize(this->base, local_Ar);
    }
    if (content.contains("vec"))
    {
        FArchive local_Ar(content["vec"], Ar);
        TStaticSerializer<std::vector<float>>::Deserialize(this->vec, local_Ar);
    }
    
}
