from dataclasses import dataclass
from pickle import TRUE
import serial
import time
header = { "Botellas": b"T", "Totales": 00, "Botellas:": b"F","Fabricadas": 00, "Botellas ": b"D","Desechadas": 00}
def readInt4File(f,size=2,sign=False):
    raw=f.read(1)
    while( len(raw) < size):
        raw+=f.read(1)
    return (int.from_bytes(raw,"little",signed=sign))
def ReadData(f,h):
    find=False
    while(not find):
        data=bytearray(len(h["Botellas"]))
        while data!=h["Botellas"]:
            data+=f.read(1)
            del data[0]

        h["Totales"]=readInt4File(f,4)
        h["Botellas:"]=f.read(1)
        h["Fabricadas"]=readInt4File(f,4)
        h["Botellas "]=f.read(1)
        h["Desechadas"]=readInt4File(f,4)
        find = True
    print(h)
    time.sleep(9)

streamFile = serial.Serial('COM12',baudrate=115200,timeout=2)

for i in range(100):
    streamFile.flushInput()
    ReadData(streamFile,header)
