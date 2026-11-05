# TOTK S23

[Português](README.md) | **English**

An **unofficial** modification of [TOTKDroid](https://github.com/mamcjr/TOTKDroid), tuned for the **Samsung Galaxy S23
Ultra** (Snapdragon 8 Gen 2, Adreno 740).

> [!IMPORTANT]
> **The original project is by [mamcjr](https://github.com/mamcjr), creator of
> [TOTKDroid](https://github.com/mamcjr/TOTKDroid), built on [peiikai](https://github.com/peiikai)'s LAS Recomp. All
> credit for the project goes to them.**
>
> This version is a personal modification made by someone else, with help from AI. mamcjr took no part in these
> changes, didn't test them and isn't responsible for them.

> [!CAUTION]
> **A problem with this version? Open an [issue here](https://github.com/Moab-tk/TOTKDroid-S23/issues). Never take these
> complaints to the original project.** A bug in this version may have been caused by these changes and may not exist in
> the original. It may work well on the tester's phone and badly on yours.

> [!NOTE]
> Don't take any of this too seriously. If it doesn't work on your phone, just let it go and use whichever version you
> like best.

> [!WARNING]
> **The test phone is modified.** It's a 12 GB S23 Ultra with an overclock, the system's thermal limiter turned off by
> third-party tools, other changes and a cooler. No number on this page is a promise for a regular phone.

The original project's text, with all the technical details, is in [README-ORIGINAL.md](README-ORIGINAL.md) (and in
Portuguese, [README-ORIGINAL.pt-BR.md](README-ORIGINAL.pt-BR.md)). The list of changes is in
[MUDANCAS-S23.md](MUDANCAS-S23.md) (Portuguese).

## No game files

**No game files are included.** You need your own legally obtained dump of *Tears of the Kingdom* with update **1.4.3**
applied (`exefs` and `romfs`). No instructions on how to get it will be given.

## What's different in this version

- **Built for the S23's processor**, without the instructions it lacks.
- **Installs next to the original:** the package is different (`com.totkdroid.s23`), so one doesn't replace the other.
- **Based on version 0.2.1 of the original**, the newest so far.
- **Brought over from the S23 version of BOTW** ([BOTWRecompAndroid-S23](https://github.com/Moab-tk/BOTWRecompAndroid-S23)),
  which uses the same engine:
  - steady 30 FPS frames, with the screen at 60 Hz and each frame following its real rhythm;
  - a 30 FPS budget told to Android, so it doesn't lower the processor too far;
  - shaders: whoever waits does the job, larger tables and pipelines already seen built ahead;
  - five Turnip drivers inside the app, one-tap recommended settings and 8 saved settings blocks;
  - a larger interface, in cards;
  - a code review: waits on the right clock, settings written safely, checks in the kernel and the render thread.
- **Temperature control (optional, off by default):** lowers the scene in steps before the system cuts the GPU. In b4 it
  caused shards on the edges and a blown-out sun; b5 fixes the reason (see [MUDANCAS-S23.md](MUDANCAS-S23.md)).
- **Camera drag sensitivity up to 800%.**

## Installing

You need Android 11 or newer, an arm64 phone with Vulkan and room for the game files.

1. Download the APK from this page's **Releases** tab and install it.
2. Open it and pick the folder with your 1.4.3 `exefs` and `romfs`. The game runs as native code: the app doesn't build
   the game.
3. On the start screen, open **Saved settings** and tap **Use recommended settings**.
4. Confirm. The app restarts and the game is ready to play.

Updating from an earlier version of this modification, just install over it: saves and settings stay.

## Recommended settings

| Option | S23 Ultra | S23, S23+ and weaker |
|---|---|---|
| Frame limit | 30 FPS | 30 FPS |
| Internal resolution | 1x | 75% |
| Output resolution | 1080 lines | 900 lines |
| Upscaling | FSR 1 | FSR 1 |
| Anti-aliasing | Off | Off |
| Game code | Native | Native |
| Steady frames at 30 FPS | On | On |
| Temperature control | Off | Off |

The lighter option wasn't tested on the S23, the S23+ or weaker phones.

## Measured performance

Numbers from a single **modified** phone, game 1.4.3, native code, 30 FPS.

| Measurement | Settings | Average FPS | Notes |
|---|---|---:|---|
| b1, guided part | GPU held by the system | 25.5 | 67% of frames on time |
| b1, guided part | GPU released | 26.8 | 75% of frames on time |
| b4, 30 minutes playing | 1x, 1080 output, temperature control off | 27.7 | 71% of samples at 27 or more; 5% under 20 |
| b4, inside a shrine | the same | 29.8 | almost no drops |

In open, busy areas the game sends 4 to 5 thousand draws per frame. There the limit is the processor, not the GPU:
lowering the resolution helps little. In the same 30-minute session the system lowered the fastest core's cap from
3,360 to 1,555 MHz because of heat, and the FPS dropped with it.

## Known problems

- **b5 not tested by playing:** the fix for the temperature control's shards was built and checked, but not played
  yet. If shards show with it on, turn it off and report it.
- **Drops to 17-24 FPS in open areas:** the processor can't keep up with so many draws, and heat makes it worse. The
  next step is profiling where the render thread spends its time.
- **Memory:** on a 12 GB phone the game reached 9.2 GB in use and filled the swap in the first session. Phones with less
  memory may close the game.
- **Building ahead:** the first session after installing a new version only records the pipelines; the gain shows from
  the second one on.
- **Only tested on the S23 Ultra.** There's no Universal version of TOTK for now.

## Test options

`LARECOMP_*` switches only work with a `testes.txt` file in the app's folder, which only `adb` can create. Some useful
ones: `LARECOMP_SCALE_SNAP=0` (the scene scale as before b5), `LARECOMP_NO_PREWARM=1` (no building ahead),
`LARECOMP_LOG_HEAT=1` (a heat log line every 30 s).

## Support, if you want

A voluntary donation, with no benefit at all, to help with costs such as Claude's tokens. It goes to Moab, the author of
this modification, not to mamcjr, peiikai or Nintendo. Pix (random key): `5eb148c4-81dc-47cd-b2cc-b3171d3c06ab`

To support the original project, use [mamcjr's GitHub Sponsors](https://github.com/sponsors/mamcjr).

## Building

As in the original: see [README-ORIGINAL.md](README-ORIGINAL.md). The signing key lives outside the repository
(`~/.totkrecompandroid/keystore.properties`).

## Credits

- [mamcjr](https://github.com/mamcjr), for [TOTKDroid](https://github.com/mamcjr/TOTKDroid).
- [peiikai](https://github.com/peiikai), for LAS Recomp (MIT license, in [LICENSE](LICENSE)).
- All the original's credits still apply: see [README-ORIGINAL.md](README-ORIGINAL.md#credits).
- Turnip drivers: Mesa builds by [The412Banner](https://github.com/The412Banner/Banners-Turnip).
