# **pomfit - Pomf it !**
## Current version 0.0.5b
### Now with batch upload !
------------------------
### Note: 0.0.5b fixes 
0.0.5b fixes a link issue with recent (temporary?)storage changes.
Aside from that pomfit works fine but some browsers may not be able to autofix '\' in links thus the hotfix.
If you are using Chromium or any other broser that have no issue with '\' in links, you dont need to update.
Also if you experienced occasional crash with screencap upload this should help.
Thats all cayooties.
P.S: Do you want a windows port? Lemme know on twitter..win32 api yay~ ;_;

### About Pomf it !
Image and file uploader for http://pomf.se written in C using GTK.
Running in tray, ready to upload anytime through cli, file manager, keybind or trayicon.

### Features:
------------
* Batch Upload
* Global hotkeys
* Notifications
* Upload log
* Upload status bar
* Direct upload from file manager / cli / tray menu / keybinds
* Simplicity & Usability

### Global hotkeys:	
----------------
* "Ctrl + Alt + U" - Screencap Upload (click on window or select area)
* "Ctrl + Alt + B" - Browser (opens last upload link)
* "Ctrl + Alt + C" - ScreenCap (click on window or select area)
* "Ctrl + Alt + F" - File upload (opens selection dialog)
* You can always easily edit them in the Main source file to fit your taste.

### Install:
	Extract -> Run autogen.sh -> make ->(su/sudo) make install

### Dependencies:
	'scrot' 'libcurl' 'libkeybinder3' 'libnotify' 'gtk3-dev'
* Names may slightly vary on different distributions
* Assuming you have basic dev tools to build (automake , autoconf , gcc ...)

### Unofficial Arch Linux Package:
[Version 0.0.5b](http://a.pomf.se/1/acgsun.tar.xz)
* Install: pacman -U "packagename"
* If someone feels like making real AUR package, go ahead.

### Contact
* Feedback , thoughts , bug reports or just talk ?
* Feel free to follow me on [twitter](https://twitter.com/Senketsu_Dev) or visit pomf.se IRC channel
