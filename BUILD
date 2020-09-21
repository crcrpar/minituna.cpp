load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "v1_example",
    deps = [
        "//v1:minituna_v1",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/functional:function_ref",
        "@com_google_absl//absl/random:random",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:any",
    ],
    srcs = ["v1_example.cpp"]
)

load("@rules_foreign_cc//tools/build_defs:cmake.bzl", "cmake_external")

cmake_external(
    name = "openblas",
    # Values to be passed as -Dkey=value on the CMake command line;
    # here are serving to provide some CMake script configuration options
    cache_entries = {
       "NOFORTRAN": "on",
       "BUILD_WITHOUT_LAPACK": "no",
    },
    lib_source = "@openblas//:all",

    # We are selecting the resulting static library to be passed in C/C++ provider
    # as the result of the build;
    # However, the cmake_external dependants could use other artefacts provided by the build,
    # according to their CMake script
    static_libraries = ["libopenblas.a"],
)

cmake_external(
    name = "eigen",
    lib_source = "@eigen//:all",
    # These options help CMake to find prebuilt OpenBLAS, which will be copied into
    # $EXT_BUILD_DEPS/openblas by the cmake_external script
    cache_entries = {
       "BLAS_VENDOR": "OpenBLAS",
       "BLAS_LIBRARIES": "$EXT_BUILD_DEPS/openblas/lib/libopenblas.a",
    },
    headers_only = True,
    # Dependency on other cmake_external rule; can also depend on cc_import, cc_library rules
    deps = [":openblas"],
)

cmake_external(
    name = "rapidjson",
    lib_source = "@rapidjson//:all",
    cache_entries = {
        "RAPIDJSON_BUILD_DOC": "OFF",
        "RAPIDJSON_BUILD_EXAMPLES": "OFF",
        "RAPIDJSON_BUILD_TESTS": "OFF",
    },
    headers_only = True,
)

cmake_external(
    name = "spdlog",
    cache_entries = {
        "SPDLOG_BUILD_BENCH": "OFF",
        "SPDLOG_BUILD_ALL": "OFF",
    },
    lib_source = "@spdlog//:all",
    headers_only = True,
)

cc_binary(
    name = "v2_example",
    deps = [
        "//v2:minituna_v2",
    ],
    srcs = ["v2_example.cpp"]
)
