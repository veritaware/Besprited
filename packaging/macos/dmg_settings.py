# dmgbuild settings for the Besprited "drag to install" DMG.
#
# Usage:
#   dmgbuild -Dapp=bundle/besprited.app -s packaging/macos/dmg_settings.py "Besprited" besprited.dmg
#
# `app` defaults to bundle/besprited.app so this also works when invoked
# without -D from the repo root (e.g. via packaging/package_macos.sh).

import os.path

application = defines.get('app', 'bundle/besprited.app')
appname = os.path.basename(application)

format = 'UDZO'
files = [application]
symlinks = {'Applications': '/Applications'}

# dmgbuild execs this file without setting `__file__` in its namespace, so
# the background path can't be derived from this script's own location.
# Both callers (the CI workflow and packaging/package_macos.sh) always invoke
# dmgbuild from the repo root with `-s packaging/macos/dmg_settings.py`, so a
# path relative to the current working directory works the same way `app`
# above does.
background = 'packaging/macos_dmg_background.png'

# packaging/macos_dmg_background.png (640x400) is the @1x background; the
# repo also ships packaging/macos_dmg_background@2x.png (1280x800, an exact
# 2x pixel-doubled render of the same art) alongside it. dmgbuild's
# @<scale>x naming convention makes it find that sibling automatically and
# combine both into a HiDPI TIFF, so Finder opens a window sized to the @1x
# image (crisp on both standard and Retina displays) instead of the much
# larger window you'd get from handing it the @2x file alone.
window_rect = ((100, 100), (640, 400))
icon_size = 100
icon_locations = {
    appname: (128, 210),
    'Applications': (512, 210),
}

show_status_bar = False
show_tab_view = False
show_pathbar = False
show_sidebar = False
show_icon_preview = False
default_view = 'icon-view'
include_icon_view_settings = 'auto'
