---
layout: post
title: Windows Internals Basics
---

Let's start by looking at the loaded modules of notepad++ in windbg.<!--more-->

![alt text][modules]

### PE Header
Every portable executable begins with a DOS header having a structure of type IMAGE_DOS_HEADER. We can see this structure by 


#### References
[Windbg commands](http://windbg.info/doc/1-common-cmds.html)

[modules]: {{ site.baseurl }}/assets/images/modulesLoaded.JPG "modules loaded"
