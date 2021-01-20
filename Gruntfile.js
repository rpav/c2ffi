module.exports = function(grunt) {
  grunt.loadNpmTasks('grunt-rpav-cmake');

  grunt.initConfig({
    cmake_config: {
      options: {
        srcDir: '.',
        buildDir: o => "build/" + o.buildConfig,
        env: [
          ["LSAN_OPTIONS", "suppressions=../../../lsan_suppress.txt"],
        ],
      },

      "clang-Debug": {
        options: { args: "-DTOOLCHAIN=clang -DBUILD_CONFIG=Debug" },
      },

      "clang-Sanitize": {
        options: { args: "-DTOOLCHAIN=clang -DBUILD_CONFIG=Sanitize" },
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
