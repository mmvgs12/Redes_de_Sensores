import serial
import serial.tools.list_ports
import time
import icecream as ic


baudrate = 19200
Port = 'COM2'


def PortExists(DesPort='COM5'):
    
    ports_list = [port.device for port in serial.tools.list_ports.comports()]
    #return ports_list
    return DesPort in ports_list


def ReadFromSerial(ser, Port = "COM7"):
    print(PortExists(Port))
    if PortExists(Port):
        try:
            ln = ser.readline().decode("UTF-8").strip().strip("[]").replace(",",";")
            print(ln)
            return ln
        except:
            print("fallo al leer del serial")
        
    else:
        print("Error, el puerto no existe")


def AddtoList(List_name = "Save.txt", line = None):

    with open(List_name, 'a') as list:
        if not list.readline(0).startswith("numero"):

            list.write("numero de mensaje // eje x // eje y // eje z")
        else:
            list.write(line)



     
def main():
    print(PortExists("COM5"))
    try:
       with serial.Serial(port="COM5", baudrate = 19200) as ser:
           for i in range (0,100,1):
                print("lectura numero: " + str(i))
                ln=ReadFromSerial(ser)
                AddtoList(ln)
    except:
        print("Error, no se ha podido crear el puerto")

    
            
    

    #AddtoList("Test.txt")

if __name__ == "__main__":
    main()        




