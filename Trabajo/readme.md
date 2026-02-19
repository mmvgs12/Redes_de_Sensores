# Detector de Caídas con Arduino Nano 33 BLE

Sistema de detección de caídas utilizando la placa Arduino Nano 33 BLE y su unidad de medición inercial (IMU) integrada (acelerómetro y giroscopio).

---

Este proyecto implementa un algoritmo básico de detección de caídas basado en el análisis de:

- Aceleración en los tres ejes (X, Y, Z)
- Velocidad angular en los tres ejes
- Magnitud total del vector de aceleración
- Periodo de inactividad posterior al impacto

El sistema puede utilizarse en aplicaciones como:

- Monitorización de personas mayores
- Wearables de asistencia
- Seguridad industrial
- Prototipos IoT de emergencia

La forma de detectar las caidas es el siguiente:
- En estado de reposo la aceleracion en uno de los ejes es 1G, dependiendo de la orientacion del objeto.
- En caso de movimiento de una persona, la suma vectorial de los 3 ejes deberia ser similar a 1G. Los cambios bruscos de aceleracion pueden detectarse mediante el giroscopio.
- En caso de caida, la aceleración pasaria por una fase de "ingravidez" donde la aceleracion bajaria bruscamente y se produciria un cambio repentino en la orientacion
- Tras el impacto se produciria un cambio brusco en la aceleración (>1G) y un cambio repentino en la orientacion.
- El algoritmo del arduino detecta estos cambios, tanto en la aceleracion y en la orientacion y los procesa, emitiendo un aviso si se superan unos umbrales.

Para ello el algoritmo deberia funcionar de la siguiente manera:
- Tomar medidas periodicamente de la aceleracion y la orientacion. Por ejemplo cada 10 ms mediante un timer (Se puede ajustar para tomar mas o menos medidas)
- Calcular el valor absoluto de la aceleracion.
- Si se supera un umbral, comprobar un periodo de inactividad posterior para corroborar la caida.
- Si se confirma que se ha producido una caida se manda una notificacion por Bluetooth avisando de una posible caida.

Necesario:
- Timer
- Comunicacion SPI/I2C
- Conexion BLE






