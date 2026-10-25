--- RULES/POLICIES
add_rules("mode.debug", "mode.release");
add_rules("plugin.compile_commands.autoupdate")


--- DEPENDENCIES
add_repositories("mj-xrepo https://github.com/monjaris/mj-xrepo.git")
add_requires("clixx")
add_requires("tiny-fmt")


--- CONFIG
add_cxxflags(
    "-std=c++23",
    "-Wall", "-Wextra",
    "-fno-exceptions", "-fno-rtti"
)

after_build(function(package)
    os.cp(package:targetfile(), "$(projectdir)")
end)

--- TARGETS
target("timeh")
    set_kind("binary")
    add_files("src/*.cpp")
    add_packages("clixx")
    add_packages("tiny-fmt")

    set_suffixname("." .. (get_config("mode") or ""))
