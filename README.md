# KarinchanPlayer
某寝室用

## How to play

Select a folder such as the following from application menu `File->Open folder`.

<pre>
...
├ 0069_03h
│  ├ 01_a_kouimae.png        // Still image
│  ├ ...
│  ├ 04_a_jigo.png
│  ├ 0069_03h_machi_001.m4a  // Voice file
│  ├ ...
│  ├ 0069_03h_machi_100.m4a
│  ├ animation.atlas.txt     // Spine atlas
│  ├ animation.json          // Spine skeleton
│  ├ animation.png           // Spine texture
│  ├ animation_2.png
│  └ script.txt              // Scenario script
└ ...
</pre>

The scene will be set up based on the description of `script.txt` using resource files in the same folder.

## Mouse functions

| Input | Function |
| --- | --- |
| Wheel scroll | Scale up/down. Combinating `Ctrl` to retain window size. |
| L-pressed + mouse wheel | Speed up/down the animation. |
| L-drag | Move view-point. |
| M-click | Reset scale, speed, and view-point to default. |
| R-click | Show context menu. |
| R-pressed + wheel scroll | Fast-forward/rewind the text. |
| R-pressed + M-click | Show/hide windows's frame and title. |

## Keyboard functions

| Input | Function |
| --- | --- |
| <kbd>C</kbd> | Toggle text colour between black and white. |
| <kbd>T</kbd> | Show/hide text. |
| <kbd>Esc</kbd> | Close the application. |
| <kbd>∧</kbd> | Open the next folder. |
| <kbd>∨</kbd> | Open the previous folder. |
| <kbd>＞</kbd> | Fast-forward the text. |
| <kbd>＜</kbd> | Rewind the text. |

<details><summary>Navigation example</summary>
 
- When `0069_03h` is opened, `∨` key opens `0071_03h`, and `∧` key `0068_03h`.

<pre>
...
├ 0068_03h
│  └ ...
├ 0069_03h
│  └ ...
├ 0071_03h
│  └ ...
└ ...
</pre>

</details>

## External libraries

- [DxLib](https://dxlib.xsrv.jp)
- [spine-cpp-4.1](https://github.com/EsotericSoftware/spine-runtimes/tree/4.1)

## Build

1. Open `src/deps` folder with Visual Studio 2022.
    - CMake configuration downloads external libraries.
2. Install Spine generic runtime both for `x64-Debug` and for `x64-Release`.
3. Open `KarinchanPlayer.sln`.
4. Select `Build Solution` on menu items.
