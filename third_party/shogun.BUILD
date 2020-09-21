cc_library(
    name = "shogun",
    srcs = glob(
        [
            "src/shogun/**.cpp"
        ],
    ),
    hdrs = glob(
        [
            "src/shogun/**.h",
        ],
    )
    deps = [
        ":openblas",
        ":eigen",
        ":spdlog",
        ":rapidjson",
    ]
    visibility = [""//visibility:public"],
)
