benchmark('bench_bc_core_ops',
    executable('bench_bc_core_ops', 'bench_bc_core_ops.c',
        include_directories : bc_core_public_inc,
        dependencies : [bc_core_dep],
    ),
    timeout : 120,
)

benchmark('bench_bc_core_fmt',
    executable('bench_bc_core_fmt', 'bench_bc_core_fmt.c',
        include_directories : bc_core_public_inc,
        dependencies : [bc_core_dep],
    ),
    timeout : 60,
)
