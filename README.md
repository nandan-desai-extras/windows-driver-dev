### Introduction

This repository contains code for some simple Windows software drivers and filter drivers I created while learning about driver development. 

Here, the term 'software driver' refers to the driver not being associated with any hardware device. The [Microsoft documentation](https://learn.microsoft.com/en-us/windows-hardware/drivers/gettingstarted/what-is-a-driver-) presents several terminologies for the different types of drivers. A driver can be a 'Filter driver' that enhances the feature of the underlying device driver (or a filesystem driver), a device driver that operates a hardware device, a filesystem driver that deals with filesystems like NTFS and FAT, and a software driver that simply tries to access certain data structures in the kernel mode and is not associated with any device.

### Environment Setup

For setting up your environment for Driver development, read my guide [here](/environment-setup.md).

### Process Interaction

This is a very simple software driver that logs to the Debug output whenever a process opens a handle to another process. 

This driver uses `ObRegisterCallbacks()` to register callback functions on Process handle operations. In Windows, every resource is represented as an _Object_ and is handled by the _Object Manager_(_Ob_). More here: https://en.wikipedia.org/wiki/Object_Manager_(Windows)

The debug log when this driver runs, looks like this (viewable in Windbg or the Debug View app of Sysinternals):
```
[ProcessInteraction] Access to process \Windows\System32\VBoxService.exe from \Windows\System32\svchost.exe
[ProcessInteraction] Access to process \Windows\System32\VBoxService.exe from \Windows\System32\svchost.exe
[ProcessInteraction] Access to process \Windows\System32\wbem\WmiPrvSE.exe from \ProgramData\Microsoft\Windows Defender\Platform\4.18.23110.3-0\MsMpEng.exe
[ProcessInteraction] Access to process \Windows\System32\wbem\WmiPrvSE.exe from \ProgramData\Microsoft\Windows Defender\Platform\4.18.23110.3-0\MsMpEng.exe
[ProcessInteraction] Access to process \Windows\System32\wbem\WmiPrvSE.exe from \ProgramData\Microsoft\Windows Defender\Platform\4.18.23110.3-0\MsMpEng.exe
```

As you may notice, we get the full PE image (the .exe file) path of the process while printing this log. A common way to get the PE image name of a process is to actually open a handle (using `ObOpenObjectByPointer()`) to that process and access the attributes of that process that way. But since we are 'intercepting' all calls to open a handle to a process, using `ObOpenObjectByPointer()` in our intercept routine would cause a recursion, since the driver will be intercepting its own calls again and again. This will eventually cause a stack overflow and the driver will crash.

To avoid this, I've created my own function to access the PE image path through the EPROCESS ("Executive Process") structure of the kernel. Here's how the function is defined: 
```C
PUNICODE_STRING GetProcessNameFromEPROCESS(PEPROCESS process) {
    if (process != NULL) {
        int IMAGE_FILE_PTR_OFFSET = 0x5a0;
        FILE_OBJECT* imageFileObj = (FILE_OBJECT*)*(UINT64*)(((UINT64)process) + IMAGE_FILE_PTR_OFFSET);
        if (imageFileObj == NULL) {
            return NULL;
        }
        return (PUNICODE_STRING)&imageFileObj->FileName;
    }
    return NULL;
}
```

This function doesn't call any Object Manager routines and hence doesn't open a handle to our target process. It simply traverses the EPROCESS and FILE_OBJECT structures. Although, this is not a recommended way of doing this since Microsoft might decide to update the layout of those structures and the offsets might change, which will break our code.

#### Running the Software Driver

After deploying the Driver files to the target machine, use the following commands to start and stop the driver:

Create the driver service:
```
sc create processinteraction type=kernel binpath="C:\DriverTest\Drivers\processinteraction.sys"
```

Start the service:
```
sc start processinteraction
```

To stop the service:
```
sc stop processinteraction
```

To delete the service:

```
sc delete processinteraction
```

#### Potential Problems and Workarounds

If you're creating a driver project from scratch, make sure to add "/INTEGRITYCHECK" as a Linker command-line option in the project properties (in Visual Studio: right-click on the project -> Properties -> Linker -> Command Line). Otherwise, you'll get "Access Denied" error upon running `sc start <driver service name>` in CMD. More: https://learn.microsoft.com/en-us/cpp/build/reference/integritycheck-require-signature-check




### Filesystem Filter

This is a "minifilter" driver that uses certain functionalities exposed by the Windows-supplied Filter Manager (_FltMgr.sys_).

This minifilter driver intercepts read/write requests going to the Filesystem and it logs the details to the Debug output (viewable in Windbg or the Debug View app of Sysinternals).

The output looks like this:
```
FilesystemFilter: File operation from PID=2904, ProcessName=\Device\HarddiskVolume1\ProgramData\Microsoft\Windows Defender\Platform\4.18.23110.3-0\MsMpEng.exe, Filename=\Device\HarddiskVolume1\Windows\System32\kernel32.dll
FilesystemFilter: File operation from PID=2904, ProcessName=\Device\HarddiskVolume1\ProgramData\Microsoft\Windows Defender\Platform\4.18.23110.3-0\MsMpEng.exe, Filename=\Device\HarddiskVolume1\Windows\System32\kernel32.dll
FilesystemFilter: File operation from PID=2904, ProcessName=\Device\HarddiskVolume1\ProgramData\Microsoft\Windows Defender\Platform\4.18.23110.3-0\MsMpEng.exe, Filename=\Device\HarddiskVolume1\Windows\System32\kernel32.dll
FilesystemFilter: File operation from PID=2904, ProcessName=\Device\HarddiskVolume1\ProgramData\Microsoft\Windows Defender\Platform\4.18.23110.3-0\MsMpEng.exe, Filename=\Device\HarddiskVolume1\Windows\System32\ntdll.dll
FilesystemFilter: File operation from PID=2904, ProcessName=\Device\HarddiskVolume1\ProgramData\Microsoft\Windows Defender\Platform\4.18.23110.3-0\MsMpEng.exe, Filename=\Device\HarddiskVolume1\Windows\System32\ntdll.dll
```

As you can notice, it prints the Process ID, full PE image path and the file that the process is accessing.

#### Running the Filter Driver

After deploying the Driver files to the target machine, right click on the .inf file and select 'Install'. After installation, open a CMD, and execute `fltmc load <filter driver name>`. Here, the filter driver name is whatever mentioned in the .inf file (usually the name of the sys file without the ".sys" extension). To stop the filter driver, just use `fltmc unload <filter driver name>`.

#### Potential Problems and Workarounds

- If you're making your own Filter Driver project from scratch, make sure you've put the correct configuration in the .inf file.
- If you ever get "Section [DefaultInstall] should have an architecture decoration." error on the INF file, change the Target Platform from 'Universal' to 'Desktop' in Driver Settings (in Visual Studio: right-click on the project -> Properties -> Driver Settings -> Target Platform).
- Even after adding #include<fltKernel.h> header while developing the minifilter, Linker complains about not finding Filter-related functions. I found the solution using this SO answer:

> I ran into the same issue. For me, the problem was that the
> mini-filter template was not showing up in the templates listing for
> new projects and so I had to create it from scratch and I inevitably
> missed something. After cross-checking the linker options against the
> minifilter projects provided by Microsoft as reference ([check here](https://github.com/microsoft/Windows-driver-samples/tree/main/filesys/miniFilter)) I
> realized that fltMgr.lib has to be specifically provided to the
> linker. In order to do that right-click on the project in the
> "Solution Explorer" left pane. Then go to
> Properties->Linker->Input->Additional Dependencies. Add
> $(DDK_LIB_PATH)\fltMgr.lib to the list of additional dependencies and
> rebuild your project! I hope this does it for you, but like the
> Microsoft documentation points out LNK2019 can be caused by lots of
> other problems with your configuration.

Source: https://stackoverflow.com/a/72845445

- If you ever use "sc" command to create a service for your filter driver, you might receive "[SC] StartService FAILED 2: The system cannot find the file specified.", after running "sc start <service name>". I'm exactly sure about the reason for this but I simply prefer using "fltmc" command to load the filter.

## License

MIT License

Copyright 2024 Nandan Desai

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.