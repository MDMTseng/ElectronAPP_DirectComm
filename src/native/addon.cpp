#include <napi.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include <iostream>
#include <string>

namespace demo {

// Global handle for the dynamic library
#ifdef _WIN32
static HINSTANCE dylib_handle = nullptr;
#define LOAD_LIB(path) LoadLibrary(path)
#define GET_SYMBOL(handle, name) GetProcAddress(handle, name)
#define CLOSE_LIB(handle) FreeLibrary(handle)
#define GET_ERROR() std::to_string(GetLastError())
#else
static void* dylib_handle = nullptr;
#define LOAD_LIB(path) dlopen(path, RTLD_LAZY)
#define GET_SYMBOL(handle, name) dlsym(handle, name)
#define CLOSE_LIB(handle) dlclose(handle)
#define GET_ERROR() std::string(dlerror())
#endif

// Helper function for throwing errors
void ThrowError(Napi::Env env, const std::string& message) {
    Napi::Error::New(env, message).ThrowAsJavaScriptException();
}

Napi::Value LoadDyLib(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (dylib_handle) {
        ThrowError(env, "Library already loaded");
        return env.Null();
    }

    if (info.Length() < 1 || !info[0].IsString()) {
        ThrowError(env, "String path expected as first argument");
        return env.Null();
    }

    std::string dlib_path = info[0].As<Napi::String>().Utf8Value();
    
    // Set PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION=cpp before loading to avoid descriptor conflicts
#ifdef _WIN32
    _putenv("PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION=cpp");
#else
    setenv("PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION", "cpp", 1);
#endif

    dylib_handle = LOAD_LIB(dlib_path.c_str());
    std::cout << "Loaded library" << std::endl;
    if (!dylib_handle) {
        ThrowError(env, "Cannot open library at '" + dlib_path + "': " + GET_ERROR());
        return env.Null();
    }
    return env.Undefined();
}

Napi::Value UnloadDyLib(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (!dylib_handle) {
        ThrowError(env, "Library not loaded");
        return env.Null();
    }
    
    // Unload the library
    if (CLOSE_LIB(dylib_handle) != 0) {
        std::string error = GET_ERROR();
        dylib_handle = nullptr;
        ThrowError(env, "Failed to unload library: " + error);
        return env.Null();
    }
    
    std::cout << "Unloaded library" << std::endl;
    dylib_handle = nullptr;
    
    // Clear Protocol Buffers environment variable
#ifdef _WIN32
    _putenv("PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION=");
#else
    unsetenv("PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION");
#endif
    
    return env.Undefined();
}

Napi::Value ExchangeDataInPlace(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (!dylib_handle) {
        ThrowError(env, "Library not loaded. Call LoadDyLib first.");
        return env.Null();
    }

    if (info.Length() != 3) {
        ThrowError(env, "Expected 3 parameters: buffer, data_size, can_override");
        return env.Null();
    }

    if (!info[0].IsBuffer()) {
        ThrowError(env, "First parameter arraybuffer must be a buffer");
        return env.Null();
    }

    if (!info[1].IsNumber()) {
        ThrowError(env, "Second parameter data_size must be a number");
        return env.Null();
    }

    if (!info[2].IsBoolean()) {
        ThrowError(env, "Third parameter can_override must be a boolean");
        return env.Null();
    }

    // Handle optional can_override parameter
    size_t data_size = info[1].As<Napi::Number>().Uint32Value(); // data size
    bool can_override = info[2].As<Napi::Boolean>().Value();

    Napi::Buffer<uint8_t> js_buffer = info[0].As<Napi::Buffer<uint8_t>>();
    
    typedef size_t (*exchange_inplace_t)(void*, size_t, size_t, bool);
    auto exchange_func = (exchange_inplace_t)GET_SYMBOL(dylib_handle, "exchange_inplace");

    if (!exchange_func) {
        ThrowError(env, "Cannot load symbol 'exchange_inplace': " + GET_ERROR());
        return env.Null();
    }

    size_t bytes_written = exchange_func(js_buffer.Data(), js_buffer.Length(),data_size, can_override);//buffer_ptr, buffer_size, data_size, can_override
    return Napi::Number::New(env, bytes_written);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("loadDyLib", Napi::Function::New(env, LoadDyLib));
    exports.Set("exchangeDataInPlace", Napi::Function::New(env, ExchangeDataInPlace));
    exports.Set("unloadDyLib", Napi::Function::New(env, UnloadDyLib));
    return exports;
}

NODE_API_MODULE(addon, Init)

} // namespace demo 