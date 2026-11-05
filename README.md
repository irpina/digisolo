# SOLO & MUTE GROUPS, Digitone mk1

Solo and four mute groups, on the stock global mutes (the ones FUNC + a
track key toggles), for the Digitone mk1 and Digitone Keys:

- **Solo: hold a track key, then press FUNC.** The track plays alone among
  its four (the synth tracks, or the MIDI tracks when the track keys are on
  them): the other three are muted and it is unmuted. The same again (that
  track key, then FUNC) gives the four their mutes back as they were before
  the solo; another track's solo moves the solo there. Press FUNC soon after
  the track key: a track key held on its own for about 0.4 s opens core's
  Mod Menu first. FUNC first and then a track key is the stock mute, as ever,
  and a mute set that way during a solo ends the solo where it is (nothing is
  given back).
- **SOLO** in the Mod Menu (hold a track key) solos the held track the same
  way.
- **MUTE GROUPS** in the Mod Menu opens RECALL 1-4 and STORE 1-4. STORE n
  keeps the mutes of all eight tracks as group n; RECALL n sets them (a group
  never stored does nothing). The groups are saved with the project.

Each action shows a word in the screen's corner for a second (SOLO T2,
UNSOLO, STORED 1, GROUP 1, EMPTY 3).

For OS 1.43 and 1.44, on [core-dn1](https://github.com/irpina/elekloader/tree/main/mods/core-dn1) 5.0.1. It names no
firmware address: its 1.44 port is empty, and both builds carry the same
code.

## Install

Not in elekloader's shop yet. You need:
- **core-dn1 5.0.1,** which is not released yet: build it from elekloader's source
  (`mods/core-dn1`, its README says how) with the stock OS file. Older cores refuse this mod.
- **elekloader from source with core-dn1 5.0.1** (its `digitone-mk1/core3.h` declares what this
  mod calls), with Python 3.9 or newer, to build the mod and the OS.
- **This mod:** `solo-0.1.elemod` for OS 1.43, `solo-0.1-os1.44.elemod` for 1.44, from
  [this repository's releases](https://github.com/irpina/digisolo/releases) once there are any, or built
  from this folder (Build, below).
- **The stock OS file:** `Digitone_and_Digitone_Keys_OS1.44.syx` or `..._OS1.43.syx`, from
  [Elektron's Digitone downloads](https://www.elektron.se/support-downloads/digitone).
  elekloader recognises the file by its hash.

Build your OS (or in elekloader's window, run from source: **+ Install from file...** both files):

```bash
python -m elekloader.patch --stock Digitone_and_Digitone_Keys_OS1.44.syx \
    --mod core-5.0.1-os1.44.elemod --mod solo-0.1-os1.44.elemod \
    --out Digitone_OS1.44-solo.syx --version XXXX
```

`--version` is the 4 characters the unit will show. Flash the `.syx` with
Elektron Transfer, as for any OS update: connect the unit over USB, select
it in Transfer and **Connect**, drag the `.syx` onto **Drop files here**,
and press **YES** on the unit. Don't turn it off until the upgrade is done.

**Recovery:** elekloader never changes the bootloader, so the stock OS
file always restores the unit. Hold **FUNC** while powering on for the
startup menu, and press **TRIG 4** for OS UPGRADE. Then send the stock
`.syx` with Transfer.

## How

Stock combos with a track key: FUNC + a track key mutes, a double press
opens the sound browser, a held track key alone opens the Mod Menu (core),
a held track key + NO reloads its sound. A track key held and then FUNC does
nothing in the stock (the FUNC overlay opens and closes), so that is the
solo. The mod watches the track keys (`ev_key`), and when FUNC goes down
while one is held it solos the active track (the key selected it) and takes
FUNC's press, repeats and release, so the overlay does not flash. The mutes
are set with core 5.0.1's `core_mute_set`, the stock's own mute setter (the
one FUNC + a track key calls), and read with `core_mutes`; `ev_mute` tells
it when the stock sets one. The groups are core project data (`MUTG`, 20
bytes).

## Build

```bash
python -m elekloader.sdk.build . --stock Digitone_and_Digitone_Keys_OS1.43.syx   # out/solo-0.1.elemod
python -m elekloader.sdk.build . --stock Digitone_and_Digitone_Keys_OS1.44.syx   # out/solo-0.1-os1.44.elemod
python -m elekloader.lint out/solo-0.1.elemod --stock Digitone_and_Digitone_Keys_OS1.43.syx --with core-5.0.1.elemod
```

## What was checked

In digikit's emulator (digiemu, headless `Session`), with core-dn1 5.0.1,
LOOPBACK, PC SOUND and a probe mod logging `ev_mute` (and the mutes after
each) in the same build, on 1.43 and on 1.44:

- T2 held, FUNC: T1, T3 and T4 muted (`core_mute_set` through the stock
  setter, so `ev_mute` saw each with where 0), the corner showed SOLO T2, and
  no FUNC overlay opened. The same again: the three unmuted, as before.
- Stock FUNC + T3 muted T3; MUTE GROUPS > STORE 1; FUNC + T3 and FUNC + T4
  (T3 open, T4 muted); MUTE GROUPS > RECALL 1: T3 muted and T4 open again,
  the corner showing GROUP 1; RECALL 4, never stored, changed nothing.
- The Mod Menu's grid shows SOLO and MUTE GROUPS with their icons.

`lint` with core-dn1 5.0.1 (`--with core-5.0.1.elemod`) says it patches no site and links, on
1.43 and 1.44, with the same code on both; with core-dn1 5.0 it is refused: it needs 5.0.1.

Not checked: the audio of a solo (the mutes were read back, not heard), the
MIDI tracks' solo, the groups saved and loaded with a project, the pattern
and song-row mute modes (this mod works on the global mutes only), the
Digitone Keys, and anything on hardware.

## Licence

GPL-2.0-or-later, as elekloader's core it builds on (LICENSE).
