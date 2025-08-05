#include <cstddef>
#include <cstring>
#include <cstdlib>
#include <string>

// Use extern "C" to prevent C++ name mangling for the exported function
extern "C" {
    // New function to modify data in-place with can_override flag
    size_t exchange_inplace(void* in_out_data, size_t buffer_size, size_t data_size, bool can_override) {

        //get data
        //....



        if (!can_override) {
            // Read-only mode - don't modify buffer, just return what would be written
            // Return the size that would be needed if modification were allowed
            return 0;
        }


        std::string response_str = "Hello from dlib (in-place)!";
        
        // Check if the buffer is large enough
        if (buffer_size < response_str.length() + 1) {
            return 0; // Not enough space
        }

        // Copy the data into the provided buffer (existing logic)
        memcpy(in_out_data, response_str.c_str(), response_str.length() + 1);

        // Return the number of bytes written
        return response_str.length() + 1;
    }
} 