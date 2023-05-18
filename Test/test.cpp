#include <iostream>
#include <reflection/Class.h>
#include "Test.h"
using namespace nilou;


int main()
{
    {
        nlohmann::json root;
        std::vector<FArchiveBuffer> Buffers;
        FArchive Ar(root, Buffers);
        Derived* derived = new Derived;
        derived->BaseField = 1;
        derived->my_struct = MyStruct{{1}, 2, derived};
        derived->DerivedField = "123456";
        derived->Buffer.Buffer = std::make_shared<unsigned char[]>(10);
        for (int i = 0; i < 10; i++)
        {
            derived->Buffer.Buffer.get()[i] = 'a'+i;
        }
        derived->Buffer.BufferSize = 10;
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