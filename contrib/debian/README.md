
Debian
====================
This directory contains files used to package talerd/taler-qt
for Debian-based Linux systems. If you compile talerd/taler-qt yourself, there are some useful files here.

## bitcoin: URI support ##


taler-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install taler-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your taler-qt binary to `/usr/bin`
and the `../../share/pixmaps/bitcoin128.png` to `/usr/share/pixmaps`

taler-qt.protocol (KDE)

