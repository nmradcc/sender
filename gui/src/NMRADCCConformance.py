#!/usr/bin/python

import sys
import serial
import glob
from Tkinter import *
from ttk import *

# Get a list of available serial ports
def serial_ports():
    """Lists serial ports

    :raises EnvironmentError:
        On unsupported or unknown platforms
    :returns:
        A list of available serial ports
    """
    if sys.platform.startswith('win'):
        ports = ['COM' + str(i + 1) for i in range(256)]

    elif sys.platform.startswith('linux'):
        # this is to exclude your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')

    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/cu.*')

    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result

# The main application window
class App:
    #initialize main window
    def __init__(self, master):
        master.title("DCC Conformance Tool")
        frame = Frame(master)
        frame.pack(fill=BOTH, expand=1)

        #frame.columnconfigure(1, weight=1)
        #frame.columnconfigure(2, weight=3)
        #frame.columnconfigure(3, weight=1)
        #frame.rowconfigure(1, weight=10)
        #frame.rowconfigure(2, weight=1)
        
        self.exit = Button(frame, text="Quit", command=frame.quit)
        self.exit.grid(row=7, column=2, columnspan=2, sticky=S+E, padx=5, pady=5)

        Label(frame, text="Serial Port").grid(row=0, column=0, sticky=S+W, padx=10)
        self.box_value = StringVar()
        self.serial_port = Combobox(frame, textvariable=self.box_value, state="readonly")
        self.serial_port.bind('<<ComboboxSelected>>', self.port_select)
        self.serial_port['values'] = (serial_ports())
        self.serial_port.grid(row=1, column=0, sticky=W+E, padx=5, pady=(0,20))
        self.serial_port.current(None)
        self.ser = None
        
        Label(frame, text="Manufacturer").grid(row=2, column=0, sticky=S+W, padx=10)
        self.manufacturer = Combobox(frame, textvariable=StringVar(), state="readonly")
        self.manufacturer.bind('<<ComboboxSelected>>', self.manufacturer_select)
        self.manufacturer['values'] = ('[Other]', 'Atlas', 'Bachmann', 'BLI', 'CT Elektronik', 'Digitrax', 'ESU', 'Hornby', 'Lenz', 'MRC', 'NCE', 'QSI', 'Roco', 'Soundtraxx', 'Tams', 'TCS', 'Uhlenbrock', 'Zimo', 'ZTC')
        self.manufacturer.current(None)
        self.manufacturer.grid(row=3, column=0, sticky=W+E, padx=5)
        
        self.manu_other_style = Style()
        self.manu_other_style.configure('Grey.TEntry', background="light grey")
        self.manu_other = Entry(frame, state=DISABLED)
        self.manu_other["style"] = "Grey.TEntry"
        self.manu_other.grid(row=4, column=0, sticky=W+E, padx=6, pady=(2,20))
        
        Label(frame, text="Model Number").grid(row=0, column=1, sticky=W, padx=10)
        self.model = Entry(frame)
        self.model.grid(row=1, column=1, sticky=E+W, padx=5, pady=(0,20))

        Label(frame, text="Serial Number").grid(row=2, column=1, sticky=S+W, padx=10)
        self.serial_num = Entry(frame)
        self.serial_num.grid(row=3, column=1, sticky=E+W, padx=5)

        Label(frame, text="Comments").grid(row=0, column=2, sticky=S+W, padx=10)
        self.comments = Text(frame, height=7, relief=SUNKEN, yscrollcommand=1)
        self.comments.grid(row=1, column=2, rowspan=4, sticky=N+E+W, padx=(7,0), pady=(2,0))
        self.comments_scroll = Scrollbar(frame, command=self.comments.yview)
        self.comments_scroll.grid(row=1, column=3, rowspan=4, sticky=N+S+W, padx=(0,5), pady=(2,27))
        self.comments['yscrollcommand'] = self.comments_scroll.set

        self.test1val = IntVar()
        self.test1 = Checkbutton(frame, text="test1", variable=self.test1val, state=['!selected'])
        self.test1.grid(row=1, column=4)
        #self.test1.state(['!selected'])

        Label(frame, text="Diagnostic Log").grid(row=5, column=0, sticky=S+W, padx=10)
        self.log = Text(frame, bg="light grey", height=10, relief=SUNKEN,
                        state=DISABLED, yscrollcommand=1)
        self.log.grid(row=6, columnspan=3, sticky=S+E+W, padx=(7,0))
        self.log_scroll = Scrollbar(frame, command=self.log.yview)
        self.log_scroll.grid(row=6, column=3, sticky=N+S+W, padx=(0,5))
        self.log['yscrollcommand'] = self.log_scroll.set

    # open a serial port, close previously opened serial port as needed
    def port_select(self, event):
        if (self.ser != None):
            if (self.ser.name == self.serial_port.get()):
                # serial port is already open
                return
            # close previously open serial port
            self.log_print('Closed port "' + self.ser.name + '"\n')
            del self.ser
            self.ser = None
        # open serial port
        self.ser = serial.Serial(self.serial_port.get(), 115200, timeout=1)
        if (self.ser != None):
            self.log_print('Opened port "' + self.serial_port.get() + '"\n')
        else:
            self.log_print('Unable to open serial port\n')

    # Manufacture selected
    def manufacturer_select(self, event):
        self.log_print('Manufacturer "' + self.manufacturer.get() + '" selected\n')
        if (self.manufacturer.get() == '[Other]'):
            self.manu_other["style"] = "TEntry"
            self.manu_other.configure(state=NORMAL)
            return
        self.manu_other["style"] = "Grey.TEntry"
        self.manu_other.configure(state=DISABLED)

    # print to the log
    def log_print(self, text):
        self.log.configure(state=NORMAL)
        self.log.insert(END, text)
        self.log.configure(state=DISABLED)

# Start of the application
root = Tk()
#root.geometry("800x600")

app = App(root)

root.mainloop()
