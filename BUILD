package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "roguelike",
    data = ["//content"],
    linkopts = [
        # TODO use location from archive
        "-libpath:c:/development/sdl2/lib/x64 SDL2.lib SDL2main.lib SDL2_image.lib SDL2_mixer.lib",
    ],
    srcs = ["main.cc"],
    deps = [
        "@libgam//:game",
        ":screens",
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
    ],
)

cc_library(
    name = "camera",
    srcs = [ "camera.cc" ],
    hdrs = [ "camera.h" ],
    deps = [ ":dungeon" ],
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
        ":rect",
    ],
)

cc_library(
    name = "rect",
    srcs = [ "rect.cc" ],
    hdrs = [ "rect.h" ],
    deps = [ "@libgam//:graphics" ],
)
