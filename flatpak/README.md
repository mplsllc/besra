# Flatpak packaging

`com.mplsllc.Besra.yaml` is the manifest; `release.sh` builds it and publishes
to the ostree repo served at `https://flatpak.mp.ls/repo`.

## Install (on any machine, once published)

```sh
flatpak remote-add --user --no-gpg-verify --if-not-exists besra https://flatpak.mp.ls/repo
flatpak install --user besra com.mplsllc.Besra
flatpak run com.mplsllc.Besra
```

`flatpak update --user com.mplsllc.Besra` picks up new releases after the
next `release.sh` run.

## Publish a new build (on the server hosting flatpak.mp.ls)

```sh
flatpak/release.sh
```

## Local build/test only (no publish)

```sh
cd flatpak
flatpak-builder --user --install --force-clean build-dir com.mplsllc.Besra.yaml
flatpak run com.mplsllc.Besra
```
