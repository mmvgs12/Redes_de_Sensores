#ifndef WEBPAGE_H
#define WEBPAGE_H

const char index_html[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <title>Reloj ESP32-C6</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {
      font-family: Helvetica, Arial, sans-serif;
      text-align: center;
    }
    h1 { color: #1a237e; padding: 2vh; }
    p  { font-size: 2rem; }
    .btn-reset {
      background-color: #e53935;
      color: white;
      border: none;
      border-radius: 8px;
      padding: 14px 36px;
      font-size: 1.2rem;
      cursor: pointer;
    }
    .btn-reset:hover { background-color: #b71c1c; }
  </style>
</head>
<body>
  <h1>Reloj ESP32-C6</h1>
  <p>Hora actual: <strong id="hora">HORA_ACTUAL</strong></p>
  <p><button class="btn-reset" onclick="resetHora()">Resetear a 0:00</button></p>

  <script>
    // Pide la hora actual al ESP32 cada segundo
    function actualizarHora() {
      fetch('/hora')
        .then(r => r.text())
        .then(h => document.getElementById('hora').innerText = h);
    }

    // Llama al reset y luego sigue actualizando
    function resetHora() {
      fetch('/reset')
        .then(() => actualizarHora());
    }

    // Actualiza cada 1 segundo
    setInterval(actualizarHora, 1000);
  </script>
</body>
</html>
)rawhtml";

#endif