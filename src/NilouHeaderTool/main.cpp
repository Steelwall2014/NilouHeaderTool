#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <filesystem>
#include <clang-c/Index.h>
#include <regex>

#include "utils.h"

using namespace std;

class Directory
{
public:
    Directory(const std::string &InDirectoryName) : DirectoryName(InDirectoryName) { }
    const std::string &GetDirectoryName() const;

    template <typename Func>
    void ForEachFile(bool bFindInChildren, Func&& InFunc)
    {
        if (!std::filesystem::exists(DirectoryName))
        {
            std::cout << "Directory: " + DirectoryName + " doesn't exist" << std::endl;
            return;
        }
            
        for (const std::filesystem::directory_entry & dir_entry : 
            std::filesystem::recursive_directory_iterator(DirectoryName))
        {
            if (!dir_entry.is_directory())
            {
                std::string filepath = dir_entry.path().generic_string();
                InFunc(filepath);
            }
        }
    }

private:

    std::string DirectoryName;
};

ostream& operator<<(ostream& stream, const CXString& str)
{
    const char* s = clang_getCString(str);
    stream << s;
    clang_disposeString(str);
    return stream;
}

std::vector<const char*> arguments = {
    "-x",
    "c++",
    "-std=c++20",
    "-D __clang__",
    "-D __META_PARSER__"
};

struct TypeMetaData
{
    string FileName;
    string Name;
    string BaseClass;
    set<string> DerivedClasses;
    map<string, string> Fields;
    set<string> Methods;
    vector<vector<string>> Constructors;
    string GeneratedFileCode;
    string MetaType; // class or struct
};
map<string, TypeMetaData> NTypes;
string current_filepath;

string fully_qualified(CXCursor c)
{
    if (clang_getCursorKind(c) == CXCursorKind::CXCursor_TranslationUnit || 
        clang_getCursorKind(c) == CXCursorKind::CXCursor_FirstInvalid)
        return "";
    else
    {
        string res = fully_qualified(clang_getCursorSemanticParent(c));
        if (res != "")
            return res + "::" + GetCursorSpelling(c);
    }
    return GetCursorSpelling(c);
}

bool IsReflectedStruct(const string& TypeName)
{
    return NTypes.contains(TypeName) && NTypes[TypeName].MetaType == "struct";
}

bool IsReflectedClass(const string& TypeName)
{
    return NTypes.contains(TypeName) && NTypes[TypeName].MetaType == "class";
}

bool IsNClassPtr(const string& T)
{
    bool is_ptr = T.find("*") != -1;
    string raw_T = regex_replace(T, regex("(const )| \\*|\\* "), "");
    if (is_ptr && NTypes.contains(raw_T))
        return true;
    return false;
}

bool IsNClassSmartPtr(const string& T)
{
    regex ptr_re(R"(std::shared_ptr<(.+)>)");
    smatch match;
    if (regex_match(T, match, ptr_re) && NTypes.contains(match[1].str()))
    {
        return true;
    }
    return false;
}

bool IsNClassPtr(CXCursor c)
{
    string TypeName = GetCursorTypeSpelling(c);
    bool is_reflected_class = false;
    if (TypeName.find("*") != -1)
    {
        clang_visitChildren(
            c,
            [](CXCursor c, CXCursor parent, CXClientData client_data)
            {
                if (clang_getCursorKind(c) == CXCursorKind::CXCursor_TypeRef)
                {
                    string s = GetCursorSpelling(c);
                    s = regex_replace(s, regex("class "), "");
                    *(bool*)client_data = IsReflectedClass(s);
                    return CXChildVisit_Break;
                }
                return CXChildVisit_Recurse;
            },
            &is_reflected_class);
    }
    return is_reflected_class;
}

bool IsNClassSmartPtr(CXCursor c)
{
    string TypeName = GetCursorTypeSpelling(c);
    bool is_reflected_class = false;
    if (regex_search(TypeName, regex(R"(std::shared_ptr<.+>)")))
    {
        clang_visitChildren(
            c,
            [](CXCursor c, CXCursor parent, CXClientData client_data)
            {
                if (clang_getCursorKind(c) == CXCursorKind::CXCursor_TypeRef)
                {
                    string s = GetCursorSpelling(c);
                    s = regex_replace(s, regex("class "), "");
                    *(bool*)client_data = IsReflectedClass(s);
                    return CXChildVisit_Break;
                }
                return CXChildVisit_Recurse;
            },
            &is_reflected_class);
    }
    return is_reflected_class;
}

bool IsNStructOrBuiltin(CXCursor c)
{
    static set<string> built_ins = {
        "int8",
        "int16",
        "int32",
        "int64",
        "char",
        "short",
        "int",
        "long",
        "long long",
        "uint8",
        "uint16",
        "uint32",
        "uint64",
        "unsigned char",
        "unsigned short",
        "unsigned int",
        "unsigned long",
        "unsigned long long",
        "float",
        "double",
        "vec2",
        "vec3",
        "vec4",
        "dvec2",
        "dvec3",
        "dvec4",
        "ivec2",
        "ivec3",
        "ivec4",
        "uvec2",
        "uvec3",
        "uvec4",
        "mat2",
        "mat3",
        "mat4",
        "dmat2",
        "dmat3",
        "dmat4",
        "imat2",
        "imat3",
        "imat4",
        "umat2",
        "umat3",
        "umat4",
        "std::string",
        "FBinaryBuffer"
    };
    string TypeName = GetCursorTypeSpelling(c);
    if (built_ins.contains(TypeName) || IsReflectedStruct(TypeName))
        return true;
    return false;
}

bool IsSupportedContainer(CXCursor c)
{
    string TypeName = GetCursorTypeSpelling(c);
    bool is_supported_container = true;
    if (regex_search(TypeName, regex(R"(std::(vector|map|set|unordered_map|unordered_set)<(.+)>)")))
    {
        clang_visitChildren(
            c,
            [](CXCursor c, CXCursor parent, CXClientData client_data)
            {
                string kind = GetCursorKindSpelling(c);
                string s = GetCursorSpelling(c);
                if (clang_getCursorKind(c) == CXCursorKind::CXCursor_TypeRef)
                {
                    if (!IsNStructOrBuiltin(c) && !IsNClassPtr(c) && !IsNClassSmartPtr(c))
                        *(bool*)client_data = false;
                }
                return CXChildVisit_Recurse;
            },
            &is_supported_container);
    }
    return is_supported_container;
}

string GetSmartPtrRawType(const string& T)
{
    regex ptr_re(R"(std::shared_ptr<(.+)>)");
    smatch match;
    if (regex_match(T, match, ptr_re))
    {
        return match[1].str();
    }
    return "";
}

string GetRawType(const string& T)
{
    string raw_T = regex_replace(T, regex("(const |class )|\\*|&"), "");
    raw_T = regex_replace(raw_T, regex(" "), "");
    return raw_T;
}

bool IsSupportedType(const std::string& TypeName)
{
    if (IsNClassPtr(TypeName))
        return true;
    if (IsNClassSmartPtr(TypeName))
        return true;
    regex stl_re(R"(std::(vector|map|set|unordered_map|unordered_set)<(.+)>)");
    smatch match;
    if (regex_match(TypeName, match, stl_re))
    {
        vector<string> Ts = Split(match[2].str(), ',');
        for (auto& T : Ts)
        {
            if (!IsSupportedType(T))
                return false;
            // bool is_ptr = T.find("*") != -1;
            // string raw_T = regex_replace(T, regex("(const )|\\*"), "");
            // if (is_ptr)
            // {
            //     if (!NTypes.contains(raw_T))
            //         return false;
            // }
            // else 
            // {
            //     if (!IsReflectedStruct(raw_T) && !built_ins.contains(raw_T))
            //         return false;
            // }
        }
        return true;
    }
    return false;
}

bool IsSupportedType(CXCursor c)
{
    if (IsSupportedContainer(c))
        return true;
    if (IsNStructOrBuiltin(c))
        return true;
    if (IsNClassPtr(c))
        return true;
    if (IsNClassSmartPtr(c))
        return true;
    return false;
}

bool IsReflectedType(const std::string& TypeName)
{
    return NTypes.contains(TypeName);
}

bool NeedsReflection(string filepath)
{
    vector<const char*> arguments = {
        "-x",
        "c++-cpp-output"
    };
    CXIndex index = clang_createIndex(0, 0);
    CXTranslationUnit unit = clang_parseTranslationUnit(
        index,
        filepath.c_str(), arguments.data(), (int)arguments.size(),
        nullptr, 0,
        CXTranslationUnit_None);
    if (unit == nullptr)
    {
        cerr << "Unable to parse translation unit. Quitting." << endl;
        return false;
    }
    CXCursor cursor = clang_getTranslationUnitCursor(unit);
    bool needs_reflection = false;
    clang_visitChildren(
        cursor,
        [](CXCursor c, CXCursor parent, CXClientData client_data)
        {
            string s = GetCursorSpelling(c);
            if (s == "NCLASS" || s == "NPROPERTY")
            {
                bool* needs_reflection = reinterpret_cast<bool*>(client_data);
                *needs_reflection = true;
                return CXChildVisit_Break;
            }
            return CXChildVisit_Recurse;
        },
        &needs_reflection);
    return needs_reflection;
}

void ParseHeaderFile(string filepath)
{
    current_filepath = filepath;
    CXIndex index = clang_createIndex(0, 0);
    CXTranslationUnit unit = clang_parseTranslationUnit(
        index,
        filepath.c_str(), arguments.data(), (int)arguments.size(),
        nullptr, 0,
        CXTranslationUnit_None);
    if (unit == nullptr)
    {
        cerr << "Unable to parse translation unit. Quitting." << endl;
        return;
    }
    vector<CXCursor> reflection_classes;
    CXCursor cursor = clang_getTranslationUnitCursor(unit);
    clang_visitChildren(
        cursor,
        [](CXCursor c, CXCursor parent, CXClientData client_data)
        {
            string s = GetCursorSpelling(c);
            if (clang_getCursorKind(c) == CXCursor_AnnotateAttr) 
            {
                string class_name = fully_qualified(parent);
                if ((s == "reflect-class" || s == "reflect-struct") && !NTypes.contains(class_name)) 
                {
                    vector<CXCursor>* reflection_classes = reinterpret_cast<vector<CXCursor>*>(client_data);
                    reflection_classes->push_back(parent);
                    NTypes[class_name].Name = class_name;
                    NTypes[class_name].FileName = current_filepath;
                    if (s == "reflect-class")
                        NTypes[class_name].MetaType = "class";
                    else if (s == "reflect-struct")
                        NTypes[class_name].MetaType = "struct";
                }
            }
            
            return CXChildVisit_Recurse;
        },
        &reflection_classes);

    for (auto& cursor : reflection_classes)
    {
        clang_visitChildren(
            cursor,
            [](CXCursor c, CXCursor parent, CXClientData client_data)
            {
                string cursor_spelling = fully_qualified(c);
                string cursor_kind = GetCursorKindSpelling(c);
                auto cursor_kind_raw = clang_getCursorKind(c);

                if (cursor_kind_raw == CXCursor_Constructor)
                {
                    string class_name = cursor_spelling;
                    string method_name = fully_qualified(parent);
                    string method_args = GetCursorTypeSpelling(parent);
                    if (IsReflectedType(class_name))
                    {
                        vector<string> args;
                        int args_num = clang_Cursor_getNumArguments(c);
                        for (int i = 0; i < args_num; i++)
                        {
                            auto type = GetCursorTypeSpelling(clang_Cursor_getArgument(c, i));
                            args.push_back(type);
                        }
                        NTypes[class_name].Constructors.push_back(args);
                    }
                }
                else if (cursor_kind_raw == CXCursor_AnnotateAttr) 
                {
                    if (cursor_spelling == "reflect-property") 
                    {
                        CXCursor class_cursor = clang_getCursorSemanticParent(parent);
                        string class_name = fully_qualified(class_cursor);
                        string field_name = GetCursorSpelling(parent);
                        string field_type = GetCursorTypeSpelling(parent);
                        if (IsReflectedType(class_name) && IsSupportedType(parent))
                        {
                            auto& Fields = NTypes[class_name].Fields;
                            Fields[field_name] = field_type;
                        }
                    }
                    else if (cursor_spelling == "reflect-method")
                    {
                        CXCursor class_cursor = clang_getCursorSemanticParent(parent);
                        string class_name = fully_qualified(class_cursor);
                        string method_name = GetCursorSpelling(parent);
                        string method_args = GetCursorTypeSpelling(parent);
                        if (NTypes.contains(class_name))
                        {
                            auto& Methods = NTypes[class_name].Methods;
                            Methods.insert(method_name);
                        }
                    }
                    
                }
                else if (cursor_kind_raw == CXCursor_CXXBaseSpecifier) 
                {
                    vector<string> tokens = Split(cursor_spelling, ':');
                    string base_class = cursor_spelling;
                    base_class = regex_replace(base_class, regex("class "), "");
                    string derived_class = fully_qualified(parent);
                    NTypes[base_class].DerivedClasses.insert(derived_class);
                    NTypes[derived_class].BaseClass = GetRawType(base_class);
                }
                
                return CXChildVisit_Recurse;
            },
            nullptr);
    }

    
    clang_disposeTranslationUnit(unit);
    clang_disposeIndex(index);
    
}

static const string& indent = "\t\t";

string GenerateTypeRegistry(const TypeMetaData& NClass)
{
    const auto ClassName = NClass.Name;
    string CtorBody;
    for (auto& Args : NClass.Constructors)
    {
        string args;
        for (int i = 0; i < Args.size(); i++)
        {
            args += ", " + Args[i];
        }
        CtorBody += indent+format("Mngr.AddConstructor<{}{}>();\n", ClassName, args);
    }
    string FieldsBody;
    for (auto& [FieldName, FieldType] : NClass.Fields)
    {
        FieldsBody += indent+format("Mngr.AddField<&{1}::{0}>(\"{0}\");\n", 
            FieldName, ClassName);
    }
    string MethodsBody;
    for (auto& MethodName : NClass.Methods)
    {
        MethodsBody += indent+format("Mngr.AddMethod<&{1}::{0}>(\"{0}\");\n", 
            MethodName, ClassName);
    }
    string ClassHierarchyBody;
    if (NClass.BaseClass != "")
    {
        ClassHierarchyBody += indent+format("Mngr.AddBases<{}, {}>();\n", 
            ClassName, NClass.BaseClass);
    }
    return format(
R"(
std::unique_ptr<NClass> {0}::StaticClass_ = nullptr;
const NClass *{0}::GetClass() const 
{{ 
    return {0}::StaticClass(); 
}}
const NClass *{0}::StaticClass()
{{
    return {0}::StaticClass_.get();
}}

template<>
struct TClassRegistry<{0}>
{{
    TClassRegistry(const std::string& InName)
    {{
        {0}::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<{0}>();
{1}{2}{3}{4};
        {0}::StaticClass_->Type = Type_of<{0}>;
        {0}::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<{0}>);
    }}

    static TClassRegistry<{0}> Dummy;
}};
TClassRegistry<{0}> Dummy = TClassRegistry<{0}>("{0}");

)", ClassName, CtorBody, FieldsBody, MethodsBody, ClassHierarchyBody, NClass.BaseClass);
}

pair<string, string> GenerateSerializeBody(const string& FieldName, const string& FieldType)
{
    string SerializeBody, DeserializeBody;
    if (IsReflectedStruct(FieldType))
    {
        SerializeBody += format(R"(
    {{
        FArchive local_Ar(content["{0}"], Ar);
        this->{0}.Serialize(local_Ar);
    }})", FieldName);
        DeserializeBody += format(R"(
    if (content.contains("{0}"))
    {{
        FArchive local_Ar(content["{0}"], Ar);
        this->{0}.Deserialize(local_Ar);
    }})", FieldName);
    }
    else 
    {
        SerializeBody += format(R"(
    {{
        FArchive local_Ar(content["{0}"], Ar);
        TStaticSerializer<{1}>::Serialize(this->{0}, local_Ar);
    }})", FieldName, FieldType);
        DeserializeBody += format(R"(
    if (content.contains("{0}"))
    {{
        FArchive local_Ar(content["{0}"], Ar);
        TStaticSerializer<{1}>::Deserialize(this->{0}, local_Ar);
    }})", FieldName, FieldType);
    }
    return { SerializeBody, DeserializeBody };
}

string GenerateClassSerialize(const TypeMetaData& NClass)
{
    const auto ClassName = NClass.Name;

    string SerializeBody, DeserializeBody;
    for (auto& [FieldName, FieldType] : NClass.Fields)
    {
        auto [serialize_body, deserialize_body] = GenerateSerializeBody(FieldName, FieldType);
        SerializeBody += serialize_body;
        DeserializeBody += deserialize_body;
    }

    string BaseSerialize, BaseDeserialize;
    if (NClass.BaseClass != "")
    {
        BaseSerialize = format("{}::Serialize(Ar);", NClass.BaseClass);
        BaseDeserialize = format("{}::Deserialize(Ar);", NClass.BaseClass);
    }

    if (NClass.MetaType == "class")
    {
        return format(
R"(
void {0}::Serialize(FArchive& Ar)
{{
    {1}
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "{0}";
    nlohmann::json &content = Node["Content"];
{2}
    this->bIsSerializing = false;
}}

void {0}::Deserialize(FArchive& Ar)
{{
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];
{3}
    {4}
    this->bIsSerializing = false;
}}
)", ClassName, BaseSerialize, SerializeBody, DeserializeBody, BaseDeserialize);
    }
    else 
    {
        return format(
R"(
void {0}::Serialize(FArchive& Ar)
{{
    {1}
    nlohmann::json &content = Ar.Node;
{2}
}}

void {0}::Deserialize(FArchive& Ar)
{{
    nlohmann::json &content = Ar.Node;
{3}
    {4}
}}
)", ClassName, BaseSerialize, SerializeBody, DeserializeBody, BaseDeserialize);
    }
}

void GenerateCode()
{
    for (auto& [ClassName, NClass] : NTypes)
    {
            string TypeRegistry = GenerateTypeRegistry(NClass);
            string Serialize = GenerateClassSerialize(NClass);
            NClass.GeneratedFileCode = format(R"(#include "{}"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;
{}
{})", NClass.FileName, TypeRegistry, Serialize);
    }
}

void WriteCode(string GeneratedCodePath)
{
    for (auto& [ClassName, NClass] : NTypes)
    {
        auto tokens = Split(ClassName, ':');
        string raw_class_name = tokens[tokens.size()-1];
        ofstream out_stream(GeneratedCodePath + "/" + raw_class_name + ".generated.cpp", ios::out);
        out_stream << NClass.GeneratedFileCode;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        cout << "Usage: HeaderTool <src directory> <generated code directory> <include path 0> <include path 1> ..." << endl;
        return -1;
    }
    std::string DirectoryName = argv[1];
    //std::string DirectoryName = "./src"; //argv[1];
    if (DirectoryName[DirectoryName.size()-1] == '\\' || DirectoryName[DirectoryName.size()-1] == '/')
        DirectoryName = DirectoryName.substr(0, DirectoryName.size()-1);
    std::string GeneratedCodePath = argv[2];
    //std::string GeneratedCodePath = "./src/Runtime/Generated"; //argv[2];
    if (GeneratedCodePath[GeneratedCodePath.size()-1] == '\\' || GeneratedCodePath[GeneratedCodePath.size()-1] == '/')
        GeneratedCodePath = GeneratedCodePath.substr(0, GeneratedCodePath.size()-1);

    for (int i = 3; i < argc; i++)
    {
        arguments.push_back("-I");
        arguments.push_back(argv[i]);
    }

    std::string ImplementationBody;

    std::string MarkedClassesEnumBody;

    std::string IncludedPaths;

    std::vector<string> ClassNames;

    Directory dir(DirectoryName);
    dir.ForEachFile(true, 
    [&](const std::string& filepath) 
        {
            if ((EndsWith(filepath, ".h") || EndsWith(filepath, ".hpp")) && 
                 NeedsReflection(filepath))
            {
                cout << filepath << endl;
                ParseHeaderFile(filepath);
            }
        });

    GenerateCode();
    WriteCode(GeneratedCodePath);

    return 0;
}