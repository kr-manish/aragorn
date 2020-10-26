---
layout: post
title: Fuzzing 
tags: [Fuzzing, reversing, security, windows internals]
---
So, I was reading about the fuzzers lately and it got me interested into learning more about it. But, rather than just getting to know about the fuzzing tools, why not learn about the vulnerabilities along with it. That way, we can make more sense of the output of the fuzzers and will be able to find out the root cause of the crashes.<!--more--> So, this is going to be a series of blogs, in which I will try to learn a new vulnerabilities and how we can discover them with fuzzers. I am going to use [AFL](https://github.com/google/AFL) (American Fuzzing Lop) for this, which is one of the very powerful and advanced fuzzers out there. There are other tools present which can be used for the same.   
This blog is just a basic introduction to fuzzing and the docker setup that I have created for the same. Let's start the journey.  

#### What is Fuzzing?
Fuzzing is a way of discovering bugs by providing randomized inputs to programs to find test cases that cause a crash. In simpler terms, fuzzing is a process in which different inputs are provided to the program and it's response is captured. Now we are mostly concerned about the inputs which causes the crash in the program and using which we can do some root cause analysis of the crash to pin point the location of thebug.  
To do all this manually for a program which has lot of paths is very tedious and time consuming (and boring =D). So, to make all this easier lot of automated fuzzers are written, which can give us the inputs that causes the crash which we can then use for our analysis. 

Now, there are different kind of fuzzers categorized based on:  
  * knowledge of inputs provided to them - Smart or Dumb
  * way they generate testcases - Mutation or Generation-based 
  * their awareness to internal structure of program - Black-box, White-box or Grey-box

To learn more about these different types of fuzzers, there are many resources available which I have mentioned in the resources section.  

#### Setting up the Environment
Before starting with the actual fuzzing, let's first prepare an environment for the job. There are few steps in this:
  * Deciding a suitable OS
  * Installing all the pre-requisites
  * Installing AFL

One can choose any OS for fuzzing based on their preference. I have taken ubuntu 20.04 version for this.  
There are multiple pre-requisites for AFL. Most of these requirements can be easily obtained from ubuntu repository. These pacakages include latest gcc and clang compilers and for crash analysis gdb is required.  

Building AFL is not that complex. There are few ways to do this.  

One is directly from the ubuntu Repo:
```
sudo apt-get install afl
```
 
Other wa is to compile it from the source, which can be done as follows:
```
git clone https://github.com/google/AFL.git && cd AFL && make && make install
```
##### Docker Image
To make this step easier for future and maybe for others, I have created a docker image for this. It can be pulled from [here][dockerImage].  
To pull the image and run, following steps should be taken:
```
docker pull aragornsec/fuzzing

# Running the instance locally
docker run --rm -it --mount type=bind,source=<path_to_local_directory>,target=<path_in_container> aragornsec/fuzzing
```
Few arguments needs some clarification in the docker run command above:
  * --it -> This is to make the docker image interactive 
  * --rm -> Once exit, this will remove any instance of this image. This can be checked with __docker ps -a__ command.
  * --mount -> This mounts the local directory into the image. Basically this is to retain whatever output/crashes we get after running the afl in thatimage. Even if we kill the instance of the image, we will not loose the output and next time we run the image, it will get all that reports in the image instance.

The Dockerfile is present at [this][github] github location, in case someone wants to modify this file according to their needs.

#### Preparing a Target
Let us suppose that we have chosen a target app for fuzzing and we have the source code of that app. Before running afl, we have to compile the source code with AFL's compiler wrapper for documentation of the binary. Why documentation? Well, in order to get all the code paths in the application, some instructions can be added in apps that can be detected by the fuzzers. This helps in finding more sofisticated vulnerabilities. For this documentation purpose, we can use __afl-gcc__ to compile and get the binary. Following commands can be run to prepare the binary
```
CC=afl-gcc ./configure
make
```
Before running make, setting AFL_HARDEN=1 will cause the CC wrapper to automatically enable code hardening options that make it easier to detect simple memory bugs

This is the most basic way to get the documented binary. There are many other parameters which helps in detecting more bugs. Such as:
  * -fsanitize=address => To use Address Sanitizer which helps in finding memory related issues
  * -ggdb => Produce debugging information for use by GDB
  * -fno-omit-frame-pointer => To make debugging easier 

To avoid crashes getting mis-interpreted as hangs, we can run following command
```
sudo echo core > /proc/sys/kernel/core_pattern
```

#### Running AFL
After creating the binary, we are ready to run the fuzzer on the binary. To start fuzzing, we have to run following command
```
afl-fuzz -i <input directory> -o <output directory> /path/to/program [...params...]
```
For programs that take input from a file, use ‘@@’ to mark the location in the target’s command line where the input file name should be placed.

#### References
  * [Types of Fuzzers](https://sgros-students.blogspot.com/2018/12/types-of-fuzzers.html)

[dockerImage]: https://hub.docker.com/r/aragornsec/fuzzing
[github]: https://github.com/kr-manish/FuzzingToolkit
