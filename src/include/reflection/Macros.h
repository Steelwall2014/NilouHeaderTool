#pragma once

#define NCLASS __attribute__((annotate("reflect-class")))

#define UPROPERTY() __attribute__((annotate("reflect-property")))

#define UFUNCTION() __attribute__((annotate("reflect-method")))

#define GENERATE_BODY() \
    private: \
        template<typename T>  \
        friend class reflection::TClassRegistry; \
        static std::unique_ptr<reflection::UClass> StaticClass_; \
    public: \
        virtual const reflection::UClass *GetClass(); \
        static const reflection::UClass *StaticClass();