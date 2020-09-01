load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
# load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")
# load("@com_google_glog//bazel:glog.bzl", "glog_library")

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
