load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
# load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "minituna_v2",
    deps = [
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/functional:function_ref",
        "@com_google_absl//absl/random:random",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:any",
        "@com_google_absl//absl/types:variant",
        "@com_google_glog//:glog",
    ],
    srcs = ["minituna.cpp"],
    hdrs = ["minituna.hpp"],
)
