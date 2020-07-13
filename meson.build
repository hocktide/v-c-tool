project('vctool', 'c', 'cpp',
    version : '0.0.1',
    default_options : ['c_std=gnu11', 'cpp_std=c++14', 'buildtype=release'],
    meson_version : '>=0.53.0'
)

message('Using build type: ' + get_option('buildtype'))

force_velo_toolchain = get_option('force_velo_toolchain')

# TODO: Move this into mesons built in warning level.
add_project_arguments('-Wall', '-Werror', '-Wextra', language : 'c')
add_project_arguments('-Wall', '-Werror', '-Wextra', language : 'cpp')

flex = find_program('flex', required : true)
bison = find_program('bison', required : true)

src_not_main = run_command(
    'find', './src', '(', '-name', '*.c', '-or', '-name', '*.h' , ')', 
            '-not', '-name', 'main.c',
    check : true
).stdout().strip().split('\n')

test_src = run_command(
    'find', './test', '-name', '*.cpp', '-or', '-name', '*.h',
    check : true
).stdout().strip().split('\n')

vcblockchain = dependency(
    'vcblockchain',
    required : true,
    fallback : ['vcblockchain', 'vcblockchain_dep']
)

minunit = dependency(
    'minunit',
    main : true,
    required : true,
    fallback : ['minunit', 'minunit_dep']
)

threads = dependency('threads')

vctool_include = include_directories('include')

vctool_exe = executable(
    'vctool',
    './src/vctool/main.c',
    src_not_main,
    include_directories : vctool_include,
    dependencies : [threads, vcblockchain]
)

vctool_test = executable(
    'vctool-test',
    src_not_main, test_src,
    include_directories : vctool_include,
    dependencies : [threads, vcblockchain, minunit]
)

test(
    'vctool-test',
    vctool_test,
)
