project('libghe', 'c',
  version : '0.1.0',
  license: 'MIT',
  meson_version: '>= 0.52.1',
  default_options : [
    'c_std=c11',
    'warning_level=3'])

version = meson.project_version().split('-')[0]
version_major = version.split('.')[0]
version_minor = version.split('.')[1]

cc = meson.get_compiler('c')
math = cc.find_library('m', required: false)

ghe_lib = library(
	'ghe',
	[
		'ghe_algorithm.c',
	],
	dependencies: [math],
	install: true,
	version: version,
	soversion: version_minor,
)

pkgconfig = import('pkgconfig')
pkgconfig.generate(
	ghe_lib,
	filebase: 'libghe',
	name: 'libghe',
	url: 'https://github.com/intel/ghe',
	description: 'Global Histogram Enhancement library',
)


ghe_dep = declare_dependency(
	link_with: ghe_lib,
)
meson.override_dependency('libghe', ghe_dep)
