### 

### Process Interaction


#### Note
If you're creating a driver project from scratch, make sure to add "/INTEGRITYCHECK" as a Linker command-line option in the project properties (in Visual Studio: (right-click on the project -> Properties -> Linker -> Command Line)). Otherwise, you'll get "Access Denied" error upon running `sc start <driver service name>` in CMD. More: https://learn.microsoft.com/en-us/cpp/build/reference/integritycheck-require-signature-check



