import time
import serial
import serial.tools.list_ports
import show_data
from pathlib import Path
 
# Definicion de variables
PORT = "COM5"
BAUDRATE = 19200
OUTPUT_FILE = "Saved_measures.txt"
TIME_READING = 25
FILE_HEADER = "numero de mensaje;eje x;eje y;eje z"
 
 
def port_exists(port: str):
    #Comprueba si el puerto COM especificado está disponible.
    return any(d.device == port for d in serial.tools.list_ports.comports())
 
 
def read_line(ser: serial.Serial):
    try:
        return ser.readline().decode("UTF-8").strip().strip("[]").replace(",", ";")
    except serial.SerialException as e:
        print(f"[ERROR] Fallo al leer del serial: {e}")
        return None
 
 
def save_line(line: str, filepath: str = OUTPUT_FILE) -> None:
    path = Path(filepath)
    #Comprueba si el archivo existe y si tiene algo escrito dentro, si no existe o esta vacio, escribe el header
    write_header = not path.exists() or path.stat().st_size == 0
 
    with open(path, "a") as f:
        if write_header:
            f.write(FILE_HEADER + "\n")
        f.write(line + "\n")
 
 
def main():
    if not port_exists(PORT):
        print(f"[ERROR] El puerto {PORT} no está disponible.")
        return
 
    try:
        with serial.Serial(port=PORT, baudrate=BAUDRATE) as ser:
            print(f"[INFO] Conectado a {PORT} a {BAUDRATE} baud.")
            start_time = time.time()
            elapsed_time=time.time()-start_time
            show_data.iniciar(
                tiempo_inicio=start_time,
                duracion_acumulacion=TIME_READING
                )
            i=0
            while (elapsed_time<TIME_READING):
                print(f"Lectura {i}")
                line = read_line(ser)
                ts = time.time()
                if line:
                    saved_line = str(i)+ ";" + line.replace(".",",")
                    save_line(saved_line)
                    try:
                        partes = line.split(";")
                        x, y, z = float(partes[0]), float(partes[1]), float(partes[2])
                        show_data.agregar_dato(x, y, z, ts)
                    except (ValueError, IndexError):
                        print(f"[WARN] No se pudo parsear: {line}")
                elapsed_time=time.time()-start_time
                print(elapsed_time)
                i+=1
            show_data.finalizar()
    except serial.SerialException as e:
        print(f"[ERROR] No se pudo abrir el puerto {PORT}: {e}")
 
 
if __name__ == "__main__":
    main()