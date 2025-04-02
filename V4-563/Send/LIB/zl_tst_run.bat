@setlocal enableextensions enabledelayedexpansion
@@echo off

dEl zl_tst.log
del zl_tst.sum
dEl zl_tst_run.log

set logfile=zl_tst_run.log

zl_tst.exe 2> !logfile!

