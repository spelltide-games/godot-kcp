#!/usr/bin/env python

env = SConscript("godot-cpp/SConstruct")

env.Append(
    CPPPATH=[
        "src",
        ".",
        "kcp",
        "godot-cpp/include",
        "godot-cpp/gen/include",
        "godot-cpp/gdextension",
    ]
)
sources = Glob("src/*.cpp") + ["kcp/ikcp.c"]

if env["platform"] == "macos":
    library = env.SharedLibrary(
        "bin/libgodot-kcp.{}.{}.framework/libgodot-kcp.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
elif env["platform"] == "ios":
    if env["ios_simulator"]:
        library = env.StaticLibrary(
            "bin/libgodot-kcp.{}.{}.simulator.a".format(env["platform"], env["target"]),
            source=sources,
        )
    else:
        library = env.StaticLibrary(
            "bin/libgodot-kcp.{}.{}.a".format(env["platform"], env["target"]),
            source=sources,
        )
else:
    library = env.SharedLibrary(
        "bin/libgodot-kcp{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

env.NoCache(library)
Default(library)
