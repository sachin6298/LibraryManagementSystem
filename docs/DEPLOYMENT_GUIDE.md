# Cross-platform deployment kit

This kit builds separate native bundles for Linux x86_64 and Windows x64 from the same C++/Qt source. It does **not** try to reuse a Linux executable on Windows.

## Files supplied

- `CMakeLists.txt`: keeps the existing target/sources and adds Qt's CMake deployment API plus install rules. Qt deployment runs on the native OS build runner.
- `.github/workflows/build-desktop.yml`: GitHub Actions workflow for Linux and Windows builds, archives, and tag-based GitHub Releases.
- `.gitignore`: excludes generated build output, installer backups, and runtime database files from Git.
- `setup_deployment.sh`: copies the three configuration files into the existing project and backs up files it replaces outside the project folder.

## 1. Apply the configuration on Ubuntu

Extract the ZIP, open a terminal in `LibraryManagementSystem_CrossPlatformDeploymentKit`, and run:

```bash
bash setup_deployment.sh
```

This targets `~/LibraryManagementSystem` by default. To use another project folder, pass its path as the first argument.

Then build locally:

```bash
cd ~/LibraryManagementSystem
cmake -S . -B build
cmake --build build
```

These commands check that the current machine's Qt development setup can still compile the project. The local build alone does not generate a Windows `.exe`.

## 2. Put the project on GitHub

Create an empty repository on GitHub (do not add a README/license there if you already have local files), then in the project directory run these commands. Replace the URL with the remote URL from your repository:

```bash
git add .
git status --short
git commit -m "Prepare cross-platform desktop release"
git branch -M main
git remote add origin https://github.com/YOUR-USERNAME/LibraryManagementSystem.git
git push -u origin main
```

If `origin` is already configured, do not run `git remote add origin` again; check it with `git remote -v` and use `git remote set-url origin <YOUR-REPOSITORY-URL>` only if the URL needs correction.

Review `git status --short` before committing. `.gitignore` is intended to prevent build files, timestamped backups, and local `.dat` records from being committed. It does not ignore C++ source or documentation.

## 3. Download builds

On a push to branch `main`, GitHub Actions runs separate native jobs:

- Linux runner: builds and bundles Qt runtime dependencies into `LibraryManagementSystem-Linux-x86_64.tar.gz`.
- Windows runner: builds a Windows executable and bundles Qt runtime dependencies into `LibraryManagementSystem-Windows-x64.zip`.

Open the GitHub repository's **Actions** tab, choose the latest successful workflow run, and download the artifacts at the bottom. Artifacts from ordinary workflow runs expire after 14 days.

## 4. Publish a durable GitHub Release

After both builds succeed, tag the current commit:

```bash
git tag v1.0.0
git push origin v1.0.0
```

The workflow runs both native builds and, when they succeed, publishes both archives to a GitHub Release. Use a new version tag (e.g. `v1.0.1`) for a later release.

## 5. How people run the builds

### Linux

Extract `LibraryManagementSystem-Linux-x86_64.tar.gz`, open a terminal in the extracted folder, and run:

```bash
./bin/LibraryManagementSystem
```

The Linux bundle was built on Ubuntu 22.04. Qt dependencies are bundled, but core OS ABI/library compatibility varies between Linux distributions. Test on the target distributions. This archive is not guaranteed to run on every Linux distribution or architecture.

### Windows

Extract `LibraryManagementSystem-Windows-x64.zip` and run `bin/LibraryManagementSystem.exe`. Test the archive on a Windows PC that does not have Qt installed before sharing it. If a Windows runtime dependency is missing, inspect the CI log and make sure the appropriate Microsoft runtime is available; do not copy arbitrary compiler DLLs from another computer.

## Notes

- The workflow installs Qt 6.10.2. The Windows target is x64 MSVC 2022; the Linux target is x86_64 GCC.
- A Linux executable and a Windows `.exe` are different native builds.
- App data remains under the application's per-user home directory, `LibraryManagementSystemData`, rather than inside the install/archive directory.
- The workflow is configured for a `main` branch and version tags `v*`. If your default branch has a different name, update the branch filter in the workflow.
- Before publishing to others, confirm the Qt licensing obligations for the way Qt is linked and distributed in your project.


## App icon and BIT Start page

The Qt resource file embeds `resources/library.png` and `resources/bit-logo.png`. The icon is applied to QApplication and MainWindow. The app opens with the `Start` tab selected; its BIT logo is centered toward the upper portion of the page, and the `Start` button switches to Dashboard. On Windows, `resources/library.ico` is embedded via a resource compiler file.
