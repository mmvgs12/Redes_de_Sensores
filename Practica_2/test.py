import serial
import serial.tools.list_ports
import time
import icecream as ic


baudrate = 19200
Port = 'COM2'


def PortExists(DesPort='COM1'):
    
    ports_list = [port.device for port in serial.tools.list_ports.comports()]

    return DesPort in ports_list

def SaveLine (List_name = "Save.txt"):
    return       
    #with  open (List_name, 'a') as messages_list:
                  

     
def main():
    print(PortExists("COM7"))

    if PortExists("COM7"):
        try:
            ser = serial.Serial(port="COM7", baudrate = 19200)
        except:
            print("Error, no se ha podido crear el puerto")

        for i in range (0,100,1):
            ln = ser.readline().strip()
            ln_dec=ln.decode("UTF-8")
            Axis = ln_dec.strip("[]").split(", ")
            print("muestra numero " + str(i))
            print(Axis[0])
            print(Axis[1])
            print(Axis[2])

    else:
            print("Error, el puerto no existe")


if __name__ == "__main__":
    main()        




