#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <filesystem>
#include <clang-c/Index.h>

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

struct ParsedClass
{
    string FileName;
    string Name;
    set<string> BaseClasses;
    set<string> DerivedClasses;
    map<string, string> Fields;
    set<string> Methods;
    vector<vector<string>> Constructors;
    string GeneratedFileCode;
};
map<string, ParsedClass> NClasses;
string current_filepath;

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
                if (s == "reflect-class") 
                {
                    vector<CXCursor>* reflection_classes = reinterpret_cast<vector<CXCursor>*>(client_data);
                    string class_name = GetCursorSpelling(parent);
                    if (!NClasses.contains(class_name))
                    {
                        reflection_classes->push_back(parent);
                        NClasses[class_name] = ParsedClass();
                        NClasses[class_name].Name = class_name;
                        NClasses[class_name].FileName = current_filepath;
                        return CXChildVisit_Continue;
                    }
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
                string cursor_spelling = GetCursorSpelling(c);
                string cursor_kind = GetCursorKindSpelling(c);
                auto cursor_kind_raw = clang_getCursorKind(c);

                if (cursor_kind_raw == CXCursor_Constructor)
                {
                    string class_name = cursor_spelling;
                    string method_name = GetCursorSpelling(parent);
                    string method_args = GetCursorTypeSpelling(parent);
                    if (NClasses.contains(class_name))
                    {
                        vector<string>& args = NClasses[class_name].Constructors.emplace_back();
                        int args_num = clang_Cursor_getNumArguments(c);
                        for (int i = 0; i < args_num; i++)
                        {
                            auto type = GetCursorTypeSpelling(clang_Cursor_getArgument(c, i));
                            args.push_back(type);
                        }
                    }
                }
                else if (cursor_kind_raw == CXCursor_AnnotateAttr) 
                {
                    if (cursor_spelling == "reflect-property") 
                    {
                        CXCursor class_cursor = clang_getCursorSemanticParent(parent);
                        string class_name = GetCursorSpelling(class_cursor);
                        string field_name = GetCursorSpelling(parent);
                        string field_type = GetCursorTypeSpelling(parent);
                        if (NClasses.contains(class_name))
                        {
                            auto& Fields = NClasses[class_name].Fields;
                            Fields[field_name] = field_type;
                        }
                    }
                    else if (cursor_spelling == "reflect-method")
                    {
                        CXCursor class_cursor = clang_getCursorSemanticParent(parent);
                        string class_name = GetCursorSpelling(class_cursor);
                        string method_name = GetCursorSpelling(parent);
                        string method_args = GetCursorTypeSpelling(parent);
                        if (NClasses.contains(class_name))
                        {
                            auto& Methods = NClasses[class_name].Methods;
                            Methods.insert(method_name);
                        }
                    }
                    
                }
                else if (cursor_kind_raw == CXCursor_CXXBaseSpecifier) 
                {
                    vector<string> tokens = Split(cursor_spelling, ':');
                    string base_class = tokens[tokens.size()-1];
                    string derived_class = GetCursorSpelling(parent);
                    NClasses[base_class].DerivedClasses.insert(derived_class);
                    NClasses[derived_class].BaseClasses.insert(base_class);
                }
                
                return CXChildVisit_Recurse;
            },
            nullptr);
    }

    
    clang_disposeTranslationUnit(unit);
    clang_disposeIndex(index);
    
}

void GenerateCode()
{
    static const string& indent = "\t\t";
    for (auto& [ClassName, NClass] : NClasses)
    {
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
        for (auto& ParentClass : NClass.BaseClasses)
        {
            ClassHierarchyBody += indent+format("Mngr.AddBases<{}, {}>();\n", 
                ClassName, ParentClass);
        }
        // for (auto& DerivedClass : NClass.DerivedClasses)
        // {
        //     ClassHierarchyBody += indent+format(".AddDerivedClass(\"{0}\")\n", 
        //         DerivedClass);
        // }
        NClass.GeneratedFileCode = format(
R"(#include "{0}"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> {1}::StaticClass_ = nullptr;
const NClass *{1}::GetClass() const 
{{ 
    return {1}::StaticClass(); 
}}
const NClass *{1}::StaticClass()
{{
    return {1}::StaticClass_.get();
}}

template<>
struct TClassRegistry<{1}>
{{
    TClassRegistry(const std::string& InName)
    {{
        {1}::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<{1}>();
{2}{3}{4}{5};
        {1}::StaticClass_->Type = Type_of<{1}>;
        {1}::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<{1}>);
    }}

    static TClassRegistry<{1}> Dummy;
}};
TClassRegistry<{1}> Dummy = TClassRegistry<{1}>("{1}");


)", NClass.FileName, ClassName, CtorBody, FieldsBody, MethodsBody, ClassHierarchyBody);
    }
}

void WriteCode(string GeneratedCodePath)
{
    for (auto& [ClassName, NClass] : NClasses)
    {
        ofstream out_stream(GeneratedCodePath + "/" + ClassName + ".generated.cpp", ios::out);
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