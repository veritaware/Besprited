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

# packaging/macos_dmg_background.png has no separate @1x asset, so the window
# is sized to the image's native 1280x800 pixels rather than relying on
# dmgbuild/Finder to infer a @2x scale factor.
window_rect = ((100, 100), (1280, 800))
icon_size = 128
icon_locations = {
    appname: (260, 420),
    'Applications': (1020, 420),
}

show_status_bar = False
show_tab_view = False
show_pathbar = False
show_sidebar = False
show_icon_preview = False
default_view = 'icon-view'
include_icon_view_settings = 'auto'
