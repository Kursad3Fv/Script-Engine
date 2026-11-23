#pragma once

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/mono-config.h>
#include <mono/metadata/mono-debug.h>

#include <string>
#include <vector>

class ScriptEngine {
public:
    bool Initialize(const std::string monoLibraries, const std::string& dllPath);

    MonoObject* CreateObject(const std::string& namespaceName, const std::string& className);

    void AddInternalCall(const std::string& name, const void* func);

    bool CallMethod(MonoObject* object, const std::string& methodName);
    bool CallMethod(MonoObject* object, const std::string& methodName, void** params, int paramCount);

    void CallStaticMethod(const std::string& namespaceName, const std::string& className, const std::string& methodName);
    void CallStaticMethod(const std::string& namespaceName, const std::string& className, const std::string& methodName, void** params, int paramCount);

    std::vector<std::string> ListPublicFields(MonoObject* object);
    MonoObject* GetFieldValue(MonoObject* object, const std::string& fieldName);
    bool SetFieldValue(MonoObject* object, const std::string& fieldName, void* value);

private:
    MonoDomain* mDomain = nullptr;
    MonoAssembly* mAssembly = nullptr;
    MonoImage* mImage = nullptr;
};
