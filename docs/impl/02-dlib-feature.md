# Implementation Log: Dynamic Library Addon

**Date:** 2024-06-11

This document logs the implementation of a feature allowing a C++ N-API addon to dynamically load a C-style shared library (`.dylib`, `.so`, `.dll`) and interact with it from the Electron renderer process.

## 1. Summary of Changes

The goal was to have the renderer process call a native addon, which would then delegate the call to a function within an external dynamic library. The core of this work involved:

1.  **Creating a C-style dynamic library:** A new library (`dlib`) was created with an `exchange` function to process data.
2.  **Modifying the build system:** `binding.gyp` was updated to build both the addon and the new dynamic library.
3.  **Updating the C++ Addon:** The addon was modified to dynamically load, interact with, and unload the `dlib`.
4.  **Refactoring for Control:** The addon's functions were refactored into `loadDyLib`, `exchangeData`, and `unloadDyLib` for granular control from JavaScript.
5.  **Updating the UI:** The HTML and renderer JavaScript were updated to control and test the new functionality.
6.  **Fixing a Buffer Security Issue:** Addressed an "External buffers are not allowed" error by changing the memory handling strategy from wrapping external pointers to copying data into Node.js-managed buffers.

## 2. File Manifest

### Created

-   `src/native/lib/dlib.cpp`: The source code for the new dynamic library.
-   `docs/plan/PLAN.md`: The initial implementation plan document.
-   `docs/impl/02-dlib-feature.md`: This implementation log.

### Modified

-   `src/native/binding.gyp`: Added a new target for the `dlib` and configured the main addon to link against it.
-   `src/native/addon.cpp`: Initially modified to load the dlib and call the `exchange` function. Later refactored to split this logic into three separate, exported functions (`loadDyLib`, `exchangeData`, `unloadDyLib`) and to fix a buffer-related security error.
-   `package.json`: Added a `clean` script to the build process.
-   `src/index.html`: Added new buttons and status displays for loading, exchanging, and unloading.
-   `src/renderer.js`: Updated to handle the new UI elements and call the refactored native addon functions in the correct sequence.
-   `docs/impl/Project-Chronology.md`: (This would be updated in a real-world scenario to include this feature, but is omitted here for brevity).

## 3. Final Implementation Details

-   The addon now maintains a static handle to the loaded dynamic library.
-   The renderer can explicitly call `loadDyLib` to open the library, `exchangeData` to perform operations, and `unloadDyLib` to release the library.
-   Memory from the dynamic library is safely handled by copying it into V8-managed `Napi::Buffer`s, which prevents security errors related to external memory.

The feature was successfully implemented and tested. 