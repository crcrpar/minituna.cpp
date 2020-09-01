load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
# load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

package(default_visibility = ["//visibility:public"])



cc_library(
    name = "minituna_v1",
    deps = [
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/functional:function_ref",
        "@com_google_absl//absl/random:random",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:any",
    ],
    srcs = ["minituna.cpp"],
    hdrs = ["minituna.hpp"],
)

cc_binary(
    name = "example",
    deps = [
        ":minituna_v1",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/functional:function_ref",
        "@com_google_absl//absl/random:random",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:any",
    ],
    srcs = ["example.cpp"]
)
