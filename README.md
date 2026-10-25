# S.I.M.P.

Compiles with [Saturn Rlng Library](https://github.com/ReyeMe/SaturnRingLib).

## Supported Formats

[VFS](https://github.com/John-Spier/VFSTool) filetypes are all added to 0xFFFFFF00. Other files in a VFS will be loaded at that position into the main memory.

### SSF (0x18)

Used in VFS files created by [xsfext](https://github.com/John-Spier/xsfext) to load extracted SSF/miniSSF files into sound RAM.

### SSF2 (0x23)

Used in VFS files created by xsfext to load Sega driver files into sound RAM.

### MOD (0x1B)

Standard MOD file, plays using [DamageX's VGM driver](https://github.com/vbt1/SndPlayEX).

### VGM (0x1C)

VGM file, plays using DamageX's VGM driver. Supports single/dual YM2203, YM2612+SN76489, YM2151, and YM2608.

### MDX (0x1D)

X68000 MDX file, plays using DamageX's VGM driver.

### VGM68.BIN/DRV (0x1A/0x20)

DamageX's VGM driver, can be reloaded. VGM68.BIN is automatically stored in memory.

### AIF (0x1E)

Standard uncompressed AIFF file. Can be 8 or 16 bit, mono or stereo.

### ADP (0x1F)

4-bit XA-ADPCM file with custom headers. Can be encoded with [psxavenc-adp](https://github.com/John-Spier/psxavenc-adp) or converted from other XA-ADPCM formats with [adptool](https://github.com/John-Spier/adptool).

### SEQ (0x21)

Sequenced music format, can be created with Sega tools or [mid2seq](https://github.com/John-Spier/mid2seq). Needs to have a TON file loaded to play.

### TON (0x22)

Music sequence sound bank, can be created with Sega tools or mid2seq.

### EXB (0x24)

SCSP DSP program.

### DSP (0x25)

SCSP DSP program RAM.

### MAP (0x26)

Sound RAM map for Sega sound driver.

### TSK (0x27)

Various versions of the Sega sound driver.

### DA (0x07)

CD audio track.

### CPK (0x26)

Cinepak video file. Can be converted from Cinepak MOV using various tools. Controls in player: B=pause, C=quit, X=prev, Z=next, Y=restart, Start=toggle 16/32-bit, D-pad=move, L/R=volume.

### VFS (0xFE)

Archive of various files, needed for playing SSF files without extraction. Can be created with VFSTool or xsfext.

### VF2 (0xED)

Archive of various files, with only SSF2 files shown on the list. Can be created with xsfext.
