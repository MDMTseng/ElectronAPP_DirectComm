{
  "targets": [
    {
      "target_name": "addon",
      "sources": [ "addon.cpp" ],
      "include_dirs": [
        "<!(node -p \"require('node-addon-api').include_dir\")"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "xcode_settings": {
        "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
        "CLANG_CXX_LIBRARY": "libc++",
        "MACOSX_DEPLOYMENT_TARGET": "10.7",
        "OTHER_LDFLAGS": [
          "-Wl,-rpath,@loader_path/../../../../ext_native/build",
          "-L../../ext_native/build",
          "-ldlib"
        ]
      },
      "msvs_settings": {
        "VCCLCompilerTool": { "ExceptionHandling": 1 }
      },
      "conditions": [
        ['OS=="linux"', {
          "ldflags": [
            "-Wl,-rpath,'$$ORIGIN/../../../../ext_native/build'",
            "-L../../ext_native/build",
            "-ldlib"
          ]
        }],
        ['OS=="win"', {
          "msvs_settings": {
            "VCLinkerTool": {
              "AdditionalLibraryDirectories": [
                "<(module_root_dir)/ext_native/build"
              ],
              "AdditionalDependencies": [
                "dlib.lib"
              ]
            }
          }
        }]
      ]
    }
  ]
} 