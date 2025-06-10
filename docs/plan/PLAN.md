# Plan: Native Addon with Dynamic Library

This plan details the implementation of a new feature allowing the Electron renderer to communicate with a native C++ addon, which in turn will load and interact with an external C-style dynamic library.

## 1. Create the External Dynamic Library

-   **Action:** Create a new C++ file for the dynamic library.
-   **Path:** `src/native/lib/dlib.cpp`
-   **Content:**
    -   Define the `ret_data` struct.
    -   Define the `exchange` function. This function will receive a buffer, create a new buffer containing a message and the received data, and return it in a `ret_data` struct.
    -   Define the `release_data` callback function.
    -   The `exchange` function will be exported with C linkage (`extern "C"`) to prevent name mangling.

## 2. Update the Build System

-   **Action:** Modify `src/native/binding.gyp` to build both the addon and the new dynamic library.
-   **Details:**
    -   A new target named `dlib` will be added to build the dynamic library (`dlib.dylib`, `dlib.so`, or `dlib.dll`). This target will be of type `shared_library`.
    -   The main `addon` target will be updated to ensure the `dlib` is built first. On macOS, we'll add linker settings to the `addon` to know where to find the `dlib`.
-   **Action:** Modify `package.json`.
-   **Details:**
    -   Update the `build:native` script to clean up previous builds to avoid confusion. A new `clean` script will be added and chained. `rm -rf src/native/build src/native/lib/build`

## 3. Modify the C++ Addon

-   **Action:** Update `src/native/addon.cpp` to load the dynamic library and expose the `exchange` functionality.
-   **Details:**
    -   A new N-API function, `ExchangeData`, will be created.
    -   This function will:
        1.  Dynamically load the `dlib` shared library created in step 1 using `dlopen()` (or `LoadLibrary` on Windows). The path will be relative to the addon's location.
        2.  Use `dlsym()` (or `GetProcAddress`) to get a pointer to the `exchange` function.
        3.  Take a JavaScript `Buffer` as input from the renderer.
        4.  Call the `exchange` function, passing the buffer's data and size.
        5.  Receive the `ret_data` struct back from the `exchange` function.
        6.  Create a new JavaScript `Buffer` from the `ret_data`. To manage memory, we will use the `Finalizer` callback of the `Buffer`. The `release` callback from the `dlib` will be passed as the `Finalizer`'s hint, and a generic C++ lambda will be the finalizer, which will, in turn, call the `release` callback.
        7.  Return the new buffer to the renderer.
    -   The `Init` function will be updated to export this new `exchangeData` function alongside the existing `hello` function.

## 4. Update the Renderer

-   **Action:** Modify `src/renderer.js` and `src/index.html` to use the new functionality.
-   **Details:**
    -   **`index.html`:** Add a new button and display area for the `exchange` feature.
    -   **`renderer.js`:**
        1.  Add an event listener to the new button.
        2.  On click, create a sample `Buffer` in JavaScript.
        3.  Call the new `addon.exchangeData()` function with the buffer.
        4.  Display the string from the returned buffer in the new display area.

## 5. Documentation

-   A new implementation log file will be created under `docs/impl/` to document the entire process once the implementation is complete and verified. 