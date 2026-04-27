cmocka_dep = dependency('cmocka')

memory_dir = '../src/memory'
hash_dir = '../src/hash'
cpu_dir = '../src/cpu'

test_bc_core_math = executable('test_bc_core_math',
    'test_bc_core_math.c',
    dependencies : [bc_core_dep, cmocka_dep],
)

test_bc_core_public_headers_hygiene = executable('test_bc_core_public_headers_hygiene',
    'test_bc_core_public_headers_hygiene.c',
    c_args : ['-DBC_CORE_SOURCE_ROOT="' + meson.project_source_root() + '"'],
    dependencies : [cmocka_dep],
)

test_bc_core_unaligned = executable('test_bc_core_unaligned',
    'test_bc_core_unaligned.c',
    dependencies : [bc_core_dep, cmocka_dep],
)

test_bc_core_cpu = executable('test_bc_core_cpu',
    'test_bc_core_cpu.c',
    dependencies : [bc_core_dep, cmocka_dep],
)

test_bc_core_fill = executable('test_bc_core_fill',
    'test_bc_core_fill.c',
    include_directories : [bc_core_internal_inc],
    dependencies : [bc_core_dep, cmocka_dep],
)

test_bc_core_copy = executable('test_bc_core_copy',
    'test_bc_core_copy.c',
    dependencies : [bc_core_dep, cmocka_dep],
)

test_bc_core_compare = executable('test_bc_core_compare',
    'test_bc_core_compare.c',
    dependencies : [bc_core_dep, cmocka_dep],
)

test_bc_core_equal_case_insensitive_ascii = executable('test_bc_core_equal_case_insensitive_ascii',
    'test_bc_core_equal_case_insensitive_ascii.c',
    dependencies : [bc_core_dep, cmocka_dep],
)
test('equal_case_insensitive_ascii', test_bc_core_equal_case_insensitive_ascii)

test_bc_core_compare_alignment = executable('test_bc_core_compare_alignment',
    'test_bc_core_compare_alignment.c',
    dependencies : [bc_core_dep, cmocka_dep],
)
test('compare_alignment', test_bc_core_compare_alignment)

test_bc_core_contains = executable('test_bc_core_contains',
    'test_bc_core_contains.c',
    dependencies : [bc_core_dep, cmocka_dep],
)

test_bc_core_find = executable('test_bc_core_find',
    'test_bc_core_find.c',
    dependencies : [bc_core_dep, cmocka_dep],
)

test_bc_core_count = executable('test_bc_core_count',
    'test_bc_core_count.c',
    dependencies : [bc_core_dep, cmocka_dep],
)

test_bc_core_length = executable('test_bc_core_length',
    'test_bc_core_length.c',
    dependencies : [cmocka_dep, bc_core_dep],
)

test_bc_core_ascii = executable('test_bc_core_ascii',
    'test_bc_core_ascii.c',
    dependencies : [cmocka_dep, bc_core_dep],
)

test_bc_core_checksum = executable('test_bc_core_checksum',
    'test_bc_core_checksum.c',
    dependencies : [bc_core_dep, cmocka_dep],
)

test_bc_core_checksum_wrap = executable('test_bc_core_checksum_wrap',
    'test_bc_core_checksum_wrap.c',
    hash_dir + '/bc_core_hash_crc32c.c',
    hash_dir + '/bc_core_hash_sha256.c',
    include_directories : [bc_core_internal_inc],
    dependencies : [bc_core_dep, cmocka_dep],
    link_args : ['-Wl,--wrap=bc_core_cpu_features_detect'],
)

test_bc_core_cpu_wrap = executable('test_bc_core_cpu_wrap',
    'test_bc_core_cpu_wrap.c',
    cpu_dir + '/bc_core_cpu.c',
    include_directories : [bc_core_internal_inc],
    dependencies : [bc_core_dep, cmocka_dep],
    link_args : ['-Wl,--wrap=sysconf'],
)

test('math', test_bc_core_math)
test('public_headers_hygiene', test_bc_core_public_headers_hygiene)
test('unaligned', test_bc_core_unaligned)
test('cpu', test_bc_core_cpu)
test('fill', test_bc_core_fill)
test('copy', test_bc_core_copy)
test('compare', test_bc_core_compare)
test('contains', test_bc_core_contains)
test('find', test_bc_core_find)
test('count', test_bc_core_count)
test('length', test_bc_core_length)
test('ascii', test_bc_core_ascii)
test('checksum', test_bc_core_checksum, suite : 'hw')

test_bc_core_zero_wrap = executable('test_bc_core_zero_wrap',
    'test_bc_core_zero_wrap.c',
    memory_dir + '/bc_core_zero.c',
    cpu_dir + '/bc_core_cpu.c',
    cpu_dir + '/bc_core_cache_sizes.c',
    include_directories : [bc_core_internal_inc],
    dependencies : [bc_core_dep, cmocka_dep],
    link_args : ['-Wl,--wrap=sysconf'],
)

test_bc_core_copy_wrap = executable('test_bc_core_copy_wrap',
    'test_bc_core_copy_wrap.c',
    memory_dir + '/bc_core_copy.c',
    cpu_dir + '/bc_core_cache_sizes.c',
    include_directories : [bc_core_internal_inc],
    dependencies : [bc_core_dep, cmocka_dep],
    link_args : ['-Wl,--wrap=bc_core_l2_cache_size'],
)

test_bc_core_cpu_dispatch_wrap = executable('test_bc_core_cpu_dispatch_wrap',
    'test_bc_core_cpu_dispatch_wrap.c',
    cpu_dir + '/bc_core_cpu.c',
    include_directories : [bc_core_internal_inc],
    dependencies : [bc_core_dep, cmocka_dep],
    link_args : ['-Wl,--wrap=bc_core_cpu_features_detect'],
)

test_bc_core_equal_dispatch_wrap = executable('test_bc_core_equal_dispatch_wrap',
    'test_bc_core_equal_dispatch_wrap.c',
    memory_dir + '/bc_core_equal.c',
    include_directories : [bc_core_internal_inc],
    dependencies : [bc_core_dep, cmocka_dep],
    link_args : ['-Wl,--wrap=bc_core_cpu_features_detect'],
)

test_bc_core_compare_dispatch_wrap = executable('test_bc_core_compare_dispatch_wrap',
    'test_bc_core_compare_dispatch_wrap.c',
    memory_dir + '/bc_core_compare.c',
    include_directories : [bc_core_internal_inc],
    dependencies : [bc_core_dep, cmocka_dep],
    link_args : ['-Wl,--wrap=bc_core_cpu_features_detect'],
)

test_bc_core_find_dispatch_wrap = executable('test_bc_core_find_dispatch_wrap',
    'test_bc_core_find_dispatch_wrap.c',
    memory_dir + '/bc_core_find.c',
    include_directories : [bc_core_internal_inc],
    dependencies : [bc_core_dep, cmocka_dep],
    link_args : ['-Wl,--wrap=bc_core_cpu_features_detect'],
)

test_bc_core_count_dispatch_wrap = executable('test_bc_core_count_dispatch_wrap',
    'test_bc_core_count_dispatch_wrap.c',
    memory_dir + '/bc_core_count.c',
    include_directories : [bc_core_internal_inc],
    dependencies : [bc_core_dep, cmocka_dep],
    link_args : ['-Wl,--wrap=bc_core_cpu_features_detect'],
)

test_bc_core_zero_dispatch_wrap = executable('test_bc_core_zero_dispatch_wrap',
    'test_bc_core_zero_dispatch_wrap.c',
    memory_dir + '/bc_core_zero.c',
    cpu_dir + '/bc_core_cpu.c',
    cpu_dir + '/bc_core_cache_sizes.c',
    include_directories : [bc_core_internal_inc],
    dependencies : [bc_core_dep, cmocka_dep],
    link_args : ['-Wl,--wrap=bc_core_cpu_features_detect', '-Wl,--wrap=sysconf'],
)

test_bc_core_fill_dispatch_wrap = executable('test_bc_core_fill_dispatch_wrap',
    'test_bc_core_fill_dispatch_wrap.c',
    memory_dir + '/bc_core_fill.c',
    cpu_dir + '/bc_core_cache_sizes.c',
    include_directories : [bc_core_internal_inc],
    dependencies : [bc_core_dep, cmocka_dep],
    link_args : ['-Wl,--wrap=bc_core_cpu_features_detect'],
)

test_bc_core_copy_dispatch_wrap = executable('test_bc_core_copy_dispatch_wrap',
    'test_bc_core_copy_dispatch_wrap.c',
    memory_dir + '/bc_core_copy.c',
    cpu_dir + '/bc_core_cache_sizes.c',
    include_directories : [bc_core_internal_inc],
    dependencies : [bc_core_dep, cmocka_dep],
    link_args : ['-Wl,--wrap=bc_core_cpu_features_detect', '-Wl,--wrap=bc_core_l2_cache_size'],
)

test_bc_core_move_dispatch_wrap = executable('test_bc_core_move_dispatch_wrap',
    'test_bc_core_move_dispatch_wrap.c',
    memory_dir + '/bc_core_move.c',
    cpu_dir + '/bc_core_cache_sizes.c',
    include_directories : [bc_core_internal_inc],
    dependencies : [bc_core_dep, cmocka_dep],
    link_args : ['-Wl,--wrap=bc_core_cpu_features_detect'],
)

test_bc_core_swap_dispatch_wrap = executable('test_bc_core_swap_dispatch_wrap',
    'test_bc_core_swap_dispatch_wrap.c',
    memory_dir + '/bc_core_swap.c',
    include_directories : [bc_core_internal_inc],
    dependencies : [bc_core_dep, cmocka_dep],
    link_args : ['-Wl,--wrap=bc_core_cpu_features_detect'],
)

test_bc_core_copy_page_boundary = executable('test_bc_core_copy_page_boundary',
    'test_bc_core_copy_page_boundary.c',
    dependencies : [bc_core_dep, cmocka_dep],
)

test('checksum_wrap', test_bc_core_checksum_wrap)
test('cpu_wrap', test_bc_core_cpu_wrap)
test('zero_wrap', test_bc_core_zero_wrap)
test('copy_wrap', test_bc_core_copy_wrap)
test('cpu_dispatch_wrap', test_bc_core_cpu_dispatch_wrap, suite : 'hw')
test('equal_dispatch_wrap', test_bc_core_equal_dispatch_wrap, suite : 'hw')
test('compare_dispatch_wrap', test_bc_core_compare_dispatch_wrap, suite : 'hw')
test('find_dispatch_wrap', test_bc_core_find_dispatch_wrap, suite : 'hw')
test('count_dispatch_wrap', test_bc_core_count_dispatch_wrap, suite : 'hw')
test('zero_dispatch_wrap', test_bc_core_zero_dispatch_wrap, suite : 'hw')
test('fill_dispatch_wrap', test_bc_core_fill_dispatch_wrap, suite : 'hw')
test('copy_dispatch_wrap', test_bc_core_copy_dispatch_wrap, suite : 'hw')
test('move_dispatch_wrap', test_bc_core_move_dispatch_wrap, suite : 'hw')
test('swap_dispatch_wrap', test_bc_core_swap_dispatch_wrap, suite : 'hw')
test('copy_page_boundary', test_bc_core_copy_page_boundary)

test_bc_core_alignment = executable('test_bc_core_alignment',
    'test_bc_core_alignment.c',
    dependencies : [bc_core_dep, cmocka_dep],
)
test('alignment', test_bc_core_alignment)

test_bc_core_cpu_features_cache = executable('test_bc_core_cpu_features_cache',
    'test_bc_core_cpu_features_cache.c',
    cpu_dir + '/bc_core_cpu_features.c',
    include_directories : [bc_core_public_inc, bc_core_internal_inc],
    dependencies : [cmocka_dep],
)
test('cpu_features_cache', test_bc_core_cpu_features_cache)

test_bc_core_cpu_features_cache_concurrent = executable('test_bc_core_cpu_features_cache_concurrent',
    'test_bc_core_cpu_features_cache_concurrent.c',
    cpu_dir + '/bc_core_cpu_features.c',
    include_directories : [bc_core_public_inc, bc_core_internal_inc],
    dependencies : [cmocka_dep, dependency('threads')],
)
test('cpu_features_cache_concurrent', test_bc_core_cpu_features_cache_concurrent)

test_bc_core_prefetch = executable('test_bc_core_prefetch',
    'test_bc_core_prefetch.c',
    dependencies : [bc_core_dep, cmocka_dep],
)
test('prefetch', test_bc_core_prefetch)

test_bc_core_with_policy = executable('test_bc_core_with_policy',
    'test_bc_core_with_policy.c',
    dependencies : [bc_core_dep, cmocka_dep],
)
test('with_policy', test_bc_core_with_policy)

test_bc_core_count_words_ascii = executable('test_bc_core_count_words_ascii',
    'test_bc_core_count_words_ascii.c',
    dependencies : [bc_core_dep, cmocka_dep],
)
test('count_words_ascii', test_bc_core_count_words_ascii)

test_bc_core_format = executable('test_bc_core_format',
    'test_bc_core_format.c',
    dependencies : [bc_core_dep, cmocka_dep],
)
test('format', test_bc_core_format)

test_bc_core_writer = executable('test_bc_core_writer',
    'test_bc_core_writer.c',
    dependencies : [bc_core_dep, cmocka_dep],
)
test('writer', test_bc_core_writer)

test_bc_core_reader = executable('test_bc_core_reader',
    'test_bc_core_reader.c',
    dependencies : [bc_core_dep, cmocka_dep],
)
test('reader', test_bc_core_reader)

test_bc_core_error = executable('test_bc_core_error',
    'test_bc_core_error.c',
    dependencies : [bc_core_dep, cmocka_dep],
)
test('error', test_bc_core_error)

test_bc_core_writer_extensions = executable('test_bc_core_writer_extensions',
    'test_bc_core_writer_extensions.c',
    dependencies : [bc_core_dep, cmocka_dep],
)
test('writer_extensions', test_bc_core_writer_extensions)

test_bc_core_sort = executable('test_bc_core_sort',
    'test_bc_core_sort.c',
    dependencies : [bc_core_dep, cmocka_dep],
)
test('sort', test_bc_core_sort)

test_bc_core_byte_mask = executable('test_bc_core_byte_mask',
    'test_bc_core_byte_mask.c',
    dependencies : [bc_core_dep, cmocka_dep],
)
test('byte_mask', test_bc_core_byte_mask)

test_bc_core_thread_stress = executable('test_bc_core_thread_stress',
    'test_bc_core_thread_stress.c',
    dependencies : [bc_core_dep, cmocka_dep, dependency('threads')],
)
test('thread_stress', test_bc_core_thread_stress, timeout : 120)

test_bc_core_huge_inputs = executable('test_bc_core_huge_inputs',
    'test_bc_core_huge_inputs.c',
    dependencies : [bc_core_dep, cmocka_dep],
)
test('huge_inputs', test_bc_core_huge_inputs, timeout : 600, suite : 'slow')

test_bc_core_writer_io_wrap = executable('test_bc_core_writer_io_wrap',
    'test_bc_core_writer_io_wrap.c',
    '../src/io/bc_core_writer.c',
    include_directories : [bc_core_internal_inc],
    dependencies : [bc_core_dep, cmocka_dep],
    link_args : ['-Wl,--wrap=write'],
)
test('writer_io_wrap', test_bc_core_writer_io_wrap)

test_bc_core_reader_io_wrap = executable('test_bc_core_reader_io_wrap',
    'test_bc_core_reader_io_wrap.c',
    '../src/io/bc_core_reader.c',
    include_directories : [bc_core_internal_inc],
    dependencies : [bc_core_dep, cmocka_dep],
    link_args : ['-Wl,--wrap=read'],
)
test('reader_io_wrap', test_bc_core_reader_io_wrap)
