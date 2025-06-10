# Project Chronology: From IPC to Direct Native Addon

This document provides a complete history of the feature implementation for direct renderer-to-native communication in Electron. It consolidates all previous plan and implementation log files.

## 1. Initial Goal: Secure Renderer-to-Main Communication

The initial request was to create a feature where the Electron renderer process could call a function and get data from the main process.

### Plan A: The Secure Approach (Using Preload Script)

The first plan followed Electron's best practices for security and process isolation.

- **Method:** Use a `preload.js` script.
- **Security:** The `contextBridge` API would expose a safe, limited API to the renderer, and `nodeIntegration` would be disabled.
- **Communication:** `ipcMain.handle` in the main process would respond to `ipcRenderer.invoke` from the preload script.
- **User Feedback:** The user rejected this plan, requesting to bypass the `preload.js` script entirely for a more direct connection.

---

## 2. Evolution: Insecure, Direct Renderer-to-Main IPC

Based on user feedback, the plan was revised to allow direct communication, knowingly sacrificing security for this test case.

### Plan B: The Insecure IPC Approach

- **Method:** Remove the `preload.js` script.
- **Security:** `nodeIntegration` was set to `true` and `contextIsolation` was set to `false` in the `BrowserWindow`'s `webPreferences`. A warning about the security risks was included in the plan.
- **Communication:** `ipcMain.on` in the main process would listen for `ipcRenderer.sendSync` from the renderer process.
- **User Feedback:** The user approved this plan. After implementation, they clarified that they wanted to call "native code," not just a function in `main.js`.

---

## 3. The Core Challenge: Calling Native C++ Code

The requirement evolved to calling actual native C++ code from the renderer.

### Plan C: The Native Addon Approach (via Main Process)

This plan introduced a native C++ addon, but still used the main process as an intermediary.

- **Method:** Create a C++ addon using N-API and build it with `node-gyp`.
- **Communication:** The renderer would send an IPC message to the main process, which would then call the native addon function and return the result.
- **User Feedback:** The user rejected this plan, requesting that the renderer access the native addon *directly*, with no middle layer.

---

## 4. Final Implementation: Direct Renderer-to-Native-Addon

This final plan achieved the user's ultimate goal.

### Plan D: The Direct Native Addon Approach

- **Method:** Build a C++ N-API addon. The renderer process would `require()` the compiled `.node` file directly.
- **Security:** This still required the insecure `nodeIntegration: true` setting.
- **Communication:** The main process's only role was to create the window. The renderer's JavaScript directly invoked the exported C++ function from the loaded addon.
- **Implementation Details:**
    1.  **Project Setup:** Initialized `package.json`, installed `electron`, `node-gyp`, and `node-addon-api`.
    2.  **Native Code:** Created `src/native/addon.cpp` with a simple C++ function returning a string, and a `binding.gyp` to configure the build.
    3.  **Build Script:** Added a `build:native` script to `package.json`.
    4.  **Renderer Logic:** Modified `src/renderer.js` to `require()` the native module and call its `hello()` function on a button click.
    5.  **Main Process:** Simplified `src/main.js` by removing all IPC logic.
    6.  **Debugging:** Corrected a file path issue in `renderer.js` that initially prevented the addon from being found.
- **Outcome:** The final implementation was successful and met all user requirements. The application correctly calls C++ code directly from the renderer process. 