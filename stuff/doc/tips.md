# Welcome to Inkframe

Inkframe is a free 2D animation tool — frame-by-frame drawing, cutout/mesh rigging, and effects, with **no frame limits**. Built as an Aetherfall development tool. Copyright (C) 2026 Joseph McCormack; built on open-source software (Help → About → License).

### Quick start

- **Workspaces** (tabs at the top right): **Draw** — canvas + timeline for frame-by-frame · **Effects** — the FX node area · **Timing** — exposure sheet + curves · **Files** — browser.
- **Draw something**: `B` brush, `E` eraser. Step frames with `,` and `.` — play with `P` — onion skin with `/`.
- **Frame rate**: the FPS field lives in the playback bar under the viewer (and in Scene Settings). No frame cap — animate as long as you like.
- **Clip Studio hands?** Load the CSP-style keys: *File → Configure Shortcuts → preset "clipstudio" → Load* (adds `G` fill, `I` eyedropper, `M` select, `R` rotate canvas, `[` `]` brush size).

### Rigging (bones on your drawings)

Two tools in the left toolbar do it:

- **Skeleton tool** — classic cutout: put each body part on its own column (import a layered **PSD** to get that for free), then Skeleton tool → *Build Skeleton* mode to place bones → *Animate* or *Inverse Kinematics* mode to pose. Keyframes record automatically as you pose on different frames.
- **Plastic tool** — Spine-style mesh deformation: select a level column → Plastic tool → *Create Mesh* → *Build Skeleton* mode to place bones inside the mesh → *Animate* mode to bend the drawing smoothly.

### Importing your art

- *File → Load Level* for images or image sequences (name frames like `name.0001.png` so they group into one level).
- Layered PSD import splits layers into separate columns — ideal for rigging.
- Export video/GIF via *Render* (ffmpeg is bundled).
