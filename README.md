# **pomfit - Pomf it !**
## Current version 0.0.6 ~Cube~
### Now with user accounts support !
------------------------
### Note: 0.0.6
* Added user accounts support.
* You can now login through pomfit to have your uploads linked to your account.
* Another update probably, when file expirations will be implemented.(unless bugs appear)
* Recommended update.
* Thats all cayooties.

### About Pomf it !
Image and file uploader for http://pomf.se written in C using GTK.
Running in tray, ready to upload anytime through cli, file manager, keybind or trayicon.

### Features:
------------
* User Accounts
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
[Version 0.0.6](http://a.pomf.se/rlmctt.tar.xz)
* Install: pacman -U "packagename"
* If someone feels like making real AUR package, go ahead.

### Contact
* Feedback , thoughts , bug reports or just talk ?
* Feel free to follow me on [twitter](https://twitter.com/Senketsu_Dev) or visit pomf.se IRC channel
