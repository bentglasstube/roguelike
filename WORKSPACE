# git_repository(
#     name = "libgam",
#     remote = "https://github.com/bentglasstube/gam.git",
#     tag = "v1.2"
# )

local_repository(
    name = "libgam",
    path = "c:/users/alan/source/gam/",
)

new_http_archive(
    name = "sdl",
    url = "https://www.libsdl.org/release/SDL2-devel-2.0.6-VC.zip",
    strip_prefix = "SDL2-2.0.6",
    build_file = "sdl.BUILD",
)

new_http_archive(
    name = "sdl_image",
    url = "https://www.libsdl.org/projects/SDL_image/release/SDL2_image-devel-2.0.1-VC.zip",
    strip_prefix = "SDL2_image-2.0.1",
    build_file = "sdl_image.BUILD",
)

new_http_archive(
    name = "sdl_mixer",
    url = "https://www.libsdl.org/projects/SDL_mixer/release/SDL2_mixer-devel-2.0.1-VC.zip",
    strip_prefix = "SDL2_mixer-2.0.1",
    build_file = "sdl_mixer.BUILD",
)
