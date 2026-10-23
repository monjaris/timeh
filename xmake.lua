--- RULES/POLICIES
add_rules("mode.debug", "mode.release");
set_defaultmode("debug")
add_rules("plugin.compile_commands.autoupdate")


--- DEPENDENCIES
add_repositories("mj-xrepo https://github.com/monjaris/mj-xrepo.git")
add_requires("clixx")


--- CONFIG
set_languages("c++23")


--- TARGETS
target("timeh")
    set_kind("binary")
    add_files("src/*.cpp")

    add_packages("clixx")
