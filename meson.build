#
# cidrtools
# 
project('cidrtools', 'c',
  version: files('src/version.txt'),
  meson_version: '>=1.10.0',
  default_options: [
    'c_std=c23',
    'warning_level=3',
    'b_pie=true',
    'prefix=/usr',
    'mandir=share/man',
    ],
)

# ----------------------
#
## version
#
conf_data = configuration_data()
conf_data.set('VERSION', meson.project_version())
version_h = configure_file(
  input: 'src/lib/version.h.in',
  output: 'version.h',
  configuration: conf_data
)

#
## Configs
#
#
# Install dirs - reused below for both build targets and data/config installs
# so every install_dir is consistent and respects cross-distro overrides.
#
d_etc = get_option('sysconfdir')
d_lib = get_option('libdir')
d_bin = get_option('bindir')
d_man = get_option('mandir')
d_share = get_option('datadir')

cpu_level = get_option('cpu_level')

if cpu_level == 'x86-64-v3'
  cpu_cflags = ['-march=x86-64-v3', '-mtune=generic']
else
  cpu_cflags = ['-march=x86-64', '-mtune=generic']
endif

optimized = false
if get_option('optimization') not in ['0', 'g']
  optimized = true
endif

use_omp = false
if get_option('use_omp')
  use_omp = true
endif


my_cflags = [
  ## General
  '-D_GNU_SOURCE',
  '-pipe',                              # Speeds up compilation
  '-fno-plt',                           # Optimization for shared library calls
  '-Wmissing-prototypes',               # Warn for global function missing prototype decl
  '-fvisibility=hidden',                # Only exported symbols in shared lib sym table
  ## Memory Protections
  '-fstack-protector-strong',           # Protect function calls with stack canaries
  '-fstack-clash-protection',           # Prevents stack-clash exploits
  '-ftrivial-auto-var-init=zero',       # Eliminate uninitialized stack memory leaks
  '-fzero-call-used-regs=used-gpr',     # Wipe general registers before returning
  ## Control Flow & Runtime Integrity
  '-mshstk',                            # Intel Shadow Stack protection
  '-fcf-protection',                    # Control-flow enforcement
  ## Aggressive Diagnostic Hardening
  '-Wshadow',                           # Warn if local variable/decl shadows another
  '-Wconversion',                       # Warn about type conversions that may alter value
  '-Wformat=2',                         # Warn about format/args mismatch printf, scanf etc
  '-Werror=format-security',            # Also warn about insecure printf format strings
] + cpu_cflags

buildtype = get_option('buildtype')

if optimized
  my_cflags += ['-D_FORTIFY_SOURCE=3']
endif

if get_option('analyzer')
  my_cflags += ['-fanalyzer']
endif

#
# link-time hardening
#
my_ldflags = [
  '-Wl,--as-needed',                    # Limit shared to 'as needed'
  '-Wl,-z,relro',                       # Mark relocation tables read-only
  '-Wl,-z,now',                         # Force immediate binding (Full RELRO)
  '-Wl,-z,noexecstack',                 # Strictly enforce non-executable stack
  '-Wl,-z,pack-relative-relocs',        # Optimize layout with DT_RELR packing
  '-Wl,-z,defs',                        # Catch unresolved symbols at link time, not at runtime
]

#
# Apply cflags/ldflags
#
add_project_arguments(my_cflags, language: 'c')
add_project_link_arguments(my_ldflags, language: 'c')

#
## Dependenceis
#
version_dep = declare_dependency(sources: version_h)
cc = meson.get_compiler('c')
resolv_dep = cc.find_library('resolv', required: true)

openmp_dep = dependency('openmp', required: use_omp)

inc_dir = []
lib_sources = []

subdir('src/lib')
subdir('src/tests')


# ----------------------
## Libs
#
cidrtools_lib = library('cidrtools',
  sources: lib_sources,
  include_directories: inc_dir,
  c_args: my_cflags,
  link_args: my_ldflags,
  dependencies: [
    version_dep,
    resolv_dep,
    openmp_dep,
    ],
    install: true,
  install_dir: d_lib
  )

cidrtools_dep = declare_dependency(
  link_with: cidrtools_lib,
  include_directories: inc_dir
)

# Define the headers to install
lib_headers = files([
  'src/lib/cidrtools.h',
  'src/lib/cidrtools-export.h'
])

# Install them into /usr/include/cidrtools/
install_headers(lib_headers, subdir: 'cidrtools')


# ----------------------
## Executables
#
hostcheck_bin = executable('hostcheck',
  sources: [
    'src/apps/hostcheck.c',
    ],
  dependencies: [cidrtools_dep, resolv_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: true,
  install_dir: d_bin
)

#
## Unit Test Executables
#
t_str_to_ip_address = executable('t_str_to_ip_address',
  sources: [
    'src/tests/t_str_to_ip_address.c',
    ],
  dependencies: [cidrtools_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: false,
  install_dir: d_bin
)

t_str_to_cidr_parts = executable('t_cidr_parts',
  sources: [
    'src/tests/t_cidr_parts.c',
    ],
  dependencies: [cidrtools_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: false,
  install_dir: d_bin
)

t_ip_address_range = executable('t_ip_address_range',
  sources: [
    'src/tests/t_ip_address_range.c',
    ],
  dependencies: [cidrtools_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: false,
  install_dir: d_bin
)

t_cidr_is_subnet = executable('t_cidr_is_subnet',
  sources: ['src/tests/t_cidr_is_subnet.c'],
  dependencies: [cidrtools_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: false,
  install_dir: d_bin
)

t_ip_address_to_str = executable('t_ip_address_to_str',
  sources: ['src/tests/t_ip_address_to_str.c'],
  dependencies: [cidrtools_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: false,
  install_dir: d_bin
)

t_cidr_to_str = executable('t_cidr_to_str',
  sources: ['src/tests/t_cidr_to_str.c'],
  dependencies: [cidrtools_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: false,
  install_dir: d_bin
)

t_num_ips = executable('t_num_ips',
  sources: ['src/tests/t_num_ips.c'],
  dependencies: [cidrtools_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: false,
  install_dir: d_bin
)

t_ip_address_increment = executable('t_ip_address_increment',
  sources: ['src/tests/t_ip_address_increment.c'],
  dependencies: [cidrtools_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: false,
  install_dir: d_bin
)

t_range_to_cidrs = executable('t_range_to_cidrs',
  sources: ['src/tests/t_range_to_cidrs.c'],
  dependencies: [cidrtools_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: false,
  install_dir: d_bin
)

t_cidr_to_range = executable('t_cidr_to_range',
  sources: ['src/tests/t_cidr_to_range.c'],
  dependencies: [cidrtools_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: false,
  install_dir: d_bin
)

t_cidr_to_range_mid = executable('t_cidr_to_range_mid',
  sources: ['src/tests/t_cidr_to_range_mid.c'],
  dependencies: [cidrtools_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: false,
  install_dir: d_bin
)

t_compact = executable('t_compact',
  sources: ['src/tests/t_compact.c'],
  dependencies: [cidrtools_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: false,
  install_dir: d_bin
)

t_sort = executable('t_sort',
  sources: ['src/tests/t_sort.c'],
  dependencies: [cidrtools_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: false,
  install_dir: d_bin
)

t_family_checks = executable('t_family_checks',
  sources: ['src/tests/t_family_checks.c'],
  dependencies: [cidrtools_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: false,
  install_dir: d_bin
)

t_cidr_fix_host_bits = executable('t_cidr_fix_host_bits',
  sources: ['src/tests/t_cidr_fix_host_bits.c'],
  dependencies: [cidrtools_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: false,
  install_dir: d_bin
)

t_exclude_cidrs = executable('t_exclude_cidrs',
  sources: ['src/tests/t_exclude_cidrs.c'],
  dependencies: [cidrtools_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: false,
  install_dir: d_bin
)

t_set_prefix = executable('t_set_prefix',
  sources: ['src/tests/t_set_prefix.c'],
  dependencies: [cidrtools_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: false,
  install_dir: d_bin
)

t_subnets_split = executable('t_subnets_split',
  sources: ['src/tests/t_subnets_split.c'],
  dependencies: [cidrtools_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: false,
  install_dir: d_bin
)

t_clean_cidrs = executable('t_clean_cidrs',
  sources: ['src/tests/t_clean_cidrs.c'],
  dependencies: [cidrtools_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: false,
  install_dir: d_bin
)

t_clean_cidr = executable('t_clean_cidr',
  sources: ['src/tests/t_clean_cidr.c'],
  dependencies: [cidrtools_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: false,
  install_dir: d_bin
)

t_str_to_cidr_block = executable('t_str_to_cidr_block',
  sources: ['src/tests/t_str_to_cidr_block.c'],
  dependencies: [cidrtools_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: false,
  install_dir: d_bin
)

t_cidr_host_bits = executable('t_cidr_host_bits',
  sources: ['src/tests/t_cidr_host_bits.c'],
  dependencies: [cidrtools_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: false,
  install_dir: d_bin
)

t_cidr_contains_ip = executable('t_cidr_contains_ip',
  sources: ['src/tests/t_cidr_contains_ip.c'],
  dependencies: [cidrtools_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: false,
  install_dir: d_bin
)


t_ip_str_to_hostname = executable('t_ip_str_to_hostname',
  sources: ['src/tests/t_ip_str_to_hostname.c'],
  dependencies: [cidrtools_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: false,
  install_dir: d_bin
)

t_hostname_to_address = executable('t_hostname_to_address',
  sources: ['src/tests/t_hostname_to_address.c'],
  dependencies: [cidrtools_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: false,
  install_dir: d_bin
)

t_split_by_family = executable('t_split_by_family',
  sources: ['src/tests/t_split_by_family.c'],
  dependencies: [cidrtools_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: false,
  install_dir: d_bin
)

t_version = executable('t_version',
  sources: ['src/tests/t_version.c'],
  dependencies: [cidrtools_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: false,
  install_dir: d_bin
)

t_flat_buffer_to_cidrs = executable('t_flat_buffer_to_cidrs',
  sources: ['src/tests/t_flat_buffer_to_cidrs.c'],
  dependencies: [cidrtools_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: false,
  install_dir: d_bin
)

t_cidrs_to_flat_buffer = executable('t_cidrs_to_flat_buffer',
  sources: ['src/tests/t_cidrs_to_flat_buffer.c'],
  dependencies: [cidrtools_dep],
  c_args: my_cflags,
  link_args: my_ldflags,
  install: false,
  install_dir: d_bin
)

# ------------------
## Install
#
s_docs = 'src/data/docs'
s_man = 'src/data/man'
s_license = 'src/data/license'

p_man1 = 'man1'
manual = 'cidrtools.pdf'

install_data(
  [ 
    s_docs / manual,
  ],
  install_dir: d_share / 'cidrtools' 
)

install_data(
  [
    s_license / 'License-gpl-2.0.txt',
    s_license / 'License.rst',
  ],
  install_dir: d_share / 'cidrtools/License', 
)

install_data(
  [
    s_license / 'License-gpl-2.0.txt',
    s_license / 'License.rst',
  ],
  install_dir: d_share / 'licenses/cidrtools'
)

install_data(
  [
    s_man / p_man1 / 'hostcheck.1.gz',
  ],
  install_dir: d_man / p_man1
)



# ----------------------------
## Tests
#
#
add_test_setup('standard',
  is_default: true,
)

#
# Valgrind setup (wraps the execution automatically)
# We normally use 
# --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all
#  --error-exitcode=1 --vex-iropt-register-updates=allregs-at-mem-access
#  --suppressions=valgrind.suppress
#
add_test_setup('valgrind',
  exe_wrapper: [
    'valgrind',
    '--tool=memcheck',
    '--leak-check=full',
    '--show-leak-kinds=all',
    '--track-origins=yes',
    '--error-exitcode=1' # Forces Meson to mark the test as FAILED if leaks occur
  ],
  timeout_multiplier: 10 # Valgrind is slow; prevent premature timeouts
)


# Register it with Meson's test runner
test( 't_str_to_ip_address,_test', t_str_to_ip_address, suite: ['unit', 'network'])
test( 't_str_to_cidr_parts_test', t_str_to_cidr_parts, suite: ['unit', 'network'])
test( 't_ip_address_range_test', t_ip_address_range, suite: ['unit', 'network'])
test( 't_cidr_is_subnet_test', t_cidr_is_subnet, suite: ['unit', 'network'])
test( 't_ip_address_to_str_test', t_ip_address_to_str, suite: ['unit', 'network'])
test( 't_cidr_to_str_test', t_cidr_to_str, suite: ['unit', 'network'])
test( 't_num_ips_test', t_num_ips, suite: ['unit', 'network'])
test( 't_ip_address_increment_test', t_ip_address_increment, suite: ['unit', 'network'])
test( 't_range_to_cidrs_test', t_range_to_cidrs, suite: ['unit', 'network'])
test( 't_cidr_to_range_test', t_cidr_to_range, suite: ['unit', 'network'])
test( 't_cidr_to_range_mid_test', t_cidr_to_range_mid, suite: ['unit', 'network'])
test( 't_compact_test', t_compact, suite: ['unit', 'network'])
test( 't_sort_test', t_sort, suite: ['unit', 'network'])
test( 't_family_checks_test', t_family_checks, suite: ['unit', 'network'])
test( 't_cidr_fix_host_bits_test', t_cidr_fix_host_bits, suite: ['unit', 'network'])
test( 't_exclude_cidrs_test', t_exclude_cidrs, suite: ['unit', 'network'])
test( 't_set_prefix_test', t_set_prefix, suite: ['unit', 'network'])
test( 't_subnets_split_test', t_subnets_split, suite: ['unit', 'network'])
test( 't_clean_cidrs_test', t_clean_cidrs, suite: ['unit', 'network'])
test( 't_clean_cidr_test', t_clean_cidr, suite: ['unit', 'network'])
test( 't_str_to_cidr_block_test', t_str_to_cidr_block, suite: ['unit', 'network'])
test( 't_cidr_host_bits_test', t_cidr_host_bits, suite: ['unit', 'network'])
test( 't_cidr_contains_ip_test', t_cidr_contains_ip, suite: ['unit', 'network'])
test( 't_version', t_version, suite: ['unit', 'network'])

test('t_ip_str_to_hostname', t_ip_str_to_hostname, suite: ['unit', 'network'])
test('t_hostname_to_address', t_hostname_to_address, suite: ['unit', 'network'])
test('t_split_by_family', t_split_by_family, suite: ['unit', 'network'])
test('t_flat_buffer_to_cidrs', t_flat_buffer_to_cidrs, suite: ['unit', 'network'])
test('t_cidrs_to_flat_buffer', t_cidrs_to_flat_buffer, suite: ['unit', 'network'])




