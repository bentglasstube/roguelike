package(default_visibility = ["//visibility:public"])

load("@bazel_tools//tools/build_defs/pkg:pkg.bzl", "pkg_tar")
load("@mxebzl//tools/windows:rules.bzl", "pkg_winzip")

config_setting(
    name = "windows",
    values = {
        "crosstool_top": "@mxebzl//tools/windows:toolchain",
    }
)

cc_binary(
    name = "roguelike",
    data = ["//content"],
    linkopts = select({
        ":windows": [ "-mwindows", "-lSDL2main" ],
        "//conditions:default": [],
    }) + [
        "-lSDL2",
        "-lSDL2_image",
        "-lSDL2_mixer",
        "-static-libstdc++",
        "-static-libgcc",
    ],
    srcs = ["main.cc"],
    deps = [
        "@libgam//:game",
        ":screens",
    ],
)

pkg_winzip(
    name = "roguelike-windows",
    files = [
        ":roguelike",
        "//content",
    ]
)

pkg_tar(
    name = "roguelike-linux",
    extension = "tgz",
    files = [
        ":roguelike",
        "//content",
    ],
)

cc_library(
    name = "screens",
    srcs = [
        "title_screen.cc",
        "dungeon_screen.cc",
    ],
    hdrs = [
        "title_screen.h",
        "dungeon_screen.h",
    ],
    deps = [
        "@libgam//:backdrop",
        "@libgam//:screen",
        "@libgam//:text",
        ":camera",
        ":dungeon",
        ":player",
    ],
)

cc_library(
    name = "camera",
    srcs = [ "camera.cc" ],
    hdrs = [ "camera.h" ],
    deps = [
        ":player",
    ],
)

cc_library(
    name = "dungeon",
    srcs = [ "dungeon.cc" ],
    hdrs = [ "dungeon.h" ],
    deps = [
        "@libgam//:graphics",
        "@libgam//:spritemap",
    ],
)

cc_library(
    name = "player",
    srcs = [ "player.cc" ],
    hdrs = [ "player.h" ],
    deps = [
        "@libgam//:graphics",
        "@libgam//:spritemap",
        ":dungeon",
        ":rect",
    ],
)

cc_library(
    name = "rect",
    srcs = [ "rect.cc" ],
    hdrs = [ "rect.h" ],
    deps = [ "@libgam//:graphics" ],
)
