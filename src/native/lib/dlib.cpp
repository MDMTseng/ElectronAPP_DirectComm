#include <cstddef>
#include <cstring>
#include <cstdlib>
#include <string>

// Define the function pointer type for the release callback
typedef void (*release_callback_t)(void* data);

// Define the struct to be returned by the exchange function
struct ret_data {
    void* data;
    size_t data_size;
    release_callback_t release;
};

// Define the release function
void release_data(void* data) {
    if (data) {
        free(data);
    }
}

// Use extern "C" to prevent C++ name mangling for the exported function
extern "C" {
    // The main data exchange function
    ret_data exchange(void* data_from_renderer, size_t data_size) {
        // Create a response string
        std::string response_str = "Hello from the dynamic library! You sent: ";
        response_str.append(static_cast<char*>(data_from_renderer), data_size);

        // Allocate memory for the response buffer.
        // The memory allocated here will be freed by the N-API addon
        // using the 'release_data' function.
        void* response_data = malloc(response_str.length() + 1);
        if (response_data) {
            memcpy(response_data, response_str.c_str(), response_str.length() + 1);
        }

        // Populate the return struct
        ret_data ret = {
            response_data,
            response_data ? response_str.length() + 1 : 0,
            release_data
        };

        return ret;
    }
} 