{
    files = {
        "third_party/cimgui/imgui/imgui_draw.cpp"
    },
    depfiles_format = "gcc",
    values = {
        "/usr/bin/gcc",
        {
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
    depfiles = "imgui_draw.o: third_party/cimgui/imgui/imgui_draw.cpp  third_party/cimgui/imgui/imgui.h third_party/cimgui/imgui/imconfig.h  third_party/cimgui/imgui/imgui_internal.h  third_party/cimgui/imgui/imstb_rectpack.h  third_party/cimgui/imgui/imstb_truetype.h\
"
}