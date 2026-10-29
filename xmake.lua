-- libjstd：Java 语言 / JVM 语义的 C++ 拟合 header 库（独立构建根）。
--
-- 依赖：
--   - libca（luiox-repo git 直连包）：core 模块（datatype 基础类型），头依赖。
--   - gtest（xrepo，仅 --with_tests=y）。
--
-- 仓库注册：本文件不声明 add_repositories——包沙箱会继承宿主的仓库注册，
-- 在工程内重复注册同名仓库会导致嵌套包构建时 remote 路径错乱。独立开发/CI
-- 先执行：
--   xmake repo --add luiox-repo https://github.com/luiox/luiox-repo.git

set_languages("cxx17")
add_rules("mode.debug", "mode.release")

add_requires("libca 0.0.16", {configs = {modules = "core"}})

option("with_tests")
    set_default(false)
    set_showmenu(true)
    set_description("Enable unit tests (pulls gtest)")
option_end()

if has_config("with_tests") then
    add_requires("gtest")
end

target("libjstd")
    set_kind("headeronly")
    add_packages("libca")
    add_headerfiles("src/(jstd/**.hpp)")
    add_includedirs("src", {public = true})
    if is_plat("windows") then
        add_cxflags("/utf-8", {tools = "cl"})
    end

if has_config("with_tests") then

target("libjstd_unittest")
    set_kind("binary")
    set_default(false)
    add_deps("libjstd")
    add_packages("libca")
    add_packages("gtest")
    add_files("unittest/main.cpp")
    add_files("unittest/*_test.cpp")
    add_includedirs("src")
    if is_plat("linux") then
        add_syslinks("pthread")
    end
    if is_plat("windows") then
        add_cxflags("/utf-8", {tools = "cl"})
    end

end
