---
layout: post
title: FLARE On Challenge 2018 
tags: [CTF, reversing]
---

Flare-On is a CTF challenge organized by Flare team of FireEye labs. There are a total of 12 challenges totally based on reverse engineering. Without further ado, lets get started on the write ups of challenges which I was able to solve.<!--more-->

#### 1. MinesweeperChampionshipRegistration

The first challenge is a jar file. Upon double clicking it brings up a registration box which asks for a validation code. On wrong entry, error message. 

![validation box][validation]

Lets open this file in a java decompiler "JD-GUI". This utility displays java source codes of .class files. We can see that there is a inviteValidator.class file is present which has the following content

![validator class][classValidator] 

In the main function, it is comparing the input with a string "GoldenTicket2018@flare-on.com". So, this is our flag for this level. We can check by providing this as input to that validation box and the result is

![success][success]  

#### 2. UltimateMineSweeper

![Minesweeper game][game]

![failure][fail]

![main function][main]

![main form][mainform]

![Allocate memory][allocateMem]

![DeriveVallocType][derive]

![Valloc Types][constants]

![break point][break]

![success][success2]

#### 3. FLEGGO


[validation]: {{ site.baseurl }}/assets/images/flare/validation.PNG
[classValidator]: {{ site.baseurl }}/assets/images/flare/classValidator.PNG
[success]: {{ site.baseurl }}/assets/images/flare/success.PNG
[main]: {{ site.baseurl }}/assets/images/flare/main.PNG
[mainform]: {{ site.baseurl }}/assets/images/flare/mainform.PNG
[allocateMem]: {{ site.baseurl }}/assets/images/flare/allocateMem.PNG
[derive]: {{ site.baseurl }}/assets/images/flare/derive.PNG
[constants]: {{ site.baseurl }}/assets/images/flare/constants.PNG
[break]: {{ site.baseurl }}/assets/images/flare/break.PNG
[fail]: {{ site.baseurl }}/assets/images/flare/fail.PNG
[success2]: {{ site.baseurl }}/assets/images/flare/success2.PNG
[game]: {{ site.baseurl }}/assets/images/flare/game.PNG
