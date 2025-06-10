# Implementation Log: dylib Extraction and CMake Build

**Date:** 2024-06-12

This document logs the process of refactoring the project to extract the `dlib` dynamic library into a separate module, building it with `CMake`, and linking the main `node-gyp`-based N-API addon against it.

## 1. Summary of Changes

The primary goal was to decouple the dynamic library from the N-API addon's build process, treating it as an external, independently compiled dependency.

1.  **Directory Restructure:** The `dlib.cpp` source file was moved from `src/native/lib` to a new top-level `ext_native` directory.
2.  **CMake for dylib:** A `CMakeLists.txt` file was created in `ext_native` to compile `dlib.cpp` into a shared library (`libdlib.dylib`).
3.  **Updated Addon Build:** The `src/native/binding.gyp` file was heavily modified. The target for building `dlib` was removed. Instead, linker flags (`-L`), library flags (`-ldlib`), and a runtime search path (`-rpath`) were added to the `addon` target to link it against the externally built `dlib`.
4.  **Orchestrated Build Scripts:** The `package.json` scripts were updated to first build the dylib using `CMake` and then build the addon using `node-gyp`.
5.  **Code & Build Fixes:**
    -   The `dlopen` path in `src/native/addon.cpp` was updated to use the correct library name (`libdlib.dylib`).
    -   The `rpath` in `binding.gyp` was corrected after initial runtime errors showed it was pointing to the wrong location. Several attempts were needed to get the relative path correct.

## 2. File Manifest

### Created

-   `ext_native/dlib.cpp`: (Moved from `src/native/lib/dlib.cpp`)
-   `ext_native/CMakeLists.txt`: Build script for the dynamic library.
-   `docs/plan/03-cmake-refactor-plan.md`: The plan for this refactor.
-   `docs/impl/03-cmake-refactor-impl.md`: This implementation log.
-   `.gitignore`: Added to ignore build artifacts.

### Modified

-   `src/native/binding.gyp`: Removed dlib target, added linker settings to link against the external dylib.
-   `package.json`: Updated build scripts to use `cmake` for the dylib and `node-gyp` for the addon.
-   `src/native/addon.cpp`: Updated the `dlopen` call to use the correct library name (`libdlib.dylib`) and an `rpath`-relative path.

### Deleted

-   `src/native/lib`: This directory is no longer needed.

## 3. Final Outcome

After fixing the initial linker errors and a persistent runtime `rpath` issue, the final implementation was successful. The application now builds in two stages (CMake for the library, node-gyp for the addon) and runs correctly, with the addon successfully loading its external dependency at runtime. 