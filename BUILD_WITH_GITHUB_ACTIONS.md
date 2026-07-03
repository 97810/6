# 不安装本地环境，使用 GitHub Actions 云端打包 Windows exe

你的电脑磁盘空间不足时，可以把本项目上传到 GitHub，让 GitHub 的 Windows 云端环境自动编译并生成交付压缩包。

## 第一步：新建仓库

1. 打开 GitHub。
2. 新建一个仓库，例如 `F2_NetworkSwitch_Rebuild`。
3. 仓库可以设为 Private。
4. 不需要勾选初始化 README。

## 第二步：上传项目

把本项目文件夹中的全部内容上传到仓库根目录。上传后，仓库根目录应该能直接看到：

```text
CMakeLists.txt
src/
resources/
lib/
packaging/
.github/workflows/windows-release.yml
```

注意：不要只上传外层压缩包，要上传解压后的项目内容。

## 第三步：运行云端编译

1. 进入仓库页面。
2. 点击顶部的 `Actions`。
3. 选择左侧的 `Build Windows Release`。
4. 点击 `Run workflow`。
5. 选择默认分支，然后点击绿色按钮运行。

## 第四步：下载交付包

编译成功后：

1. 打开刚刚完成的 workflow 运行记录。
2. 在页面底部找到 `Artifacts`。
3. 下载 `F2_NetworkSwitch_v1.0`。
4. 解压后会得到 `F2_NetworkSwitch_v1.0.zip`。
5. 再解压这个 zip，里面就是最终可交付文件夹。

最终结构应该是：

```text
F2一键断网_v1.0/
  F2一键断网.exe
  lib/
    start.mp3
    end.mp3
  使用说明.txt
```

把这个 `F2一键断网_v1.0` 文件夹压缩后发给用户即可。用户解压后双击 `F2一键断网.exe` 运行。

## 注意事项

- 不要让用户只拿走 exe，必须保留 `lib` 文件夹。
- `config.ini` 会在用户第一次保存热键配置后自动生成在 exe 同目录。
- 如果 GitHub Actions 报错，先打开日志查看是 CMake 配置错误、编译错误，还是打包文件缺失。
