#pragma once

#define NCLASS __attribute__((annotate("reflect-class")))

#define NPROPERTY() __attribute__((annotate("reflect-property")))

#define NFUNCTION() __attribute__((annotate("reflect-method")))

#define GENERATE_BODY() \
    private: \
        template<typename T>  \
        friend class reflection::TClassRegistry; \
        static std::unique_ptr<reflection::NClass> StaticClass_; \
    public: \
        virtual const reflection::NClass *GetClass(); \
        static const reflection::NClass *StaticClass();