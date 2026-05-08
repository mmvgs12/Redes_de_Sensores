import time
import socket
import show_data
from pathlib import Path

# ——— Configuración TCP ———
SERVER_HOST = "10.189.115.97"   # Escucha en todas las interfaces
SERVER_PORT = 5000

# ——— Configuración general ———
OUTPUT_FILE     = "Saved_measures.txt"
TIME_READING    = 25
FILE_HEADER     = "numero de mensaje;timestamp;eje x;eje y;eje z"


def save_line(line: str, filepath: str = OUTPUT_FILE) -> None:
    path = Path(filepath)
    write_header = not path.exists() or path.stat().st_size == 0
    with open(path, "a") as f:
        if write_header:
            f.write(FILE_HEADER + "\n")
        f.write(line + "\n")


def main():
    # ——— Crear servidor TCP ———
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind((SERVER_HOST, SERVER_PORT))
    server.listen(1)
    print(f"[INFO] Servidor TCP escuchando en {SERVER_HOST}:{SERVER_PORT}")
    print(f"[INFO] Esperando conexión del ESP32...")

    conn, addr = server.accept()
    print(f"[INFO] ESP32 conectado desde {addr}")

    try:
        start_time = time.time()
        elapsed_time = 0

        show_data.iniciar(
            tiempo_inicio=start_time,
            duracion_acumulacion=TIME_READING
        )

        # Arrancar el envío en el ESP32
        conn.sendall(b"START\n")
        print("[INFO] Comando START enviado al ESP32")

        i = 0
        buffer = ""

        while elapsed_time < TIME_READING:
            # Leer chunk del socket
            try:
                conn.settimeout(2.0)
                chunk = conn.recv(1024).decode("UTF-8")
                if not chunk:
                    print("[WARN] Conexión cerrada por el ESP32")
                    break
                buffer += chunk
            except socket.timeout:
                print("[WARN] Timeout esperando datos")
                elapsed_time = time.time() - start_time
                continue
            except OSError as e:
                print(f"[ERROR] Fallo en la conexión: {e}")
                break

            # Procesar líneas completas del buffer
            while "\n" in buffer:
                line, buffer = buffer.split("\n", 1)
                line = line.strip()
                if not line:
                    continue

                ts = time.time()
                elapsed_time = ts - start_time

                print(f"[{i}] {line}")

                try:
                    # Formato esperado: "YYYY-MM-DD HH:MM:SS,x,y,z"
                    partes = line.split(",")
                    timestamp_str = partes[0]
                    x = float(partes[1])
                    y = float(partes[2])
                    z = float(partes[3])

                    # Guardar en fichero con ; como separador
                    saved_line = f"{i};{timestamp_str};{str(x).replace('.',',')};{str(y).replace('.',',')};{str(z).replace('.',',')}"
                    save_line(saved_line)

                    show_data.agregar_dato(x, y, z, ts)

                except (ValueError, IndexError):
                    print(f"[WARN] No se pudo parsear: {line}")

                i += 1

        # Parar el envío
        conn.sendall(b"STOP\n")
        print("[INFO] Comando STOP enviado al ESP32")

    finally:
        show_data.finalizar()
        conn.close()
        server.close()
        print("[INFO] Conexión y servidor cerrados.")


if __name__ == "__main__":
    main()