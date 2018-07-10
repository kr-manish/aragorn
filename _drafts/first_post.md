---
layout: post
title: Reverse Engineering - Part 1
---

Let's start by looking at the loaded modules of notepad++ in windbg. List of all loaded modules can be displayed using `lm` command.<!--more-->

![alt text][modules]

### PE Header
Every portable executable begins with a DOS header having a structure of type IMAGE_DOS_HEADER. We can see this structure with the help of following command.

**``` !dt _IMAGE_DOS_HEADER <base address of PE> ```**

This gives us two important information:
   * e_magic - This has the __MZ__ signature hex valus 0x5a4d. Every portable executable will begin with this sequence.
   * e_lfanew - Offset of PE file header. This value is in decimal and hence must be converted into hexadecimal before locating PE file header.

#### Data Directories
The syntax to see **Image Header** is: _```!dh <image base address> <option>```_. This gives many information including data directory arrays.

#### Import Directory
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
IMAGE -> dd base_address+RVA of import table

We can now relate this to the _IMAGE_IMPORT_DESCRIPTOR structure.  
OriginalFirstThunk = ?  
ForwardChain = ?  
TimeDateStamp = ?  
Name = ?  
FirstThunk = ?  

Let us now view the address pointed out by OriginalFirstThunk:
IMAGE -> dd addr  
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
#### References
[Windbg commands](http://windbg.info/doc/1-common-cmds.html)

[modules]: {{ site.baseurl }}/assets/images/modulesLoaded.JPG "modules loaded"
