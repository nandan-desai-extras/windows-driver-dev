You HAVE to test your driver in a Virtual Machine, because any unhandled error/exception in your driver will cause a Blue Screen of Death (BSOD) and render your system unusable for the time being. If its a VM, you can simply restart the VM or reinstall the VM to recover (taking snapshots of the VM when it is in a stable state will significantly help).  I also carry out my driver 'development' inside a VM. In this guide, I'll be explaining setting up two VMs for driver development and testing.

I use Virtual Box to setup my Virtual Machines (VMs) and hence, this guide will also refer to Virtual Box. But the procedure remains same regardless of the underlying hypervisor.

### Setup

-   Create two VMs with Windows 10. Let's call them "Development VM" and "Test VM". "Development VM" will be used to develop the driver, and "Test VM" will be used to deploy and test the driver. Next, make both the VMs exist in the same “NAT Network” in Virtual Box.
    
-   After successful installation of Windows 10/11 on both the VMs, make sure to change the Windows Firewall settings in both the VMs so that they can ping each other. This can help us to troubleshoot in the future. Refer here on how to change the Firewall settings: [https://activedirectorypro.com/allow-ping-windows-firewall/](https://activedirectorypro.com/allow-ping-windows-firewall/)
    
-   Download Visual Studio on the Development VM, and install “Desktop Development with C++” module and also select Windows 11 SDK. Wait for around 20-30 mins for it to download and install.
    
-   Next, the same Development VM, download and install Windows Driver Kit (WDK). It will also install the Visual Studio extension for Driver development. [Download the Windows Driver Kit (WDK) - Windows drivers | Microsoft Learn](https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk#download-icon-step-3-install-wdk)
    
-   After that’s done, copy the kdnet.exe and VerifiedNICList.xml (usually found in `C:\Program Files (x86)\Windows Kits\10\Debuggers\x64`) from Development VM to the Test VM (create a Shared Folder between the two VMs and the Host PC to make it easy to move files).
    
-   On the Test VM, create C:\KDNET folder and move these two files in there. Then execute `kdnet.exe <development vm ip> <port of your choice>`
    
-   That will give you a debug key. Use this key in WinDbg on the Development VM to connect to the Test VM. Use the same port you used in that kdnet.exe command. Also, make sure to save the debug key and the port somewhere for easy access. Windbg will display the following:
```
Using NET for debugging
Opened WinSock 2.0
Waiting to reconnect..
```

-   Then restart the Test VM and the debugger on Development VM will be attached to the Test upon reboot. Refer: [https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/setting-up-a-network-debugging-connection-automatically](https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/setting-up-a-network-debugging-connection-automatically)

At this stage, the Test VM should have “Test Mode” displayed in the bottom right corner.


#### Setup Visual Studio for Driver Development and Deployment

(Here, "Deployment" refers to compiling the driver and copying the compiled driver files (the .sys and .inf files) to the Test VM).

-   Create a “KMDF, Empty” type project

-   Install WDK on the Test VM as well. [https://learn.microsoft.com/en-us/windows-hardware/drivers/gettingstarted/provision-a-target-computer-wdk-8-1](https://learn.microsoft.com/en-us/windows-hardware/drivers/gettingstarted/provision-a-target-computer-wdk-8-1). And run "WDK Test Target Setup x64-x64_en-us.msi" file on the Test VM.

-   Next, add the Test VM for deployment in Visual Studio (by going to Configuration Properties > Driver Install > Deployment). As shown here: [https://learn.microsoft.com/en-us/windows-hardware/drivers/gettingstarted/writing-a-very-small-kmdf--driver#deploy-the-driver](https://learn.microsoft.com/en-us/windows-hardware/drivers/gettingstarted/writing-a-very-small-kmdf--driver#deploy-the-driver)
    
-   The Test VM might reboot a few times, and you might see “failed to install some components” on the Development VM during setup, but just proceed.
    
-   Update the “Hardware ID Driver Update” after the Test VM is added (again, as shown in the previous link)
    
-   That’s it! Now go to Build > Deploy Build and your driver will be built and the files will be available in C:\DriverTest folder on the Test VM.

#### Setup Driver Debugging

For capturing debug logs in Windbg on the host, execute the following in CMD on the Test VM:

```powershell
reg add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Debug Print Filter" /V DEFAULT /t REG_DWORD /d 0xf
```

In Windbg, to load the symbols (the following commands are specific to Windbg and should be executed in Windbg prompt):

```
.sympath+ <location of your driver project folder>\x64\Debug
```

```
.reload /f
```
  

To load the source:

```
.srcpath+ <location of your driver project folder>
```
 
 To pause execution immediately after the driver is loaded:
```
sxe ld <driver name>.sys
```

Add the breakpoint AFTER the above exception is caught while loading the driver:
```
bp <driver name>!DriverEntry
```
  
Whenever your driver crashes, use the following command to know exactly which line caused the error:
```
!analyze -v
```

At this point, you should have a working system where you can add breakpoints, get debug messages to WinDbg console, etc.

### Start executing the Driver

On the Test VM:

Create the driver service:
```
sc create <driver name> type=kernel binpath="C:\DriverTest\Drivers\<driver name>.sys"
```

Start the service:
```
sc start <driver name>
```

To stop the service:
```
sc stop <driver name>
```

Delete and recreate the service if you've recompiled the driver files. To delete the service:

```
sc delete <driver name>
```

### Automating the setup process

Although I haven't tried, there are some guides on how to automate this entire process.: https://secret.club/2020/04/10/kernel_debugging_in_seconds.html


