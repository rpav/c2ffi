const { cmake } = require('./CMake/gruntutil.js');

module.exports = function(grunt) {
  grunt.loadNpmTasks('grunt-rpav-cmake');

  grunt.initConfig({
    cmake_config: {
      options: {
        srcDir: '.',
        buildDir: o => "build/" + o.buildConfig,
        env: {
          LSAN_OPTIONS: "suppressions=../../../lsan_suppress.txt",
        },
      },

      configs: {
        "clang-Debug": cmake({ tc: 'clang', c: 'Debug' }),
        "clang-Sanitize": cmake({ tc: 'clang', c: 'Sanitize' }),
        "clang-Release": cmake({ tc: 'clang', c: 'Release' }),
      },
    },

    cmake_run: {
      c2ffi: { build: "c2ffi", run: "./c2ffi", args: '/usr/include/stdio.h', cwd: "bin" },
    },

    cmake_build_run: {
      c2ffi: {},
    },

    run: {}
  });
};
