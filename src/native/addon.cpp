#include <napi.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include <iostream>
#include <string>

// Define the function pointer type for the release callback
typedef void (*release_callback_t)(void* data);

// Define the struct to be returned by the exchange function
struct ret_data {
    void* data;
    size_t data_size;
    release_callback_t release;
};

// Define the function signature for the 'exchange' function from the dlib
typedef ret_data (*exchange_t)(void*, size_t);

// Global handle for the dynamic library
#ifdef _WIN32
static HINSTANCE dylib_handle = nullptr;
#else
static void* dylib_handle = nullptr;
#endif

namespace demo {

Napi::String Method(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  return Napi::String::New(env, "Hello from C++!");
}

Napi::Value LoadDyLib(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (dylib_handle) {
        Napi::Error::New(env, "Library already loaded").ThrowAsJavaScriptException();
        return env.Null();
    }

    // Check if path argument is provided
    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "String path expected as first argument").ThrowAsJavaScriptException();
        return env.Null();
    }

    // Get the path from the argument
    std::string dlib_path = info[0].As<Napi::String>().Utf8Value();

#ifdef _WIN32
    dylib_handle = LoadLibrary(dlib_path.c_str());
#else
    dylib_handle = dlopen(dlib_path.c_str(), RTLD_LAZY);
#endif

    if (!dylib_handle) {
#ifdef _WIN32
        Napi::Error::New(env, "Cannot open library at '" + dlib_path + "': " + std::to_string(GetLastError())).ThrowAsJavaScriptException();
#else
        Napi::Error::New(env, std::string("Cannot open library at '") + dlib_path + "': " + dlerror()).ThrowAsJavaScriptException();
#endif
    }
    return env.Undefined();
}

Napi::Value UnloadDyLib(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (!dylib_handle) {
        Napi::Error::New(env, "Library not loaded").ThrowAsJavaScriptException();
        return env.Null();
    }
#ifdef _WIN32
    FreeLibrary(dylib_handle);
#else
    dlclose(dylib_handle);
#endif
    dylib_handle = nullptr;
    return env.Undefined();
}

// The new function that will call the dynamic library
Napi::Value ExchangeData(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (!dylib_handle) {
        Napi::Error::New(env, "Library not loaded. Call LoadDyLib first.").ThrowAsJavaScriptException();
        return env.Null();
    }

    // 1. Check input arguments
    if (info.Length() < 1 || !info[0].IsBuffer()) {
        Napi::TypeError::New(env, "Buffer expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    Napi::Buffer<uint8_t> js_buffer = info[0].As<Napi::Buffer<uint8_t>>();

    // 2. Get symbol for the 'exchange' function
#ifdef _WIN32
    exchange_t exchange_func = (exchange_t)GetProcAddress(dylib_handle, "exchange");
#else
    exchange_t exchange_func = (exchange_t)dlsym(dylib_handle, "exchange");
#endif

    if (!exchange_func) {
#ifdef _WIN32
        Napi::Error::New(env, "Cannot load symbol 'exchange': " + std::to_string(GetLastError())).ThrowAsJavaScriptException();
#else
        Napi::Error::New(env, std::string("Cannot load symbol 'exchange': ") + dlerror()).ThrowAsJavaScriptException();
#endif
        return env.Null();
    }

    // 3. Call the 'exchange' function
    ret_data result = exchange_func(js_buffer.Data(), js_buffer.Length());

    // 4. Create a result buffer by copying the data, then release the original.
    if (result.data == nullptr) {
        Napi::Error::New(env, "Received null data from dynamic library").ThrowAsJavaScriptException();
        return env.Null();
    }

    // Create a new buffer by copying the data from the dlib.
    Napi::Buffer<uint8_t> result_buffer = Napi::Buffer<uint8_t>::Copy(env, static_cast<uint8_t*>(result.data), result.data_size);

    // Since the data has been copied into a Node-managed buffer,
    // we can and should release the original memory immediately.
    if (result.release) {
        result.release(result.data);
    }

    return result_buffer;
}

// The new function that will call the dynamic library to modify data in-place
Napi::Value ExchangeDataInPlace(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (!dylib_handle) {
        Napi::Error::New(env, "Library not loaded. Call LoadDyLib first.").ThrowAsJavaScriptException();
        return env.Null();
    }

    // 1. Check input arguments
    if (info.Length() < 1 || !info[0].IsBuffer()) {
        Napi::TypeError::New(env, "Buffer expected").ThrowAsJavaScriptException();
        return env.Null();
    }
    Napi::Buffer<uint8_t> js_buffer = info[0].As<Napi::Buffer<uint8_t>>();

    // 2. Get symbol for the 'exchange_inplace' function
    typedef size_t (*exchange_inplace_t)(void*, size_t);
#ifdef _WIN32
    exchange_inplace_t exchange_func = (exchange_inplace_t)GetProcAddress(dylib_handle, "exchange_inplace");
#else
    exchange_inplace_t exchange_func = (exchange_inplace_t)dlsym(dylib_handle, "exchange_inplace");
#endif

    if (!exchange_func) {
#ifdef _WIN32
        Napi::Error::New(env, "Cannot load symbol 'exchange_inplace': " + std::to_string(GetLastError())).ThrowAsJavaScriptException();
#else
        Napi::Error::New(env, std::string("Cannot load symbol 'exchange_inplace': ") + dlerror()).ThrowAsJavaScriptException();
#endif
        return env.Null();
    }

    // 3. Call the 'exchange_inplace' function
    size_t bytes_written = exchange_func(js_buffer.Data(), js_buffer.Length());

    // 4. Return the number of bytes written
    return Napi::Number::New(env, bytes_written);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "hello"),
              Napi::Function::New(env, Method));
  exports.Set(Napi::String::New(env, "loadDyLib"),
              Napi::Function::New(env, LoadDyLib));
  exports.Set(Napi::String::New(env, "exchangeData"),
              Napi::Function::New(env, ExchangeData));
  exports.Set(Napi::String::New(env, "exchangeDataInPlace"),
              Napi::Function::New(env, ExchangeDataInPlace));
  exports.Set(Napi::String::New(env, "unloadDyLib"),
              Napi::Function::New(env, UnloadDyLib));
  return exports;
}

NODE_API_MODULE(addon, Init)

} // namespace demo 