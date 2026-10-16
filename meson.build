project('mauikit', ['c', 'cpp'], version: '4.0.5', meson_version: '>=1.0.0', default_options: ['cpp_std=c++17', 'buildtype=release'])

cpp_compiler = meson.get_compiler('cpp')
if cpp_compiler.get_id() == 'gcc' and cpp_compiler.has_argument('-Wno-sfinae-incomplete')
    add_project_arguments('-Wno-sfinae-incomplete', language: 'cpp')
endif
qt6 = import('qt6')
qt = dependency('qt6', version: '>=6.9', modules: ['Core', 'Concurrent', 'DBus', 'Gui', 'Multimedia', 'Qml', 'Quick', 'QuickControls2', 'WaylandClient', 'Svg', 'Widgets'])
qt6_includedir = qt.get_variable(pkgconfig: 'includedir')
qt6_version = qt.version()
qt_private_include = include_directories(
    qt6_includedir / 'QtCore' / qt6_version,
    qt6_includedir / 'QtCore' / qt6_version / 'QtCore',
    qt6_includedir / 'QtGui' / qt6_version,
    qt6_includedir / 'QtGui' / qt6_version / 'QtGui',
    qt6_includedir / 'QtWaylandClient' / qt6_version,
    qt6_includedir / 'QtWaylandClient' / qt6_version / 'QtWaylandClient',
    qt6_includedir / 'QtWaylandGlobal' / qt6_version,
    qt6_includedir / 'QtWaylandGlobal' / qt6_version / 'QtWaylandGlobal',
)
dbus = dependency('dbus-1')
kf6_include_prefix = get_option('prefix') / 'include' / 'KF6'
kf6_core = declare_dependency(
    include_directories: include_directories(kf6_include_prefix / 'KCoreAddons'),
    dependencies: [cpp_compiler.find_library('KF6CoreAddons')],
)
kf6_config = declare_dependency(
    include_directories: include_directories(kf6_include_prefix / 'KConfigCore'),
    dependencies: [cpp_compiler.find_library('KF6ConfigCore')],
)
kf6_i18n = declare_dependency(
    include_directories: include_directories(kf6_include_prefix / 'KI18n'),
    dependencies: [cpp_compiler.find_library('KF6I18n')],
)
kf6_notifications = declare_dependency(
    include_directories: include_directories(kf6_include_prefix / 'KNotifications'),
    dependencies: [cpp_compiler.find_library('KF6Notifications'), dbus],
)
kf6_window = declare_dependency(
    include_directories: include_directories(kf6_include_prefix / 'KWindowSystem'),
    dependencies: [cpp_compiler.find_library('KF6WindowSystem'), dbus],
)
mauiman = dependency('MauiMan4', version: '>=4.0.5')
wayland = dependency('wayland-client')
xcb = dependency('xcb')
xcb_icccm = dependency('xcb-icccm')
xcb_shape = dependency('xcb-shape')
cpp_args = host_machine.cpu_family() == 'x86_64' ? ['-march=x86-64-v3', '-DBUNDLE_MAUI_STYLE'] : ['-DBUNDLE_MAUI_STYLE']

mauikit_version_header = configure_file(
    input: 'meson-content.in',
    output: 'mauikit_version.h',
    configuration: {'CONTENT': ('#pragma once\n#define MAUIKIT_VERSION_STRING "' + meson.project_version() + '"\n#define GIT_BRANCH "Stable"\n#define GIT_COMMIT_HASH "' + meson.project_version() + '"\n')}
)

wayland_scanner = find_program('wayland-scanner')
wayland_protocol = 'src/platforms/linux/blurhelper/ext-background-effect-v1.xml'
wayland_client_header = custom_target(
    'maui-wayland-client-header',
    input: wayland_protocol,
    output: 'wayland-ext-background-effect-v1-client-protocol.h',
    command: [wayland_scanner, 'client-header', '@INPUT@', '@OUTPUT@'],
)
wayland_private_code = custom_target(
    'maui-wayland-private-code',
    input: wayland_protocol,
    output: 'wayland-ext-background-effect-v1-protocol.c',
    command: [wayland_scanner, 'private-code', '@INPUT@', '@OUTPUT@'],
)
qtwaylandscanner = find_program('qtwaylandscanner', dirs: ['/usr/lib/qt6/libexec', '/usr/lib/qt6/bin'])
protocol_header = custom_target('maui-wayland-header', input: wayland_protocol, output: 'qwayland-ext-background-effect-v1.h', command: [qtwaylandscanner, 'client-header', '@INPUT@'], capture: true, depends: [wayland_client_header])
protocol_code = custom_target('maui-wayland-code', input: wayland_protocol, output: 'qwayland-ext-background-effect-v1.cpp', command: [qtwaylandscanner, 'client-code', '@INPUT@'], capture: true, depends: [wayland_client_header])
all_cpp = run_command('find', 'src', '-type', 'f', '-name', '*.cpp', check: true).stdout().strip().split('\n')
lib_cpp = []
foreach source : all_cpp
    if not source.endswith('mauikit.cpp') and not source.endswith('pluginmanager.cpp') and not source.endswith('mauistyleplugin.cpp')
        lib_cpp += [source]
    endif
endforeach
export_header = configure_file(
    input: 'meson-content.in',
    output: 'mauikit_export.h',
    configuration: {'CONTENT': ('#pragma once\n#define MAUIKIT_EXPORT __attribute__((visibility("default")))\n#define MAUIKIT_NO_EXPORT __attribute__((visibility("hidden")))\n')}
)
core_moc_headers = ['src/libs/controls.h', 'src/libs/style.h', 'src/platforms/abstractplatform.h', 'src/platforms/linux/mauilinux.h', 'src/platforms/linux/blurhelper/windowblur.h', 'src/platforms/linux/shadowhelper/windowshadow.h', 'src/platforms/linux/windowhelper.h', 'src/platforms/platform.h', 'src/utils/appsettings.h', 'src/utils/basictheme_p.h', 'src/utils/csdcontrols.h', 'src/utils/fontpickermodel.h', 'src/utils/handy.h', 'src/utils/icon.h', 'src/utils/imagecolors.h', 'src/utils/colorutils.h', 'src/utils/mauiapp.h', 'src/utils/platformtheme.h', 'src/utils/model_template/mauilist.h', 'src/utils/model_template/mauimodel.h', 'src/utils/notify.h', 'src/utils/wheelhandler.h']
core_moc = qt6.preprocess(moc_headers: core_moc_headers, moc_sources: ['src/utils/platformtheme.cpp', 'src/platforms/linux/blurhelper/windowblur.cpp'], dependencies: [qt])
assets_qt = qt6.preprocess(qresources: ['src/assets.qrc'])
lib = shared_library('MauiKit4', lib_cpp + core_moc + assets_qt + [export_header, mauikit_version_header, wayland_client_header, wayland_private_code, protocol_header, protocol_code], dependencies: [qt, kf6_core, kf6_i18n, kf6_config, kf6_notifications, kf6_window, mauiman, wayland, xcb, xcb_icccm, xcb_shape], include_directories: [include_directories('src', 'src/utils', 'src/utils/model_template', 'src/libs', 'src/platforms', 'src/platforms/linux'), qt_private_include], cpp_args: cpp_args, version: '4.0.5', soversion: '4', install: true)
plugin_moc = qt6.preprocess(moc_sources: ['src/mauikit.cpp'], moc_headers: ['src/mauikit.h'], dependencies: [qt])
plugin = shared_module('MauiKit4plugin', ['src/mauikit.cpp'] + plugin_moc, link_with: lib, dependencies: [qt, kf6_core, kf6_i18n, mauiman], include_directories: include_directories('src'), cpp_args: cpp_args, install: true, install_dir: get_option('libdir') / 'qt6/qml/org/mauikit/controls')
qml_dir = get_option('libdir') / 'qt6/qml/org/mauikit/controls'
install_data(configure_file(
    input: 'meson-content.in',
    output: 'qmldir',
    configuration: {'CONTENT': ('module org.mauikit.controls 1.3\nplugin MauiKit4plugin\n')}
), install_dir: qml_dir)
install_subdir('src/controls.6', install_dir: qml_dir, strip_directory: true)
install_subdir('src/csd.6', install_dir: get_option('datadir') / 'org.mauikit.controls', strip_directory: true)
style_qml_dir = get_option('libdir') / 'qt6/qml/org/mauikit/style'
style_plugin_source = configure_file(
    input: 'meson-content.in',
    output: 'maui_style_plugin.cpp',
    configuration: {'CONTENT': ('''#include <QQmlExtensionPlugin>
#include <QtQml/qqmlextensioninterface.h>

class MauiStylePlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *) override {}
};

#include "maui_style_plugin.moc"
''')}
)
style_plugin_moc = qt6.preprocess(moc_sources: [style_plugin_source], dependencies: [qt])
style_plugin = shared_module('MauiStyle', [style_plugin_source, style_plugin_moc], dependencies: [qt], cpp_args: cpp_args, install: true, install_dir: style_qml_dir)
install_data(configure_file(
    input: 'meson-content.in',
    output: 'style_qmldir',
    configuration: {'CONTENT': ('''module org.mauikit.style
plugin MauiStyle
depends QtQuick.Controls 2.15
depends org.mauikit.controls
designersupported
''')}
), install_dir: style_qml_dir, rename: 'qmldir')
style_qml_files = run_command('find', 'src/style.6/qtquickcontrols', '-maxdepth', '1', '-type', 'f', '-name', '*.qml', check: true).stdout().strip().split('\n')
style_private_qml_files = run_command('find', 'src/style.6/qtquickcontrols/private', '-maxdepth', '1', '-type', 'f', '-name', '*.qml', check: true).stdout().strip().split('\n')
install_data(style_qml_files, install_dir: style_qml_dir)
install_data(style_private_qml_files, install_dir: style_qml_dir / 'private')
install_headers(files('src/platforms/abstractplatform.h', 'src/platforms/linux/mauilinux.h', 'src/platforms/linux/blurhelper/windowblur.h', 'src/utils/fmh.h', 'src/utils/handy.h', 'src/utils/notify.h', 'src/utils/mauiapp.h', 'src/utils/imagecolors.h', 'src/utils/colorutils.h', 'src/utils/appsettings.h', 'src/utils/model_template/mauilist.h', 'src/utils/moduleinfo.h') + [export_header], subdir: 'MauiKit4/Core')
pkg = import('pkgconfig')
pkg.generate(lib, name: 'MauiKit 4', filebase: 'MauiKit4', version: meson.project_version(), description: 'MauiKit controls and utilities', requires: 'Qt6Core Qt6Gui Qt6Qml Qt6Quick Qt6QuickControls2')

cmake_targets = configure_file(
    input: 'meson-content.in',
    output: 'MauiKit4Targets.cmake',
    configuration: {'CONTENT': (('''get_filename_component(_MAUIKIT_PREFIX "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
if(NOT TARGET MauiKit4)
  add_library(MauiKit4 SHARED IMPORTED)
  set_target_properties(MauiKit4 PROPERTIES
    IMPORTED_LOCATION "${_MAUIKIT_PREFIX}/@LIBDIR@/libMauiKit4.so.4.0.5"
    INTERFACE_INCLUDE_DIRECTORIES "${_MAUIKIT_PREFIX}/include/MauiKit4/Core")
endif()
if(NOT TARGET MauiKit4::Controls)
  add_library(MauiKit4::Controls ALIAS MauiKit4)
endif()
if(NOT TARGET MauiKit4::MauiKit)
  add_library(MauiKit4::MauiKit ALIAS MauiKit4)
endif()
if(NOT TARGET MauiKit)
  add_library(MauiKit ALIAS MauiKit4)
endif()
''')).replace('@LIBDIR@', (get_option('libdir')))}
)
cmake_config = configure_file(
    input: 'meson-content.in',
    output: 'MauiKit4Config.cmake',
    configuration: {'CONTENT': ('''include(CMakeFindDependencyMacro)
find_dependency(Qt6 6.4 COMPONENTS Core)
include("${CMAKE_CURRENT_LIST_DIR}/MauiKit4Targets.cmake")
set(MauiKit4_FOUND TRUE)
foreach(_component IN LISTS MauiKit4_FIND_COMPONENTS)
  find_package(MauiKit${_component}4 CONFIG QUIET)
  if(MauiKit${_component}4_FOUND)
    set(MauiKit4_${_component}_FOUND TRUE)
  else()
    set(MauiKit4_${_component}_FOUND FALSE)
    set(MauiKit4_FOUND FALSE)
  endif()
endforeach()
''')}
)
cmake_version = configure_file(
    input: 'meson-content.in',
    output: 'MauiKit4ConfigVersion.cmake',
    configuration: {'CONTENT': (('''set(PACKAGE_VERSION "@VERSION@")
if(PACKAGE_FIND_VERSION VERSION_LESS_EQUAL PACKAGE_VERSION)
  set(PACKAGE_VERSION_COMPATIBLE TRUE)
endif()
if(PACKAGE_FIND_VERSION VERSION_EQUAL PACKAGE_VERSION)
  set(PACKAGE_VERSION_EXACT TRUE)
endif()
''')).replace('@VERSION@', (meson.project_version()))}
)
install_data([cmake_targets, cmake_config, cmake_version], install_dir: get_option('libdir') / 'cmake' / 'MauiKit4')
