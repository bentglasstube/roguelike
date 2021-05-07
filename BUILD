package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "roguelike",
    data = ["//content"],
    linkopts = [
        "-lSDL2",
        "-lSDL2_image",
        "-lSDL2_mixer",
        "-static-libstdc++",
        "-static-libgcc",
    ],
    srcs = ["main.cc"],
    deps = [
        "@libgam//:game",
        ":config",
        ":screens",
    ],
)

cc_library(
    name = "config",
    srcs = ["config.cc"],
    hdrs = ["config.h"],
    deps = ["@libgam//:game"],
)

cc_library(
    name = "screens",
    srcs = [
        "title_screen.cc",
        "dungeon_screen.cc",
        "overworld_screen.cc",
    ],
    hdrs = [
        "title_screen.h",
        "dungeon_screen.h",
        "overworld_screen.h",
    ],
    deps = [
        "@libgam//:backdrop",
        "@libgam//:screen",
        "@libgam//:text",
        ":camera",
        ":dungeon",
        ":overworld",
    ],
)

cc_library(
    name = "camera",
    srcs = [ "camera.cc" ],
    hdrs = [ "camera.h" ],
    deps = [
        ":config",
        ":dungeon",
    ],
)

cc_library(
    name = "dungeon",
    srcs = [
        "bat.cc",
        "dungeon.cc",
        "dungeon_set.cc",
        "entity.cc",
        "player.cc",
        "powerup.cc",
        "slime.cc",
        "spike_trap.cc",
    ],
    hdrs = [
        "bat.h",
        "dungeon.h",
        "dungeon_set.h",
        "entity.h",
        "player.h",
        "powerup.h",
        "slime.h",
        "spike_trap.h",
    ],
    deps = [
        "@libgam//:graphics",
        "@libgam//:spritemap",
        "@libgam//:text",
        "@libgam//:util",
        ":log",
        ":rect",
    ],
)

cc_library(
    name = "rect",
    srcs = [ "rect.cc" ],
    hdrs = [ "rect.h" ],
    deps = [ "@libgam//:graphics" ],
)

cc_library(
    name = "log",
    hdrs = [ "log.h" ],
)

cc_library(
    name = "overworld",
    srcs = [ "overworld.cc" ],
    hdrs = [ "overworld.h" ],
    deps = [
        ":voronoi",
        "@libgam//:graphics"
    ],
)

cc_library(
    name = "voronoi",
    srcs = [ "voronoi.cc" ],
    hdrs = [
        "voronoi.h",
        "jc_voronoi.h",
        "stb_perlin.h",
    ],
    deps = [
        "@libgam//:graphics",
    ],
)
