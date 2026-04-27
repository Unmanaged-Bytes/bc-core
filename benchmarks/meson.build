benchmark('bench_bc_core_ops',
    executable('bench_bc_core_ops', 'bench_bc_core_ops.c',
        include_directories : bc_core_public_inc,
        dependencies : [bc_core_dep],
    ),
    timeout : 120,
)

benchmark('bench_bc_core_format',
    executable('bench_bc_core_format', 'bench_bc_core_format.c',
        include_directories : bc_core_public_inc,
        dependencies : [bc_core_dep],
    ),
    timeout : 60,
)
