#include <cstddef>
#include <cstring>
#include <cstdlib>
#include <string>

// Define the function pointer type for the release callback
typedef void (*release_callback_t)(void* data);



// Use extern "C" to prevent C++ name mangling for the exported function
extern "C" {
    // New function to modify data in-place
    size_t exchange_inplace(void* in_out_data, size_t buffer_size) {
        std::string response_str = "Hello from dlib (in-place)!";

        // Check if the buffer is large enough
        if (buffer_size < response_str.length() + 1) {
            return 0; // Not enough space
        }

        // Copy the data into the provided buffer
        memcpy(in_out_data, response_str.c_str(), response_str.length() + 1);

        // Return the number of bytes written
        return response_str.length() + 1;
    }
} 