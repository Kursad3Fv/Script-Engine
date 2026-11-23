#include "ScriptEngine.h"

bool ScriptEngine::Initialize(const std::string monoLibraries, const std::string& dllPath) {

    mono_set_dirs(
        monoLibraries.c_str(),  
        monoLibraries.c_str()
    );

    mDomain = mono_jit_init("ScriptDomain");
    if (!mDomain) {
        return false;
    }

    mAssembly = mono_domain_assembly_open(mDomain, dllPath.c_str());
    if (!mAssembly) {
        return false;
    }

    mImage = mono_assembly_get_image(mAssembly);
    return true;
}

MonoObject* ScriptEngine::CreateObject(const std::string& namespaceName, const std::string& className) {
    MonoClass* klass = mono_class_from_name(mImage, namespaceName.c_str(), className.c_str());
    if (!klass) {
        return nullptr;
    }

    MonoObject* obj = mono_object_new(mDomain, klass);
    mono_runtime_object_init(obj); 
    return obj;
}

void ScriptEngine::AddInternalCall(const std::string& name, const void* func)
{
    mono_add_internal_call(name.c_str(), func);
}

bool ScriptEngine::CallMethod(MonoObject* object, const std::string& methodName) {
    MonoClass* klass = mono_object_get_class(object);
    MonoMethod* method = mono_class_get_method_from_name(klass, methodName.c_str(), 0);
    if (!method) {
        return false;
    }

    mono_runtime_invoke(method, object, nullptr, nullptr);
    return true;
}

bool ScriptEngine::CallMethod(MonoObject* object, const std::string& methodName, void** params, int paramCount)
{
    MonoClass* klass = mono_object_get_class(object);
    MonoMethod* method = mono_class_get_method_from_name(klass, methodName.c_str(), paramCount);
    if (!method) {
        return false;
    }

    mono_runtime_invoke(method, object, params, nullptr);
    return true;
}

void ScriptEngine::CallStaticMethod(const std::string& namespaceName, const std::string& className, const std::string& methodName)
{
    MonoClass* klass = mono_class_from_name(mImage, namespaceName.c_str(), className.c_str());
    MonoMethod* method = mono_class_get_method_from_name(klass, methodName.c_str(), 0);
    mono_runtime_invoke(method, nullptr, nullptr, nullptr);
}


void ScriptEngine::CallStaticMethod(const std::string& namespaceName, const std::string& className, const std::string& methodName, void** params, int paramCount)
{
    MonoClass* klass = mono_class_from_name(mImage, namespaceName.c_str(), className.c_str());
    MonoMethod* method = mono_class_get_method_from_name(klass, methodName.c_str(), paramCount);
    mono_runtime_invoke(method, nullptr, params, nullptr);
}

std::vector<std::string> ScriptEngine::ListPublicFields(MonoObject* object) {
    std::vector<std::string> fieldNames;

    MonoClass* klass = mono_object_get_class(object);
    void* iter = nullptr;

    while (MonoClassField* field = mono_class_get_fields(klass, &iter)) {
        uint32_t flags = mono_field_get_flags(field);

        if (flags & 0x0006) {
            const char* name = mono_field_get_name(field);
            if (name) {
                fieldNames.emplace_back(name);
            }
        }
    }

    return fieldNames;
}

MonoObject* ScriptEngine::GetFieldValue(MonoObject* object, const std::string& fieldName)
{
    MonoClass* klass = mono_object_get_class(object);
    MonoClassField* field = mono_class_get_field_from_name(klass, fieldName.c_str());
    if (!field) {
        return nullptr;
    }

    MonoObject* value = mono_field_get_value_object(mDomain, field, object);
    return value;
}

bool ScriptEngine::SetFieldValue(MonoObject* object, const std::string& fieldName, void* value)
{
    MonoClass* klass = mono_object_get_class(object);
    MonoClassField* field = mono_class_get_field_from_name(klass, fieldName.c_str());
    if (!field) {
        return false;
    }

    mono_field_set_value(object, field, value);
    return true;
}
