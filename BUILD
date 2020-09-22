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

cc_binary(
    name = "v2_example",
    deps = [
        "//v2:minituna_v2",
        "@shogun//:shogun",
    ],
    srcs = ["v2_example.cpp"]
)
