if exist senderfb.DXF del senderfb.DXF
if exist senderas.DXF del senderas.DXF
if exist senderpd.plt del senderpd.plt
if exist senderl1.plt del senderl1.plt
if exist senderl2.plt del senderl2.plt
if exist senderl3.plt del senderl3.plt
if exist senderl4.plt del senderl4.plt
if exist sendersm.plt del sendersm.plt
if exist sendersk.plt del sendersk.plt
if exist senderfb.plt del senderfb.plt
if exist senderas.plt del senderas.plt
pccards -r sender.pcb @sender.mac
pcphoto -r senderpd.asr -c sender.ppp -a rev85.apr -o D -s 1.000 senderpd.plt 
if exist PHOTO1.ERR del PHOTO1.ERR
if exist PCPHOTO.LOG ren PCPHOTO.LOG PHOTO1.ERR
pcphoto -r senderl1.asr -c sender.ppp -a rev85.apr -o D -s 1.000 senderl1.plt 
if exist PHOTO2.ERR del PHOTO2.ERR
if exist PCPHOTO.LOG ren PCPHOTO.LOG PHOTO2.ERR
pcphoto -r senderl2.asr -c sender.ppp -a rev85.apr -o D -s 1.000 senderl2.plt 
if exist PHOTO3.ERR del PHOTO3.ERR
if exist PCPHOTO.LOG ren PCPHOTO.LOG PHOTO3.ERR
pcphoto -r senderl3.asr -c sender.ppp -a rev85.apr -o D -s 1.000 senderl3.plt 
if exist PHOTO4.ERR del PHOTO4.ERR
if exist PCPHOTO.LOG ren PCPHOTO.LOG PHOTO4.ERR
pcphoto -r senderl4.asr -c sender.ppp -a rev85.apr -o D -s 1.000 senderl4.plt 
if exist PHOTO5.ERR del PHOTO5.ERR
if exist PCPHOTO.LOG ren PCPHOTO.LOG PHOTO5.ERR
pcphoto -r sendersm.asr -c sender.ppp -a rev85.apr -o D -s 1.000 sendersm.plt 
if exist PHOTO6.ERR del PHOTO6.ERR
if exist PCPHOTO.LOG ren PCPHOTO.LOG PHOTO6.ERR
pcphoto -r sendersk.asr -c sender.ppp -a rev85.apr -o D -s 1.000 sendersk.plt 
if exist PHOTO7.ERR del PHOTO7.ERR
if exist PCPHOTO.LOG ren PCPHOTO.LOG PHOTO7.ERR
pcphoto -r senderfb.asr -c sender.ppp -a rev85.apr -o D -s 1.000 senderfb.plt 
if exist PHOTO8.ERR del PHOTO8.ERR
if exist PCPHOTO.LOG ren PCPHOTO.LOG PHOTO8.ERR
pcplots senderfb.plt -c sender.ppp -a rev85.apr -o D -s 1.000
if exist PLOTS8.ERR del PLOTS8.ERR
if exist PCPLOTS.LOG ren PCPLOTS.LOG PLOTS8.ERR
pcplots senderas.plt -c sender.ppp -a rev85.apr -o D -s 1.000
if exist PLOTS9.ERR del PLOTS9.ERR
if exist PCPLOTS.LOG ren PCPLOTS.LOG PLOTS9.ERR
HPGL2DXF SENDERFB.HP
HPGL2DXF SENDERAS.HP

