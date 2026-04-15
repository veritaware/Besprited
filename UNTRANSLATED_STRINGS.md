# Curated List of Untranslated Strings in Besprited

This list contains string literals that are likely user-facing and should be internationalized using the `i18n()` function.

## Summary

- **Total user-facing strings found:** 220+ (updated after review)
- **Excluded internal strings:** 63 (serialization identifiers, enum values, debug messages, etc.)
- **Update Note:** Added 25+ strings missed in initial analysis (Alert::show, MenuItem declarations, member initializations)

---

## Strings That Need Internationalization

### Alert Messages (1 string)
Alert dialog messages not using i18n():
- `base/launcher.cpp:54`: "Problem<<Cannot open:<<%s<<%s||&Close" (Windows-specific file open error)

### Filter Names (5 strings)
These are displayed in the UI when filters are applied:
- `filters/convolution_matrix_filter.cpp`: "Convolution Matrix"
- `filters/median_filter.cpp`: "Median Blur"
- `filters/invert_color_filter.cpp`: "Invert Color"
- `filters/replace_color_filter.cpp`: "Replace Color"
- `filters/color_curve_filter.cpp`: "Color Curve"

### View/Tab Names (3 strings)
These appear as tab titles in the UI:
- `app/ui/data_recovery_view.cpp`: "Data Recovery"
- `app/ui/home_view.cpp`: "Home"
- `app/ui/devconsole_view.cpp`: "Console"

### Window Titles (4 strings)
Popup window and dialog titles:
- `app/ui/palette_popup.cpp`: "Palettes"
- `app/ui/font_popup.cpp`: "Fonts"
- `app/ui/configure_timeline_popup.cpp`: "Timeline Settings"

### UI Labels (20+ strings)
Text labels in various dialogs and UI elements:
- `app/commands/cmd_mask_by_color.cpp`: "Color:", "Tolerance:"
- `app/ui/status_bar.cpp`: "Frame:"
- `app/ui/context_bar.cpp`: "Pick:", "Sample:", "Tolerance:", "Opacity:", "Spray:", "Freehand:"
- `app/commands/cmd_about.cpp`: "Animated sprite editor & pixel art tool", "Authors:", "- Lead developer, graphics & maintainer", "- Default skin & graphics introduced in v0.8"
- `app/commands/cmd_sprite_properties.cpp`: "(only for indexed images)"
- `app/ui/color_popup.cpp:57`: "Transparent Color Selected" (m_maskLabel member)
- `app/ui/devconsole_view.cpp:64`: "Welcome to Besprited Scripting Console\n(Experimental)" (m_textBox member)
- `app/ui/devconsole_view.cpp:65`: ">" (m_label member, likely doesn't need i18n)
- `app/ui/hex_color_entry.cpp`: "#" (m_label member, likely doesn't need i18n)

### Buttons (15+ strings)
Button labels throughout the UI:
- `app/commands/cmd_mask_by_color.cpp`: "&OK", "&Cancel"
- `app/commands/cmd_about.cpp`: "&Close"
- `app/console.cpp`: "&Close", "C&lear"
- `app/ui/status_bar.cpp`: "+" (New Frame button), "Disable Snap to Grid" (m_button member)
- `app/commands/filters/filter_window.cpp`: "&OK", "&Cancel" (m_okButton, m_cancelButton members)
- `app/ui/color_bar.cpp`: "Remap" (m_remapButton member)
- `app/ui/data_recovery_view.cpp` (lines 46-47) - Conditional initialization:
  - "Open" / "Open All" (m_openButton member)
  - "Delete" / "Delete All" (m_deleteButton member)

### Checkboxes (6 strings)
Checkbox labels:
- `app/commands/cmd_mask_by_color.cpp`: "&Preview"
- `app/ui/context_bar.cpp`: "Contiguous", "Pixel-perfect", "Auto Select Layer"
- `app/commands/filters/filter_window.cpp`: "&Tiled", "&Preview" (m_tiledCheck, m_showPreview members)

### Menu Items (30+ strings)
Menu and context menu items:
- `app/ui/editor/editor.cpp`: "Play Once", "Rewind on Stop"
- `app/app_menus.cpp`: "WARNING!", "You should update your customized gui.xml file to the new version to get", "the latest commands available.", etc.
- `app/ui/brush_popup.cpp` (lines 158-161):
  - "Save Brush Here"
  - "Locked"
  - "Delete"
  - "Delete All"
- `app/ui/color_wheel.cpp` (lines 320-328) - Color harmony options:
  - "Discrete"
  - "Without Harmonies"
  - "Complementary"
  - "Monochromatic"
  - "Analogous"
  - "Split-Complementary"
  - "Triadic"
  - "Tetradic"
  - "Square"
- `app/ui/data_recovery_view.cpp` (lines 102-103):
  - "Raw Images as Frames"
  - "Raw Images as Layers"
- `ui/entry.cpp` (lines 787-789) - Text entry context menu:
  - "Cut"
  - "Copy"
  - "Paste"

### Combo Box Items (90+ strings)
Dropdown menu options across many dialogs:
- Color types: "RGB", "HSB", "Index", "Gray", "Mask"
- Blend modes: "Normal", "Multiply", "Screen", "Overlay", "Darken", "Lighten", "Color Dodge", "Color Burn", "Hard Light", "Soft Light", "Difference", "Exclusion", "Hue", "Saturation", "Color", "Luminosity"
- Channel options: "Color+Alpha", "Color", "Alpha", "RGB+Alpha", "RGB", "HSB+Alpha", "HSB", "Gray+Alpha", "Gray", "Best fit Index"
- Layer sampling: "All Layers", "Current Layer"
- Sheet types: "Horizontal Strip", "Vertical Strip", "By Rows", "By Columns"
- Frame/layer options: "All frames", "Selected frames", "Visible layers", "Selected layers"
- Scaling methods: "Nearest-neighbor", "Bilinear", "RotSprite"
- Right-click behaviors: "Paint with background color", "Pick foreground color", "Erase", "Scroll"
- Grid scopes: "Global", "Current Document"
- Background sizes: "16x16", "8x8", "4x4", "2x2"
- Animation directions: "Forward", "Reverse", "Ping-pong"
- Change modes: "Abs", "Rel"
- Pattern alignment: "Pattern aligned to source", "Pattern aligned to destination", "Paint brush"
- Filter target buttons: "R", "G", "B", "A", "K", "Index"

### Tooltips (30+ strings)
Tooltip text for various UI elements in context bar, color bar, status bar, etc.:
- `app/ui/context_bar.cpp`: "Brush Type", "Brush Size (in pixels)", "Brush Angle (in degrees)", "Ink", "Opacity (paint intensity)", "Shades", "Spray Width", "Spray Speed", "Rotation Pivot", "Transparent Color", "Rotation Algorithm", "Freehand trace algorithm", "Extra paint bucket options", "Horizontal Symmetry", "Vertical Symmetry", "Replace selection", "Add to selection\n(Shift)", "Subtract from selection\n(Shift+Alt)", "Drop pixels here", "Cancel drag and drop"
- `app/ui/color_bar.cpp`: "Foreground color", "Background color", "Add foreground color to the palette", "Add background color to the palette", "Edit Color", "Sort & Gradients", "Presets", "Options", "Matches old indexes with new indexes"
- `app/ui/status_bar.cpp`: "Current Frame", "Zoom Level", "New Frame"
- `app/ui/select_accelerator.cpp`: "Also known as Windows key, logo key,\ncommand key, or system key."

### Separators (3 strings)
Menu separator labels:
- `app/ui/brush_popup.cpp`: "Saved Parameters", "Parameters to Save"
- `app/commands/cmd_about.cpp`: "Authors:"

### Status Bar Messages (15+ strings)
Messages displayed in the status bar:
- `app/commands/cmd_goto_layer.cpp`: "Layer `%s' selected"
- `app/commands/cmd_save_file.cpp`: "File %s, saved."
- `app/ui/document_view.cpp`: "Sprite '%s' closed."
- `app/ui/timeline.cpp`: "%s cels", "%s before frame %d", "%s after frame %d", "Onionskin is %s", "Layer '%s' [%s%s]", "Layer '%s' is %s", "Layer '%s' is %s (%s)"
- `app/app.cpp`: "%s | %s" (showing PACKAGE_AND_VERSION and COPYRIGHT)

### Widget Text Updates (10+ strings)
Dynamic text set on widgets:
- `app/commands/cmd_palette_editor.cpp`: " No Entry", " Multiple Entries"
- `app/commands/cmd_color_quantization.cpp`: "256"
- `app/commands/cmd_layer_properties.cpp`: "No Layer"
- `app/commands/cmd_modify_selection.cpp`: "Width:"
- `app/commands/cmd_export_sprite_sheet.cpp`: "Select File: " + filename
- `app/commands/cmd_keyboard_shortcuts.cpp`: "Add"
- `app/ui/context_bar.cpp`: "Select colors in the palette"
- `app/ui/brush_popup.cpp`: "Save Brush"
- `app/ui/file_selector.cpp`: "-------- Recent Paths --------"

### Ink Types (6 strings)
Display names for ink types shown in UI:
- `app/tools/ink_type.cpp`: "Simple Ink", "Alpha Compositing", "Copy Color+Alpha", "Lock Alpha", "Shading", "Unknown"

### Context Bar Help Text (3 strings)
Help text shown in the context bar:
- `app/commands/cmd_new_brush.cpp`: "Select brush bounds | Right-click to cut"
- `app/commands/cmd_canvas_size.cpp`: "Select new canvas size"
- `app/commands/cmd_import_sprite_sheet.cpp`: "Select bounds to identify sprite frames"

### Action Names (4 strings)
Names for undo/redo actions:
- `app/commands/cmd_modify_selection.cpp`: "Border", "Expand", "Contract", "Modify"

### Notification Text (1 string)
Text shown in notifications:
- `app/send_crash.cpp`: "Report last crash"

### Error Messages (2 strings)
User-facing error messages:
- `base/errno_string.cpp`: "Unknown error"
- `app/ui/keyboard_shortcuts.cpp`: "Unknown"

---

## Excluded Internal Strings (Not User-Facing)

These strings were identified as internal and do NOT need internationalization:

### Serialization/API Identifiers (37 strings)
- Blend mode identifiers: "normal", "multiply", "screen", etc. (lowercase, for file format)
- Animation direction: "forward", "reverse", "pingpong"
- Brush types: "circle", "square", "line", "image", "unknown"
- Ink type identifiers: "simple", "alpha_compositing", "copy_color", "lock_alpha", "shading", "unknown"
- Default palette identifier: "default"
- Platform identifiers: "emscripten", "windows", "macos", "android", "linux"

### Debug/Internal Messages (26 strings)
- Message type enum names: "Open", "Close", "Paint", "Timer", "KeyDown", etc.
- JavaScript object representation: "[Object object]"
- File system paths: "/tmp", "/"

---

## Notes

1. All strings listed in the "Strings That Need Internationalization" section should be wrapped with `i18n()`.
2. The excluded strings are used for serialization, APIs, or internal purposes and should remain as plain strings.
3. Some combo box items contain technical color channel names (R, G, B, A, K) which may or may not need translation depending on localization policy.
4. Status bar format strings include printf-style placeholders (%s, %d) which should be preserved in translations.

---

## Update History

### Update 2026-04-15
Added 25+ strings that were missed in the initial analysis:

**New Categories/Strings Added:**
1. **Alert::show calls** (1 string):
   - `base/launcher.cpp`: Windows file open error message

2. **MenuItem declarations** (18 strings):
   - `app/ui/brush_popup.cpp`: 4 brush menu items
   - `app/ui/color_wheel.cpp`: 9 color harmony menu items
   - `app/ui/data_recovery_view.cpp`: 2 import options (already partially listed)
   - `ui/entry.cpp`: 3 text entry context menu items (Cut, Copy, Paste)

3. **Member variable initializations** (6+ strings):
   - `app/ui/data_recovery_view.cpp`: m_openButton, m_deleteButton (conditional strings)
   - `app/ui/color_popup.cpp`: m_maskLabel
   - `app/ui/devconsole_view.cpp`: m_textBox welcome message, m_label
   - `app/ui/status_bar.cpp`: m_button text
   - `app/commands/filters/filter_window.cpp`: m_showPreview
   - `app/ui/hex_color_entry.cpp`: m_label

These patterns were identified through additional analysis focusing on:
- Member variable initializations (`: m_variable("text")` pattern)
- Local MenuItem/AppMenuItem declarations
- Alert::show() calls without i18n() wrappers

