# Windows USB serial name

`NMRA_DCC_Tester.inf` assigns a descriptive Ports-class name to the device with
hardware ID `USB\VID_0483&PID_5710`. It references the Windows inbox
`usbser.inf` installation sections; it does not install a third-party serial
driver.

The INF is an unsigned development package and does not include a catalog, so
it is not ready to install on standard 64-bit Windows. Generate and sign a
catalog before deployment. For production distribution, submit the package
through the Windows Hardware Dev Center; for local development, use an
appropriately configured test-signing environment.

The VID is STMicroelectronics' `0x0483`; use this package only with the
confirmed device ID. If the device's VID/PID changes, update the model entry
in the INF to match its USB descriptors.
