import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import threading

# --- Estado interno ---
_stats_history = {
    "x": {"promedio": [], "std": []},
    "y": {"promedio": [], "std": []},
    "z": {"promedio": [], "std": []},
}
_ventanas = []
_fig = None
_axes = None
_ani = None
_duracion = None
_tiempo_inicio = None
_lock = threading.Lock()
_finalizado = False  # flag para detener _actualizar


def iniciar(tiempo_inicio, duracion_acumulacion, intervalo_ms=50):
    global _duracion, _tiempo_inicio, _finalizado

    _duracion = duracion_acumulacion
    _tiempo_inicio = tiempo_inicio
    _finalizado = False

    hilo = threading.Thread(target=_lanzar_grafica, args=(intervalo_ms,), daemon=False)
    hilo.start()


def agregar_dato(x, y, z, timestamp):
    global _tiempo_inicio

    with _lock:
        if _finalizado:
            return

        # Acumular temporalmente para la ventana actual
        _buffer["x"].append(x)
        _buffer["y"].append(y)
        _buffer["z"].append(z)
        _buffer["ts"].append(timestamp)

        elapsed = timestamp - _tiempo_inicio

        if elapsed >= _duracion:
            _calcular_stats(_buffer)
            for key in _buffer:
                _buffer[key].clear()
            _tiempo_inicio += _duracion


# Buffer temporal solo para acumular dentro de la ventana
_buffer = {"x": [], "y": [], "z": [], "ts": []}


def finalizar():
    """
    Llamado desde main.py al terminar el bucle.
    Procesa datos residuales, para la animación y deja la ventana abierta.
    """
    global _finalizado

    with _lock:
        # Procesar datos residuales de la última ventana parcial
        if _buffer["x"]:
            _calcular_stats(_buffer)
        for key in _buffer:
            _buffer[key].clear()
        _finalizado = True  # flag: _actualizar dejará de redibujar

    print("[INFO] Captura finalizada. Cierre la ventana para salir.")


def _calcular_stats(buf):
    n = len(buf["ts"])
    if n == 0:
        return

    ventana = len(_ventanas) + 1
    _ventanas.append(ventana)

    for eje in ("x", "y", "z"):
        promedio = np.mean(buf[eje])
        desviacion = np.std(buf[eje], ddof=1) if n > 1 else 0.0
        _stats_history[eje]["promedio"].append(promedio)
        _stats_history[eje]["std"].append(desviacion)

    intervalo_medio = np.mean(np.diff(buf["ts"])) * 1000 if n > 1 else 0
    print(f"[Ventana {ventana}] N={n} | Intervalo medio: {intervalo_medio:.1f} ms")
    for eje in ("x", "y", "z"):
        p = _stats_history[eje]["promedio"][-1]
        s = _stats_history[eje]["std"][-1]
        print(f"  Eje {eje.upper()} → Promedio: {p:.4f} | Std: {s:.4f}")


def _lanzar_grafica(intervalo_ms):
    global _fig, _axes, _ani

    _fig, _axes = plt.subplots(2, 1, figsize=(12, 8))
    _fig.suptitle("Análisis — Ejes X, Y, Z", fontsize=14)

    _ani = animation.FuncAnimation(
        _fig, _actualizar, interval=intervalo_ms, cache_frame_data=False
    )
    plt.show()


def _actualizar(frame):
    with _lock:
        finalizado_local = _finalizado
        buffer_local = {k: list(v) for k, v in _buffer.items()}
        stats_local = {
            eje: {k: list(v) for k, v in _stats_history[eje].items()}
            for eje in _stats_history
        }
        ventanas_local = list(_ventanas)
        t_inicio_local = _tiempo_inicio

    # --- Si ha finalizado, dejar de redibujar ---
    if finalizado_local:
        _ani.event_source.stop()  # detiene FuncAnimation desde el hilo de la UI
        _dibujar_historico_final(stats_local, ventanas_local)
        return

    colores = {"x": "steelblue", "y": "seagreen", "z": "tomato"}

    # --- Subplot 1: los 3 ejes en tiempo real ---
    ax_live = _axes[0]
    ax_live.cla()
    if buffer_local["ts"]:
        t_rel = [t - t_inicio_local for t in buffer_local["ts"]]
        elapsed = t_rel[-1]
        for eje in ("x", "y", "z"):
            ax_live.plot(t_rel, buffer_local[eje], color=colores[eje],
                         linewidth=1.5, label=f"Eje {eje.upper()}")
        ax_live.set_xlim(0, _duracion)
        ax_live.set_title(f"Datos en tiempo real — ({elapsed:.1f}s / {_duracion}s)")
    else:
        ax_live.set_title("Esperando datos...")
    ax_live.set_xlabel("Tiempo (s)")
    ax_live.set_ylabel("Valor")
    ax_live.legend(loc="upper right")
    ax_live.grid(True, alpha=0.3)

    # --- Subplot 2: histórico ---
    ax_hist = _axes[1]
    ax_hist.cla()
    if ventanas_local:
        for eje in ("x", "y", "z"):
            ax_hist.errorbar(ventanas_local,
                             stats_local[eje]["promedio"],
                             yerr=stats_local[eje]["std"],
                             fmt="o-", color=colores[eje],
                             ecolor=colores[eje], capsize=5,
                             linewidth=2, alpha=0.7,
                             label=f"Eje {eje.upper()} ± Std")
        ax_hist.set_title("Histórico por ventana")
        ax_hist.legend(loc="upper right")
    else:
        ax_hist.set_title("Histórico (esperando primera ventana...)")
    ax_hist.set_xlabel("Ventana")
    ax_hist.set_ylabel("Valor")
    ax_hist.grid(True, alpha=0.3)

    _fig.tight_layout(rect=[0, 0, 1, 0.95])


def _dibujar_historico_final(stats_local, ventanas_local):
    """Dibuja solo el histórico final, ocupa toda la figura."""
    colores = {"x": "steelblue", "y": "seagreen", "z": "tomato"}

    # Ocultar subplot de tiempo real y expandir el histórico
    _axes[0].set_visible(False)
    _axes[1].set_position([0.1, 0.1, 0.85, 0.75])  # ocupa toda la figura

    ax_hist = _axes[1]
    ax_hist.cla()
    if ventanas_local:
        for eje in ("x", "y", "z"):
            ax_hist.errorbar(ventanas_local,
                             stats_local[eje]["promedio"],
                             yerr=stats_local[eje]["std"],
                             fmt="o-", color=colores[eje],
                             ecolor=colores[eje], capsize=5,
                             linewidth=2, alpha=0.7,
                             label=f"Eje {eje.upper()} ± Std")
    ax_hist.set_title("Histórico final por ventana", fontsize=13)
    ax_hist.set_xlabel("Ventana")
    ax_hist.set_ylabel("Valor")
    ax_hist.legend(loc="upper right")
    ax_hist.grid(True, alpha=0.3)
    _fig.suptitle("Captura finalizada — Ejes X, Y, Z", fontsize=14)
    _fig.canvas.draw()