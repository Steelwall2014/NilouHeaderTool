#include <iostream>
#include <reflection/Class.h>
#include "Test.h"
using namespace nilou;

struct X {
     int serialize(const std::string&) { return 42; } 
     void serialize(FArchive&) { }
};

int main()
{
    bool aaa = HasMethodSerialize<nilou::MyStruct, void(FArchive&)>::value;
    {
        nlohmann::json root;
        std::vector<FArchiveBuffer> Buffers;
        FArchive Ar(root, Buffers);
        Derived* derived = new Derived;
        derived->BaseField = 1;
        derived->my_struct = MyStruct{{1}, 2, derived};
        derived->DerivedField = "123456";
        FBinaryBuffer& Buffer = derived->Buffer.emplace_back();
        Buffer.Buffer = std::make_shared<unsigned char[]>(10);
        for (int i = 0; i < 10; i++)
        {
            Buffer.Buffer.get()[i] = 'a'+i;
        }
        Buffer.BufferSize = 10;
        derived->my_structs.push_back(derived->my_struct);
        derived->Serialize(Ar);
        std::ofstream out("test.nasset", std::ios::binary);
        out << Ar;
    }

    {
        std::ifstream in("test.nasset", std::ios::binary);
        nlohmann::json root;
        std::vector<FArchiveBuffer> Buffers;
        FArchive Ar(root, Buffers);
        in >> Ar;
        Base* pb = nullptr;
        TStaticSerializer<Base*>::Deserialize(pb, Ar);
    }

    return 0;
}