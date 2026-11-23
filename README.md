# C# Script Engine

A lightweight C++ scripting engine built on top of the **Mono Runtime**, allowing C++ applications and game engines to execute C# scripts, create managed objects, invoke methods, access fields, and expose native C++ functions to managed code through Internal Calls.

The project is designed to provide a simple bridge between C++ and C#, making it suitable for game engines, application frameworks, and custom scripting systems.

## Features

- **Mono Runtime Integration:** Initialize and host the Mono runtime from C++.
- **Assembly Loading:** Load compiled C# DLL assemblies.
- **Object Creation:** Instantiate C# classes from C++.
- **Instance Method Invocation:** Execute methods on managed objects.
- **Static Method Invocation:** Execute static C# methods without creating an object.
- **Field Access:** Enumerate, read, and modify fields of managed objects.
- **Internal Calls:** Register native C++ functions and call them directly from C#.

## Requirements

- C++20 compatible compiler.
- CMake 3.21 or newer.
- Mono development package.
- A C# compiler compatible with the installed Mono runtime.

## Project Structure

```text
ScriptEngine/
├── CMakeLists.txt
├── Code/
│   ├── ScriptEngine.h
│   └── ScriptEngine.cpp
└── README.md
```

Mono is not included in this repository. Developers must install Mono separately.

## Building

### 1. Install Mono

Install the Mono development package on your system.

The installation must provide:

- Mono development headers.
- Mono import libraries.
- Mono runtime DLLs and dependencies.
- Mono framework class libraries.

### 2. Configure CMake

The Mono installation directory is provided through the `MONO_ROOT` CMake variable.

```bash
cmake -S . -B build -DMONO_ROOT="C:/Program Files/Mono"
```

Then build the project:

```bash
cmake --build build --config Release
```

CMake uses the installed Mono headers and libraries to compile and link the static ScriptEngine library.

### 3. Mono Runtime Files

The Mono runtime requires its framework assemblies to be available when the application starts.

Mono provides different framework profiles under:

```text
MONO_ROOT/
└── lib/
    └── mono/
        ├── 2.0/
        ├── 4.0/
        └── 4.5/
```

The appropriate framework directory must match the target framework used by your C# project.

For example, if your C# project targets .NET Framework 4.5, use the corresponding `4.5` framework assemblies.

Copy the appropriate framework directory into your application's output directory, preserving the directory structure expected by your Mono runtime configuration.

The Mono runtime DLL and its required dependencies must also be accessible at runtime, either through the system PATH or the application's output directory.

**Important:** The directory passed to `ScriptEngine::Initialize()` must match the assembly directory expected by your Mono configuration.

## C# Scripting

Before using the ScriptEngine, create a C# project and compile it into a DLL.

For example:

```text
GameScripts/
├── GameScripts.csproj
├── Player.cs
└── InternalCalls.cs
```

Compile the project:

```bash
msbuild GameScripts.csproj
```

Alternatively, use your preferred C# compiler or build system.

The resulting DLL will be loaded by the ScriptEngine.

### Example C# Script

```csharp
namespace GameScripts
{
    public class Player
    {
        public int Health = 100;
        public float Speed = 5.0f;
        public string Name = "Player";

        public Player()
        {
            System.Console.WriteLine("Player created!");
        }

        public void Start()
        {
            System.Console.WriteLine("Player started!");
        }

        public void Update()
        {
            System.Console.WriteLine("Player updated!");
        }

        public static void Initialize()
        {
            System.Console.WriteLine("Static initialization!");
        }
    }
}
```

## Usage

Include the ScriptEngine header:

```cpp
#include "ScriptEngine.h"
```

### 1. Initialize the Script Engine

Create a ScriptEngine instance and initialize the Mono runtime with the framework assembly directory and the compiled C# DLL path.

```cpp
ScriptEngine engine;

if (!engine.Initialize(
    "../Bin/Mono/lib/mono/4.5",
    "../Bin/GameScripts.dll"))
{
    return -1;
}
```

The first argument specifies the Mono library directory used by the current runtime configuration. The second argument specifies the compiled C# assembly.

The framework profile in this example is illustrative; use the profile matching your C# project and Mono installation.

### 2. Create a C# Object

Use `CreateObject()` to instantiate a C# class.

```cpp
MonoObject* player = engine.CreateObject(
    "GameScripts",
    "Player"
);
```

The namespace and class name must match the definitions in your C# assembly.

The parameterless constructor is executed when the object is created.

### 3. Call an Instance Method

Use `CallMethod()` to invoke an instance method.

```cpp
engine.CallMethod(player, "Start");
```

Methods with parameters can also be invoked by passing a parameter array and the parameter count.

```cpp
int health = 75;

void* params[] = {
    &health
};

engine.CallMethod(
    player,
    "SetHealth",
    params,
    1
);
```

### 4. Call a Static Method

Static methods can be invoked directly using the namespace, class name, and method name.

```cpp
engine.CallStaticMethod(
    "GameScripts",
    "Player",
    "Initialize"
);
```

Static methods with parameters are also supported.

```cpp
int value = 100;

void* params[] = {
    &value
};

engine.CallStaticMethod(
    "GameScripts",
    "Player",
    "SetDefaultHealth",
    params,
    1
);
```

### 5. List Public Fields

Use `ListPublicFields()` to retrieve field names from a managed object.

```cpp
std::vector<std::string> fields =
    engine.ListPublicFields(player);

for (const std::string& field : fields)
{
    std::cout << field << std::endl;
}
```

This allows applications such as game editors and inspectors to enumerate managed object fields.

### 6. Get Field Values

Use `GetFieldValue()` to retrieve a field from a managed object.

```cpp
MonoObject* healthObject =
    engine.GetFieldValue(player, "Health");

if (healthObject)
{
    int health = *(int*)mono_object_unbox(healthObject);

    std::cout << "Health: " << health << std::endl;
}
```

Value types such as `int`, `float`, and `bool` are boxed when returned as `MonoObject*`. Use `mono_object_unbox()` to access their underlying values.

Reference types such as strings require their corresponding Mono APIs.

### 7. Set Field Values

Use `SetFieldValue()` to modify an existing field.

```cpp
int newHealth = 50;

engine.SetFieldValue(
    player,
    "Health",
    &newHealth
);
```

The value pointer must represent the actual native storage of the corresponding managed field type.

For example, an `int` field expects an `int*`, while a `float` field expects a `float*`.

## Internal Calls

Internal Calls allow C# scripts to invoke native C++ functions directly through the Mono runtime.

This is particularly useful for exposing engine functionality such as input, rendering, audio, physics, and entity operations to C# scripts.

### 1. Define an Internal Call in C\#

Create a static external method and mark it with `MethodImplOptions.InternalCall`.

```csharp
using System.Runtime.CompilerServices;

namespace GameScripts
{
    public static class InternalCalls
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern string GetMessage();
    }
}
```

### 2. Implement the Native C++ Function

```cpp
#include <mono/jit/jit.h>

MonoString* GetMessageInternal()
{
    return mono_string_new(
        mono_domain_get(),
        "Hello from C++!"
    );
}
```

### 3. Register the Internal Call

Register the native function before invoking the corresponding C# method.

```cpp
engine.AddInternalCall(
    "GameScripts.InternalCalls::GetMessage",
    reinterpret_cast<const void*>(GetMessageInternal)
);
```

The registration name must exactly match the C# namespace, class, and method name.

### 4. Call the Internal Call from C\#

```csharp
namespace GameScripts
{
    public class Player
    {
        public void Start()
        {
            string message = InternalCalls.GetMessage();

            System.Console.WriteLine(message);
        }
    }
}
```

The C# method will execute the registered native C++ function through the Mono runtime.

## Complete Example

```cpp
#include "ScriptEngine.h"

#include <iostream>

MonoString* GetMessageInternal()
{
    return mono_string_new(
        mono_domain_get(),
        "Hello from the native engine!"
    );
}

int main()
{
    ScriptEngine engine;

    if (!engine.Initialize(
        "../Bin/Mono/lib/mono/4.5",
        "../Bin/GameScripts.dll"))
    {
        return -1;
    }

    engine.AddInternalCall(
        "GameScripts.InternalCalls::GetMessage",
        reinterpret_cast<const void*>(GetMessageInternal)
    );

    MonoObject* player = engine.CreateObject(
        "GameScripts",
        "Player"
    );

    if (!player)
        return -1;

    engine.CallMethod(player, "Start");

    engine.CallStaticMethod(
        "GameScripts",
        "Player",
        "Initialize"
    );

    return 0;
}
```

## API Reference

| Method               | Description                                          |
| -------------------- | ---------------------------------------------------- |
| `Initialize()`       | Initializes Mono and loads a C# assembly.            |
| `CreateObject()`     | Creates an instance of a managed C# class.           |
| `AddInternalCall()`  | Registers a native C++ function as an Internal Call. |
| `CallMethod()`       | Invokes an instance method.                          |
| `CallStaticMethod()` | Invokes a static method.                             |
| `ListPublicFields()` | Retrieves field names from a managed object.         |
| `GetFieldValue()`    | Retrieves a managed field value.                     |
| `SetFieldValue()`    | Modifies a managed field value.                      |

## Notes

- C# scripts must be compiled into a DLL before they can be loaded.
- The C# target framework must be compatible with the installed Mono runtime.
- Mono framework assemblies must be available at runtime.
- Internal Calls must be registered before their corresponding C# methods are invoked.
- Native and managed method signatures must match.
- Managed objects are subject to Mono garbage collection. Long-lived native references should be protected using appropriate GC handles.
- The current API supports method lookup by name and parameter count; it does not perform overload resolution by parameter types.
