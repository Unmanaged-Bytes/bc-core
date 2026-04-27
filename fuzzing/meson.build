# SPDX-License-Identifier: MIT

fuzzer_c_args = []
fuzzer_link_args = []
if cc.get_id() == 'clang'
    fuzzer_c_args += ['-fsanitize=fuzzer', '-DBC_FUZZ_LIBFUZZER']
    fuzzer_link_args += ['-fsanitize=fuzzer']
endif

fuzz_targets = [
    'bc_core_hash',
    'bc_core_memory',
    'bc_core_search',
    'bc_core_format',
    'bc_core_sort',
    'bc_core_byte_mask',
    'bc_core_math',
]

foreach name : fuzz_targets
    executable('fuzz_' + name,
        'fuzz_' + name + '.c',
        dependencies : [bc_core_dep],
        c_args : fuzzer_c_args,
        link_args : fuzzer_link_args,
    )
endforeach
