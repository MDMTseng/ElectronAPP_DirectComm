# Architectural Restructuring: BIOS Configuration System Implementation

**Date**: December 2024  
**Type**: Major Architectural Refactoring  
**Status**: Implemented  

## Overview

This implementation represents a major architectural restructuring of the Electron application, introducing a BIOS-style configuration system that allows users to specify where application resources are located. The primary purpose is to enable flexible resource loading - either from a development server or from static files in production, giving users control over resource location and loading strategy.

## Key Changes Implemented

### 1. Project Structure Reorganization

**Previous Structure:**
```
ext_native/
├── CMakeLists.txt
└── dlib.cpp
src/
├── index.html
├── renderer.js
└── main.js
```

**New Structure:**
```
EXT_APP/
├── backend/
│   ├── CMakeLists.txt
│   └── dlib.cpp
├── frontend/
│   ├── index.html
│   └── renderer.js
└── build.sh
src/
├── bios.html
└── main.js
```

**Rationale**: This reorganization separates the external application components (EXT_APP) from the core Electron launcher (src), providing clearer separation of concerns and better maintainability.

### 2. BIOS Configuration System

**New File**: `src/bios.html`
- Implements a configuration interface for directing the application to resource locations
- Supports development mode (loads from dev server at configurable port) and production mode (loads from static files at specified path)
- Includes input validation and local storage for settings persistence
- Provides a clean, user-friendly interface for resource location configuration

**Key Features**:
- Radio button selection between Development (dev server) and Production (static files) resource loading
- Dynamic visibility of configuration options based on selected resource type
- Input validation for file paths and server ports
- Local storage persistence of resource location preferences
- Status feedback and error handling for resource accessibility

### 3. Dual-Mode Architecture

**Modified**: `src/main.js`
- Complete rewrite from CommonJS to ES modules
- Introduction of BIOS window as the entry point
- Configurable main window creation based on BIOS selection
- IPC communication between BIOS and main application
- Support for both development server URLs and production file paths

**Resource Loading Flow**:
1. Application starts with BIOS configuration window
2. User specifies resource location (dev server port or static file path)
3. BIOS sends resource configuration to main process via IPC
4. Main window is created loading from the specified resource location (dev server URL or static file path)
5. Resource configuration is passed to renderer for dynamic library loading

### 4. Build System Consolidation

**New File**: `EXT_APP/build.sh`
```bash
#!/bin/bash
BACKEND_DIR="backend"
if [ ! -d "$BACKEND_DIR"/build ]; then
  mkdir -p "$BACKEND_DIR"/build
fi
cd $BACKEND_DIR
cmake -S . -B build
cmake --build build
cd ..
```

**Modified**: `package.json` scripts
- `build:dlib` → `build:app`: Updated to use new build script
- `build:native`: Now calls both addon and app builds
- Updated to reflect new project structure

### 5. ES Modules Migration

**Before (CommonJS)**:
```javascript
const { app, BrowserWindow } = require('electron')
const path = require('node:path')
```

**After (ES Modules)**:
```javascript
import { app, BrowserWindow, ipcMain } from 'electron';
import path from 'path';
import { fileURLToPath } from 'url';
const __dirname = path.dirname(fileURLToPath(import.meta.url));
```

**Package.json Update**: Added `"type": "module"` to enable ES modules support.

### 6. Dynamic Library Loading

**Previous**: Hardcoded library paths in both C++ addon and renderer
**New**: Configurable paths passed from main process

**C++ Addon Changes** (`src/native/addon.cpp`):
- `LoadDyLib` function now accepts path parameter
- Improved error messages with actual path information
- Removed hardcoded path logic

**Renderer Changes** (`EXT_APP/frontend/renderer.js`):
- Dynamic library path construction based on configuration
- Receives artifact path from main process via IPC

### 7. Build Configuration Simplification

**Modified**: `src/native/binding.gyp`
- Removed hardcoded library linking and rpath configuration
- Simplified build process by removing platform-specific linker flags
- Libraries are now loaded dynamically at runtime instead of link-time

## Technical Implementation Details

### IPC Communication
- `launch-main-app`: BIOS → Main (carries configuration object)
- `get-current-config`: Renderer → Main (requests current configuration)
- `set-app-config`: Main → Renderer (sends configuration to frontend)

### Configuration Object Structure
```javascript
{
  mode: 'dev' | 'prod',
  artifactPath: string | null,  // null in dev mode
  devServerPort: number | null  // null in prod mode
}
```

### Error Handling
- Input validation in BIOS interface
- Path verification for production mode
- Port validation for development mode
- Graceful fallbacks and user feedback

## Benefits of This Architecture

1. **Resource Location Flexibility**: Users can easily specify where application resources are located (dev server vs. static files)
2. **Development Workflow**: Seamless switching between loading from dev server during development and static files in production
3. **Configuration Persistence**: Resource location settings are saved and restored automatically
4. **Separation of Concerns**: Clear distinction between launcher and application resource loading
5. **Maintainability**: Better organized codebase with logical component separation
6. **User Experience**: Intuitive interface for directing the application to correct resource locations

## Future Considerations

1. **Configuration Validation**: Could add more robust path validation
2. **Advanced Settings**: Potential for additional configuration options
3. **Themes**: BIOS interface could support multiple visual themes
4. **Profiles**: Support for saving multiple configuration profiles
5. **Auto-Detection**: Intelligent detection of common development setups

## Breaking Changes

- **Module System**: Migration to ES modules requires Node.js environments that support ES modules
- **Entry Point**: Application now starts with BIOS instead of directly launching main window
- **File Paths**: Updated paths require corresponding changes in related scripts and configurations
- **Build Process**: New build scripts and structure may require CI/CD pipeline updates

## Testing Considerations

1. Test both development and production mode launches
2. Verify configuration persistence across restarts
3. Test error handling for invalid paths/ports
4. Ensure proper IPC communication
5. Validate build script functionality
6. Test on different platforms (macOS, Windows, Linux)

This architectural restructuring provides a solid foundation for a more flexible and maintainable Electron application while introducing modern development practices and improved user experience. 