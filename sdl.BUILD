cc_library(
  name = "sdl",
  hdrs = glob(["include/*.h"]),
  defines = ["_REENTRANT"],
  includes = ["include"],
  linkopts = [
    "-lSDL2",
    "-lSDL2main",
  ],
  visibility = ["//visibility:public"],
)
