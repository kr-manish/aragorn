---
layout: post
title: Thread Local Storage
tags: [PE File, reversing, security, windows internals, TLS]
---
In the [previous]({{ page.previous.url }}) blog, we got to know that every PE file has an address of entry point from where program starts executing. But, is it really true! In this blog, we are going to learn how we can run code even before entry point using a capability typically known as Thread Local Storage (TLS) and what it can be used for.<!--more-->

#### What is TLS?  

Every thread of a process shares its virtual address space. The local variable of a function is unique to each thread. But what about global and static variables. They are shared by all the threads in a process. So, it can happen that in a multithreaded program, these global variables are being accessed and changed by one thread. Which can cause problems for other threads. To avoid this race condition, practically all the multithreaded OSes provide a mechanism to efficiently store state on a per thread basis. With TLS, we can provide unique data for each thread that they only can access. To understand more about TLS and understand how it is implemented, you can refer to the links provided in the references.  

Now, we are going to analyze a sample which has a thread local storage implemented in it. It's not doing anything malicious here rather just two message boxes pops up, one from TLS callback function and other from main function. The sample I have used can be downloaded from [here][sample], (password is: clean). The source code is present on [github][code]. So, let's get started with our analysis.  

##### Analyzing the binary  

Before starting the analysis, lets check what happens when we double click the sample. First, the following pop up box appears:  

![TLS message box][tlsMsgBox]

This message box shows that this is being called from TLS callback function. After pressing OK, we will get another message box as following:  

![Main message box][mainMsgBox]

Remember this observation, will come back to this later. Now, lets start with some static analysis. Start the CFF Explorer and open the sample which we are analyzing. (Note: Check if the binary has ASLR enabled. If it is, you may want to disable that because it will make analysis a bit more difficult. To switch this off, go to DLLCharacteristics field under Optional header in the CFF explorer and uncheck the option "DLL can move" and save the file). CFFExplorer shows that this sample has 6 sections including one ".tls" section whose relative virtual address is 0x4000.  

![TLS section][tlsSec]

Looking at the optional headers of the sample, the address of entry point is shown as 0x12D8 which is a relative address. The absolute address of entry point is "image base"(0x400000) + Relative Virtual Address of entry point, which comes to be 0x4012D8.

![Entry point][entryPoint]

Lets now open this in IDA Pro and check what is happening. After its initial analysis, IDA stops at the address 0x00401030, which it thinks is the entry point.

![IDA entry point][idaEntry]

Doing cross reference (keeping cursor on the name and pressing "x") of this main function, it shows that it is being called from an address 0x40126B present in a function which IDA named as "tmainCRTStartup". This function in turn is called from an address 0x4012DD.  

![start][start]

As we can see, this call instruction is present in a method which starts from address 0x4012D8. And if you remember, CFF Explorer also showed that the address of entry point is 0x4012D8! So, it seams IDA stoppped at the correct location from where execution should start. But wait, while analyzing the instructions at this point, we see that one variable is being moved to EAX, then printed on the screen and then few things are pushed onto the stack probably as an argument for the MessageBox windows API. In the arguments, "In Main" text is also present which will be displayed by the messagebox. And indeed in the begining we saw that "In Main" pop up box appears, but that was not the first box, right!! So, what is happening here. Where is the "In TLS" pop up box code present? It seems that IDA was not able to catch the "original" address of entry point.  

If we check the "Functions" subview in IDA, it shows one function name as __TlsCallback_0__. Lets check what's in that.

![IDA TLS][idaTls]

So, our first message box is coming from this region which starts from 0x401000! This is actually a TLS callback function which are executed before the entry point. That's why TLS message box appears first.  
If we open this file in x32Dbg, it successfully detects the TLS callback function and breaks at that point. The reason for the breakpoint and the point where it breaks are shown below.

![x32 breakpoint][x32Break]

![x32 EIP][eip]

###### WinDbg analysis  

Open the file in WinDbg. To see all the data directories of this sample, type __!dh 400000 -f__ and enter.  

![PE headers][headers] 

The TLS structure, IMAGE_TLS_DIRECTORY, pointed to by the TLS directory entry has a small number of fields. The structure looks like this:  

```cpp
typedef struct _IMAGE_TLS_DIRECTORY {
    UINT32 StartAddressOfRawData;
    UINT32 EndAddressOfRawData;
    PUINT32 AddressOfIndex;
    PIMAGE_TLS_CALLBACK *AddressOfCallBacks;
    UINT32 SizeOfZeroFill;
    UINT32 Characteristics;
} IMAGE_TLS_DIRECTORY, *PIMAGE_TLS_DIRECTORY;
```

Lets check these fields for this sample. Execute the command "__dt IMAGE_TLS_DIRECTORY <base image + TLS directory address>__.  

![TLS structure][tlsStruct]

The one special entry which is of interest is the one pointing to a list of callbacks, AddressOfCallBacks. This particular field is a pointer to a null terminated array of callbacks, which are called in sequence. Here it is pointing to a location 0x4020c4. Lets check what is at that address.

![Address of callbacks][addrOfCallback]  

It is indeed a null terminated array of addresses (here it's only 1 because only one TLS callback function is defined in this program). And look what this array contains, 0x401000! A pointer to the first and only callback function which is same as what we saw in IDA's tlscallback_0 function. Lets put a breakpoint at this location using the command __bp 0x401000__, and run the application using the command __g__ and enter. The program will stop at this location.

![TLS callback][tlsCallback]

The first instruction that we see is __mov eax,dword ptr fs:[0x2c]__. From this [wiki], we get to know that fs:[2c] stores the linear address of the thread-local storage array. Step-over few commands until the call to MessageBox and then check the TLS array.  

![variables][tlsVariables]

We can see that the value 0x21F is now stored at the 1st (eax+4) location of the array. You may wonder why it got stored at [eax+4] location? Part-4 of [this] blog, will explain you better about this. So, this is the location where all the TLS variables are stored. 

#### Conclusion

This sample is not doing anything except popping up a message box. But think what can be done using this feature. Samples can check in TLS callback functions whether they are being monitored or not in a sandbox like environment. If they are being monitored then take a different course of action than the malicious one to avoid being detected. They can have an anti-debugging check before execution reaches to the main function and do nothing to deceive the malware researchers in order to prevent or slow down the process of reverse engineering.  
Having said that, any code in TLS callback functions is not an indication that the sample is malicious. There is nothing inherently malicious about TLS, but they are being used by some forms of malware in an attempt to hide data from analysis.  

#### References

  * [msdn](https://docs.microsoft.com/en-us/windows/desktop/procthread/thread-local-storage)
  * [nynaeve's blog](http://www.nynaeve.net/?p=180)

[sample]: {{ site.baseurl }}/assets/images/TLSPost/TLS.7z
[code]: {{ site.baseurl }}/assets/images/TLSPost/tlsSourceCode.c
[tlsMsgBox]: {{ site.baseurl }}/assets/images/TLSPost/tlsMsgBox.png "TLS Message BOX" 
[mainMsgBox]: {{ site.baseurl }}/assets/images/TLSPost/mainMsgBox.png "Main message box"
[tlsSec]: {{ site.baseurl }}/assets/images/TLSPost/TLSsection.png "TLS section"
[entryPoint]: {{ site.baseurl }}/assets/images/TLSPost/entryPt.png "Entry Point"
[idaEntry]:{{ site.baseurl }}/assets/images/TLSPost/idaMain.png "IDA main"
[start]: {{ site.baseurl }}/assets/images/TLSPost/start.PNG "public start"
[idaTls]: {{ site.baseurl }}/assets/images/TLSPost/idaTLS.png "IDA TLS callback function"
[x32Break]: {{ site.baseurl }}/assets/images/TLSPost/x32Break.png
[eip]: {{ site.baseurl }}/assets/images/TLSPost/x32TLS.png
[headers]: {{ site.baseurl }}/assets/images/TLSPost/headers.PNG
[tlsStruct]: {{ site.baseurl }}/assets/images/TLSPost/tlsStruct.PNG "TLS structure"
[addrOfCallback]: {{ site.baseurl }}/assets/images/TLSPost/addrOfCallbacks.PNG "address of callbacks"
[tlsVariables]: {{ site.baseurl }}/assets/images/TLSPost/tlsVariable.PNG
[tlsCallback]:  {{ site.baseurl }}/assets/images/TLSPost/tlsCallback.PNG 
[wiki]: https://en.wikipedia.org/wiki/Win32_Thread_Information_Block
