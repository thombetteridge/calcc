{
    files = {
        "third_party/cimgui/imgui/imgui_draw.cpp"
    },
    depfiles_format = "gcc",
    values = {
        "/usr/bin/clang",
        {
            "-Qunused-arguments",
            "-m64",
            "-fvisibility=hidden",
            "-fvisibility-inlines-hidden",
            "-w",
            "-O3",
            "-Ithird_party/cimgui/imgui",
            "-Ithird_party/cimgui",
            "-Ibuild/.gens/cimgui/linux/x86_64/release/platform/windows/idl",
            "-DNDEBUG"
        }
    },
    depfiles = "build/.objs/cimgui/linux/x86_64/release/third_party/cimgui/imgui/__cpp_imgui_draw.cpp.cpp:   third_party/cimgui/imgui/imgui_draw.cpp   third_party/cimgui/imgui/imgui.h third_party/cimgui/imgui/imconfig.h   third_party/cimgui/imgui/imgui_internal.h   third_party/cimgui/imgui/imstb_rectpack.h   third_party/cimgui/imgui/imstb_truetype.h\
"
}