# **pomfit - Pomf it !**
------------------------
About:

Simple image and file uploader for http://pomf.se written in C using GTK.
Just alternative to bash scripts lying around.

## Features:
------------
* Global hotkeys
* Notifications
* Upload log
* Direct upload from file manager

## Global hotkeys:	
----------------
* "Win + U" - calls for "scrot -s" , uploads right away and copies link into clipboard
* "Win + W" - opens web browser with last link
* "Win + C" - same as "Win + U" BUT without uploading
* You can always edit them them in the Main source file to fit your taste.

## Install:
----------
	Extract -> Run autogen.sh -> make ->(su/sudo) make install

## Dependencies:
-------------
	'scrot' 'curl' 'libkeybinder3' 'libnotify'

### Arch Linux Package:
[Version 0.0.3](http://a.pomf.se/lwpbuo.tar.gz)
* Install: pacman -U "packagename"
