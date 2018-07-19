---
layout: post
title: Reverse Engineering Portable Executable 
tags: [reversing, security, windows internals]
---
The main aim of this blog is to correlate the Operating System Internals and the approach of Reverse Engineering a malware at code level. Here I will be showing what are the basic things that happen when an portable executable is started.  
You can use any executable to deep dive into OS internals. Here I have used microsoft's notepad.exe and to debug <!--more-->this I have used microsoft's debugger called **windbg**, which can be downloaded from [here][WindbgDownloadLink].
#### Portable Executable

Portable Executable (PE) format is a file format for executables, DLLs and others used in windows operating system. The PE format is a data structure that encapsulates the information necessary for the windows OS loader to manage the wrapped executable code. This includes dynamic library references for linking, API export and import tables etc.  
Here we are going to understand PE structure, the concepts and various data directories inside it. Let's get started.  
At first, open windbg and set up the symbols to point to Microsoft's Symbols Server. This is done to make our debugging easier. To understand more about symbols, you can read this [microsoft's documentation][symbol]. 
Now open up the executable (notepad.exe in this case) in windbg. Once opened, it shows all the loaded modules of notepad. We can also get the list of all loaded modules using `lm` command. Take a look at the following output:

![loaded modules][modules]

We can see all the DLLs loaded with a range of addresses. This shows that each module occupies a specific range of address. For instance, notepad.exe  uses some windows API functions exported by kernel32.dll, hence it is loaded along with notepad and the address range where it is loaded is 76e10000 76ee4000. From this we get the base address of kernel32.dll which is 76e10000. These base addresses of modules are very important as we will get to know that usually the value at hand is an RVA (Relative Value Address). This RVA has to be added in base address to get the original address.  
Every PE begins with a DOS header having structure of type **IMAGE_DOS_HEADER**. We can view this in windbg using image base address of PE image, 01000000:  
**``` dt _IMAGE_DOS_HEADER <base address of PE> ```** 

![Dos Header][dosHeader]

This gives us two important information:
   * e_magic - This has the __MZ__ signature hex value 0x5a4d. Every portable executable will begin with this sequence.
   * e_lfanew - Offset of PE file header. This value is in decimal and hence must be converted into hexadecimal before locating PE file header.

In this case, e_lfanew value is 224 which in hexadecimal is E0. Therefore, file header is present at an offset of E0 from the base address of the PE.

#### Data Directories
Now let us see the PE header of our main executable. The syntax to see **PE Header** is:  
__`!dh <image base address> <option>`__.   

![PE Header][peHeader]

This gives many information including data directory arrays. It's an array of structures of **IMAGE_DATA_DIRECTORY** type each having two fields:  
Virtual Address and size.

##### Import Directory
Import table contains an array of data structure of type *_IMAGE_IMPORT_DESCRIPTOR*. This structure has following form: 
```cpp
typedef struct _IMAGE_IMPORT_DESCRIPTOR{
    _ANONYMOUS_UNION union{
        DWORD characteristics;
        DWORD OriginalFirstThunk;
    } DummyName;
    DWORD TimeDateStamp;
    DWORD ForwardChain;
    DWORD Name;
    DWORD FirstThunk;
} IMAGE_IMPORT_DESCRIPTOR, *PIMAGE_IMPORT_DESCRIPTOR;
```
Two important fields at this point are, OriginalFirstThunk and FirstThunk. Each of these point to a table.  

**OriginalFirstThunk**  -> Imports Name Table  
**FirstThunk** -> Import Address Table (IAT)

Let us start by viewing the memory at the Import Table Virtual Address:  

![Import Directory][importDir]

We can now relate this to the _IMAGE_IMPORT_DESCRIPTOR structure.  
OriginalFirstThunk = 0000a234  
ForwardChain = ffffffff  
TimeDateStamp = ffffffff  
Name = 0000a224  
FirstThunk = 00001000  

Let us now view the address pointed out by OriginalFirstThunk:  

![Original First Thunk][oft]

![Original First Thunk Value][oft_val]

We get a list of RVAs which are also called _IMAGE_THUNK_DATA which in turn points to the structure of type _IMAGE_IMPORT_BY_NAME structure:  
```cpp
typedef struct _IMAGE_IMPORT_BY_NAME {
    WORD Hint;
    BYTE Name[1];
} IMAGE_IMPORT_BY_NAME,*PIMAGE_IMPORT_BY_NAME;
```
Here, Name[1] is the name of the imported function which can have a varibale length. So, this means that _IMAGE_THUNK_DATA and _IMAGE_IMPORT_BY_NAME structures have one to one correspondence.  
We can view this as:  
IMAGE -> dc base_addr+image_thunk_data  

Now, let us view FirstThunk.  
IMAGE -> dd base+fisrtthunk  
We see that it is already populated with virtual addresses. Let us view them:  
IMAGE -> ln addr  
IMAGE -> ln addr  
So, these are virtual address of functions imported by PE.  
We see this table already populated because our PE is already loaded by OS loader and the Import Address Table is already filled with function pointers.  

Now, let us look at the Name field in _IMAGE_IMPORT_DESCRIPTOR. This field gives us the information about the name of modules loaded.  
IMAGE -> dc base+name  
The next module loaded is:  

The end of the _IMAGE_IMPORT_DESCRIPTOR array is denoted by a structure filled with all NULL values as shown below:  
IMAGE -> dd base+ ^  
Another important point to discuss at this point is how API calls made in a program are replaced by bytecode by a compiler.  

#### Import Address Table
IAT consists of mappings between the absolute virtual addresses and the function names which are exported from different loaded modules.  
Let us see how we can grab this list of loaded modules. 
IMAGE -> dps base+VA Lsize/4  
here, L is used to denote the size and is divided by 4 to take steps of 4 bytes while displaying the addresses.

The question now is, how did IAT get filled up at run time? We will try to find out this.  
#### Export Directory
Each loaded module will have it's own Export Directory. It is a structure called, IMAGE_EXPORT_DIRECTORY, having the following form:
```cpp
Private type IMAGE_EXPORT_DIRECTORY {
    Characteristics as Long
    TimeDateStamp as Long
    MajorVersion as Integer
    MinorVersion as Integer
    lpName as Long
    Base as Long
    NumberOfFunctions as Long
    NumberOfNames as Long
    lpAddressOfFunctions as Long
    lpAddressOfNames as Long
    lpAddressOfNameOrdinals as Long
}
End Type
```
A summary of the Structure: This contains pointers to 3 arrays. The pointers are in the form of RVAs relative to the base address of the image.
These arrays are:  
**AddressOfFunctions**: It is an arrays of RVAs of the functions in the module.  
**AddressOfNames**: Array of RVAs each corresponding to function name strings of exported functions.  
**AddressOfNameOrdinals**: It gives an index or an offset into the AddressOfFunctions array to get the address of the function name.

So the flow is something like this: The function name is fetched using the Export name table and the corresponding entry in the Export Ordinals Table is looked up to get the index or offset. This index is used to parse the Export Address Table and fetch the function address.

**Export Name Table > Export Ordinals Table > Export Address Table -> Function Address (VA)** 

This should now make it clear how the OS loader gets to know the addresses of functions which are imported by the main module.  
#### Process Environment Block
Process Environment Block is an important data structure from an exploiter's perspective. A shellcode executes a set of function APIs and for this it must locate and load them.  

#### References
  * [Portable Executable File Format](https://msdn.microsoft.com/en-IN/library/ms809762.aspx)  
  * [Windbg commands](http://windbg.info/doc/1-common-cmds.html)

[WindbgDownloadLink]: https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/
[symbol]: https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/symbols-and-symbol-files
[modules]: {{ site.baseurl }}/assets/images/firstPost/modulesLoaded.JPG "modules loaded"
[dosHeader]: {{ site.baseurl }}/assets/images/firstPost/dos_header.JPG "Dos Header"
[peHeader]: {{ site.baseurl }}/assets/images/firstPost/peHeader.JPG "PE Header"
[importDir]: {{ site.baseurl }}/assets/images/firstPost//import_dir.JPG "Import Directory"
[oft]: {{ site.baseurl }}/assets/images/firstPost/original_first_thunk.JPG "Original First Thunk"
[oft_val]: {{ site.baseurl }}/assets/images/firstPost/original_first_thunk_val.JPG 
