# DirectX12 Game Project

DirectX 12を使ったC++製の2Dゲーム学習プロジェクトです。

現在は、タイトル画面からシューティングゲームへ遷移する構成になっています。レンダリング、入力、サウンド、シーン管理、Actor管理、スプライト描画、文字描画など、ゲームループに必要な基本機能がプロジェクト内にまとまっています。

## まず最初に

このプロジェクトを初めて触る場合は、次の順番で確認すると全体像をつかみやすいです。

1. `Main.cpp` でWindowsアプリの起動処理とメインループを見る
2. `Game.cpp` で初期化、入力、更新、描画、シーン遷移を見る
3. `Scene.h` / `Scene.cpp` でシーンの基本構造を見る
4. `ShootingTitleScene.cpp` でタイトル画面の流れを見る
5. `ShootingScene.cpp` でゲーム本編の生成、更新、描画を見る
6. `PlayerActor.cpp` や `EnemyActor.cpp` でActor単位の処理を見る
7. `Renderer.cpp` と `SoundSystem.cpp` は必要になったタイミングで詳しく読む

## 必要な環境

- Windows
- Visual Studio
- C++によるデスクトップ開発ワークロード
- Windows SDK
- DirectX 12に対応した実行環境
- DirectXTex

このプロジェクトはVisual StudioのC++プロジェクトです。`Game_DirectX12_Project.vcxproj` ではC++20を使用しています。

DirectXTexは次の固定パスを参照しています。

```text
C:\DirectXTex\DirectXTex
```

環境が異なる場合は、Visual Studioのプロジェクト設定、または `Game_DirectX12_Project.vcxproj` の `IncludePath` / `LibraryPath` を自分の環境に合わせて調整してください。

## セットアップ手順

1. このリポジトリを任意の場所に配置します。
2. DirectXTexを用意します。
3. DirectXTexのインクルードパスとライブラリパスが、プロジェクト設定と一致しているか確認します。
4. Visual Studioで `Game_DirectX12_Project.slnx` を開きます。
5. 構成を `Debug`、プラットフォームを `x64` にしてビルドします。
6. ビルド後、Visual Studioからデバッグ実行します。

`Game_DirectX12_Project.slnx` は `Game_DirectX12_Project.vcxproj` を参照しています。Visual Studioの表示では、環境によって `x86` と `Win32` の表記が混在することがあります。迷った場合は、まず `Debug | x64` で確認してください。

## 実行方法

Visual Studioでプロジェクトを開いたあと、次のいずれかで実行します。

- `F5`: デバッグ実行
- `Ctrl + F5`: デバッグなしで実行

実行すると、640 x 480のウィンドウが表示されます。

## 基本操作

タイトル画面:

- 上キー / 下キー: メニュー選択
- Enter: 決定
- Esc: 終了

シューティング画面:

- マウス移動: プレイヤー移動
- Zキー: ショット
- 左クリック: ショット
- Enter: プレイヤー死亡後にタイトルへ戻る
- Esc: 終了

## プロジェクト構成

主なファイルと役割は次の通りです。

```text
.
├── Main.cpp
│   Windowsアプリのエントリーポイント、ウィンドウ生成、メインループ
├── Game.cpp / Game.h
│   ゲーム全体の初期化、入力、更新、描画、シーン管理
├── Scene.cpp / Scene.h
│   シーンの基底クラス、Actorの追加・更新・削除の土台
├── ShootingTitleScene.cpp / ShootingTitleScene.h
│   タイトル画面
├── ShootingScene.cpp / ShootingScene.h
│   シューティングゲーム本編
├── Actor*.cpp / Actor*.h
│   ゲーム内オブジェクトの基底クラスや派生クラス
├── PlayerActor.cpp / PlayerActor.h
│   プレイヤーの移動、ショット、ダメージ処理
├── EnemyActor.cpp / EnemyActor.h
│   敵の移動、攻撃、当たり判定まわり
├── Renderer.cpp / Renderer.h
│   DirectX 12の初期化、テクスチャ、シェーダー、スプライト描画
├── Shader.cpp / Shader.h
│   シェーダー管理
├── SoundSystem.cpp / SoundSystem.h
│   BGM、効果音の読み込みと再生
├── Keyboard.cpp / Keyboard.h
│   キーボード入力
├── Mouse.cpp / Mouse.h
│   マウス入力
├── GameUtil.cpp / GameUtil.h
│   共通データ構造、Actor管理補助、便利関数
├── GMath.cpp / GMath.h
│   数学系の補助関数
├── src/
│   画像、音声、フォント画像などの素材
└── shader/
    ビルド後のコンパイル済みシェーダー出力先
```

## ゲームの流れ

起動後の流れは次のようになっています。

1. `wWinMain` がウィンドウを作成する
2. `Game::initialize` がRenderer、SoundSystem、入力、最初のシーンを初期化する
3. 最初のシーンとして `ShootingTitleScene` が生成される
4. タイトル画面でStartを選ぶと `ShootingScene` に遷移する
5. `Game::loop` が毎フレーム、入力、更新、描画を呼ぶ
6. シーンが終了するとゲームも終了する

シーン遷移は `SceneState` で表現されています。

- `Continue`: 現在のシーンを継続
- `Push`: 新しいシーンを上に追加
- `Pop`: 現在のシーンを終了
- `Replace`: 現在のシーンを別シーンに置き換え

## 素材について

画像や音声は `src/` に置かれています。

主な素材:

- `back-s03b.png`: 背景
- `pipo-airship01.png`: プレイヤー
- `kabocha.png`: 敵
- `pipo-hikarimono007.png`: 弾
- `pipo-mapeffect005.png`: 爆発
- `pipo-charachip_smoke02a-s.png`: 煙
- `PixelMplus12.png`: 文字描画用
- `maou_bgm_fantasy02.mp3`: BGM
- `maou_se_battle_gun05.mp3`: ショット音
- `maou_se_battle_explosion06.mp3`: 爆発音

コード内では `L"src\\ファイル名"` のような相対パスで読み込んでいます。実行時の作業ディレクトリがプロジェクトルートとずれると、素材の読み込みに失敗する可能性があります。

## シェーダーについて

HLSLファイルはルート直下にあります。

- `SimpleVS.hlsl`
- `SimplePS.hlsl`
- `AdditiveBlendPS.hlsl`
- `Simple.hlsli`

ビルド時にコンパイルされたシェーダーは `shader/` に `.cso` として出力される設定です。

Renderer側では、用途ごとに次のシェーダー番号が定義されています。

- `Renderer::Shader2DLoopLinear`
- `Renderer::Shader2DAlphaLoopPoint`
- `Renderer::Shader2DAddLoopPoint`

## 新しい要素を追加するとき

新しいActorを追加する場合の基本的な流れです。

1. 既存の `Actor`、`SpriteActor`、`BlockAnimActor`、`CharaActor` など、近い基底クラスを探す
2. `〇〇Actor.h` / `〇〇Actor.cpp` を追加する
3. `update` に移動、寿命、入力、当たり判定などを書く
4. `draw` に描画処理を書く
5. 使用するシーンで生成し、Actorリストへ追加する
6. Visual Studioのプロジェクトに新しい `.cpp` / `.h` を追加する

新しいシーンを追加する場合の基本的な流れです。

1. `Scene` を継承した `〇〇Scene` を作る
2. `update` と `draw` を実装する
3. 必要な画像や音声をコンストラクタで読み込む
4. 遷移元のシーンで `SceneState::Push` または `SceneState::Replace` を返す
5. `Game.cpp` の初期シーンを変えたい場合は、`Game::initialize` の生成箇所を変更する

## よくあるつまずき

DirectXTexが見つからない:

- `C:\DirectXTex\DirectXTex` にDirectXTexがあるか確認してください。
- パスが違う場合は、プロジェクト設定のインクルードディレクトリとライブラリディレクトリを修正してください。

素材が読み込めない:

- `src/` が実行時の作業ディレクトリから見える位置にあるか確認してください。
- Visual Studioのデバッグ設定で作業ディレクトリがプロジェクトルートになっているか確認してください。

シェーダーが見つからない:

- HLSLファイルがビルドされ、`shader/` に `.cso` が出力されているか確認してください。
- `shader/` フォルダが存在するか確認してください。

Win32でビルドできない:

- プロジェクト設定ではWin32構成もありますが、DirectXTexのライブラリパスがx64向けになっています。
- まずは `Debug | x64` でビルドするのがおすすめです。

## 動作確認チェックリスト

初回セットアップ後は、次の内容を確認してください。

- Visual Studioで `Game_DirectX12_Project.slnx` を開ける
- `Debug | x64` でビルドできる
- 実行するとウィンドウが表示される
- タイトル画面で上下キーにより選択色が変わる
- Enterでシューティング画面へ進める
- マウス移動でプレイヤーが動く
- Zキーまたは左クリックで弾が出る
- BGMと効果音が再生される
- Escで終了できる

## 開発メモ

- 画面サイズは `Main.cpp` の `WinWidth` / `WinHeight` で定義されています。
- ゲームの目標フレームレートは `Game::FrameRate` で定義されています。
- 現在の初期シーンは `Game::initialize` 内の `ShootingTitleScene` です。
- Actorの削除や一時追加リストの移動は `GameUtil` 系の補助関数で処理されています。
- DirectX 12まわりの詳細は主に `Renderer.cpp` に集約されています。
