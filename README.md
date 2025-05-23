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
| Mouse wheel | Scale up/down. Combinating `Ctrl` to retain window size. |
| Left button + mouse wheel | Speed up/down the animation. |
| Left button drag | Move view-point. |
| Middle button | Reset scale, speed, and view-point to default. |
| Right button + mouse wheel | Fast forward/rewind the text. |
| Right button + middle button | Show/hide windows's frame and title. |

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
 
- When `0069_03h` is opened, `∨` key starts playing `0071_03h`, and `∧` key `0068_03h`.

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

1. Run `src/deps/CMakeLists.txt` to obtain the external libraries. 
2. Build `spine-cpp.lib` with official [CMakeLists.txt](https://github.com/EsotericSoftware/spine-runtimes/blob/4.1/spine-cpp/CMakeLists.txt).
3. Move built static library files to `src/deps/spine-cpp-4.1/lib`.
4. Open `KarinchanPlayer.sln` with Visual Studio 2022.
5. Select `Build Solution` on menu items.
