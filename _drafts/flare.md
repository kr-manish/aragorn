---
layout: post
title: FLARE On Challenge 2018 
description: Write ups for fifth Flare-On edition
tags: [CTF, reversing]
---

Flare-On is a CTF challenge organized by Flare team of FireEye labs. There are a total of 12 challenges totally based on reverse engineering. Without further ado, lets get started on the write ups of challenges which I was able to solve.<!--more-->

#### 1. MinesweeperChampionshipRegistration

The first challenge was a jar file. Upon double clicking it brings up a registration box which asks for a validation code. On wrong entry, error message. 

![validation box][validation]

Lets open this file in a java decompiler "JD-GUI". This utility displays java source codes of .class files. We can see that there is a inviteValidator.class file is present which has the following content

![validator class][classValidator] 

In the main function, it is comparing the input with a string "GoldenTicket2018@flare-on.com". So, this is our flag for this level. We can check by providing this as input to that validation box and the result is

![success][success]  

#### 2. UltimateMineSweeper

The second challenge was an exe file. Double clicking this file opens up the minesweeper game as follows.  

![Minesweeper game][game]

We can see that this game has 900 (30\*30) squares out of which 897 are filled with mines and 3 are safe. And the task is to reveal all the non-mine squares to win this round. Well, 3 out of 900, what are the chances...right! If any of the mine squares are revealed, failure message will appear like this:  

![failure][fail]

So, lets start analyzing this sample. CFF Explorer has revealed that its a .NET compiled sample. There is one debugger which is perfect to use these kind of samples, __dnSpy__. You can download it from [here][dnSpy]. Start the dnSpy and open the file in it. On the left side of the main window of this debugger, assembly explorer is present. You can ahead and browse through this list. You will see that there is one _program_ section present which contains the following Main() function:  

![main function][main]

A MainForm() function is passed as an argument to the Application.Run() method. To navigate to this function, just left click and you will see the contents as follows:  

![main form][mainform]

Looks like this function is initializing all the components of the game, initializing a variable MineField to VALLOC_NODE_LINIT (which is 30, will see shortly) which is basically the dimensions of the minesweeper game. And this is then passed to the function named _AllocateMemory_.  

![Allocate memory][allocateMem]

After analyzing this function we can deduce that this is the function that is setting which squares will have mines and which will not. There are two for loops, each will run 30 times and on the basis of a variable named _flag's_ value, it will set the squares to true (mine present) or false (mine not present). Also, we can deduce that _num2_ represents the column and _num_ is rows (if counting of squares starts from 0). 

There is an _if_ loop inside this function which sets the value of flag to false, which is what we want, right! The iteration in which this if condition is met, we will get a pair of co-ordinates where flag is not present. This _if_ condition is checking whether __VALLOC_TYPES__ array contains the value retured by the _DeriveVallocType_ function. Lets see what this is returning.   

![DeriveVallocType][derive]

This function is returning the bitwise inversion (~) of the result of a mathematical calculation, i.e., (row\*30 + column). Now, lets see the array with which this is compared to.  

![Valloc Types][constants]

This array contains three constant variables, __VALLOC_TYPE_HEADER_PAGE__(4294966400u),  __VALLOC_TYPE_HEADER_POOL__(4294966657u) and  __VALLOC_TYPE_HEADER_RESEARVED__(4294967026u). The inversed output should be equal to these values in order to set a square with no mine. So, the output of that equation (without doing inverse) should be equal to the inversed values of these constants and those values are 895, 638 and 269. Lets do some mathematics now.  
r \* 30 + c = 895 => r = 29 and c = 25  
r \* 30 + c = 638 => r = 21 and c = 8  
r \* 30 + c = 269 => r = 8 and c = 29  

So, we got our three rows and columns set. One other way we can do this is just put a breakpoint inside the _if_ statement in the dnSpy and run the application by pressing F5. When it will breaks at your breakpoint, take a note of the variables(r and c) when that happens. It will look something like the following image. You have to do this thrice to get all the set.  

![break point][break]

Once you get the three values, open the game and reveal the specific squares and voila!! you will get the congratulations message along with the flag.  

![success][success2]

Flag is: Ch3aters_Alw4ys_W1n@flare-on.com  

__Note__: more to come....

[validation]: {{ site.baseurl }}/assets/images/flare/validation.PNG
[classValidator]: {{ site.baseurl }}/assets/images/flare/classValidator.PNG
[success]: {{ site.baseurl }}/assets/images/flare/success.PNG
[dnSpy]: https://github.com/0xd4d/dnSpy/releases
[main]: {{ site.baseurl }}/assets/images/flare/main.PNG
[mainform]: {{ site.baseurl }}/assets/images/flare/mainform.PNG
[allocateMem]: {{ site.baseurl }}/assets/images/flare/allocateMem.PNG
[derive]: {{ site.baseurl }}/assets/images/flare/derive.PNG
[constants]: {{ site.baseurl }}/assets/images/flare/constants.PNG
[break]: {{ site.baseurl }}/assets/images/flare/break.PNG
[fail]: {{ site.baseurl }}/assets/images/flare/fail.PNG
[success2]: {{ site.baseurl }}/assets/images/flare/success2.PNG
[game]: {{ site.baseurl }}/assets/images/flare/game.PNG
