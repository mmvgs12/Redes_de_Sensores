/* Edge Impulse ingestion SDK
 * Copyright (c) 2022 EdgeImpulse Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

/* Includes ---------------------------------------------------------------- */
#include <motion-classification_inferencing.h>
#include <Arduino_LSM9DS1.h> //Click here to get the library: https://www.arduino.cc/reference/en/libraries/arduino_lsm9ds1/

/* Constant defines -------------------------------------------------------- */
#define CONVERT_G_TO_MS2    9.80665f
#define MAX_ACCEPTED_RANGE  2.0f
#define CONFIDENCE_THRESH   0.70f   // umbral mínimo de confianza

/*
 ** NOTE: If you run into TFLite arena allocation issue.
 **
 ** This may be due to may dynamic memory fragmentation.
 ** Try defining "-DEI_CLASSIFIER_ALLOCATION_STATIC" in boards.local.txt (create
 ** if it doesn't exist) and copy this file to
 ** `<ARDUINO_CORE_INSTALL_PATH>/arduino/hardware/<mbed_core>/<core_version>/`.
 **
 ** See
 ** (https://support.arduino.cc/hc/en-us/articles/360012076960-Where-are-the-installed-cores-located-)
 ** to find where Arduino installs cores on your machine.
 **
 ** If the problem persists then there's not enough memory for this model and application.
 */

/* Private variables ------------------------------------------------------- */
static bool debug_nn = false; // Set this to true to see e.g. features generated from the raw signal

/**
 * @brief      Establece el color del LED RGB
 *             LED de ánodo común: valor invertido (255 - valor)
 *
 * @param[in]  r  Componente rojo (0-255)
 * @param[in]  g  Componente verde (0-255)
 * @param[in]  b  Componente azul (0-255)
 */
void setColor(uint8_t r, uint8_t g, uint8_t b)
{
    analogWrite(LEDR, 255 - r);
    analogWrite(LEDG, 255 - g);
    analogWrite(LEDB, 255 - b);
}

/**
 * @brief      Apaga el LED RGB
 */
void ledOff()
{
    digitalWrite(LEDR, HIGH);
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDB, HIGH);
}

/**
 * @brief      Mapea cada clase detectada a un color del LED
 *               roll_pos  → Rojo
 *               roll_neg  → Rosa
 *               pitch_pos → Verde
 *               pitch_neg → Amarillo
 *               yaw_pos   → Azul
 *               yaw_neg   → Morado
 *               idle      → Apagado
 *
 * @param[in]  label  Etiqueta de la clase detectada
 */
void setColorForClass(const char* label)
{
    String l = String(label);
    l.toLowerCase();
    l.trim();

    if      (l == "roll_pos")  setColor(255,   0,   0);  // Rojo
    else if (l == "roll_neg")  setColor(255, 105, 180);  // Rosa
    else if (l == "pitch_pos") setColor(  0, 255,   0);  // Verde
    else if (l == "pitch_neg") setColor(255, 255,   0);  // Amarillo
    else if (l == "yaw_pos")   setColor(  0,   0, 255);  // Azul
    else if (l == "yaw_neg")   setColor(128,   0, 128);  // Morado
    else                       ledOff();                  // idle / desconocido
}

/**
 * @brief      Arduino setup function
 */
void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    // comment out the below line to cancel the wait for USB connection (needed for native USB)
    while (!Serial);
    Serial.println("Edge Impulse Inferencing Demo");

    // Configura los LEDs RGB (apagados al inicio)
    pinMode(LEDR, OUTPUT); digitalWrite(LEDR, HIGH);
    pinMode(LEDG, OUTPUT); digitalWrite(LEDG, HIGH);
    pinMode(LEDB, OUTPUT); digitalWrite(LEDB, HIGH);

    if (!IMU.begin()) {
        ei_printf("Failed to initialize IMU!\r\n");
    }
    else {
        ei_printf("IMU initialized\r\n");
    }

    // Comprueba que el modelo espera 9 muestras por frame
    // (acelerómetro + giróscopo + magnetómetro)
    if (EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME != 9) {
        ei_printf("ERR: EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME should be equal to 9 (acc + gyr + mag)\n");
        return;
    }
}

/**
 * @brief Return the sign of the number
 * 
 * @param number 
 * @return int 1 if positive (or 0) -1 if negative
 */
float ei_get_sign(float number) {
    return (number >= 0.0) ? 1.0 : -1.0;
}

/**
 * @brief      Get data and run inferencing
 *
 * @param[in]  debug  Get debug info if true
 */
void loop()
{
    ei_printf("\nStarting inferencing in 0.2 seconds...\n");

    delay(200);

    ei_printf("Sampling...\n");

    // Allocate a buffer here for the values we'll read from the IMU
    float buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = { 0 };

    for (size_t ix = 0; ix < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; ix += 9) {
        // Determine the next tick (and then sleep later)
        uint64_t next_tick = micros() + (EI_CLASSIFIER_INTERVAL_MS * 1000);

        // Lee acelerómetro (ejes 0,1,2)
        IMU.readAcceleration(buffer[ix], buffer[ix + 1], buffer[ix + 2]);

        // Limita el rango del acelerómetro
        for (int i = 0; i < 3; i++) {
            if (fabs(buffer[ix + i]) > MAX_ACCEPTED_RANGE) {
                buffer[ix + i] = ei_get_sign(buffer[ix + i]) * MAX_ACCEPTED_RANGE;
            }
        }

        // Convierte acelerómetro de G a m/s²
        buffer[ix + 0] *= CONVERT_G_TO_MS2;
        buffer[ix + 1] *= CONVERT_G_TO_MS2;
        buffer[ix + 2] *= CONVERT_G_TO_MS2;

        // Lee giróscopo (ejes 3,4,5)
        IMU.readGyroscope(buffer[ix + 3], buffer[ix + 4], buffer[ix + 5]);

        // Lee magnetómetro (ejes 6,7,8)
        IMU.readMagneticField(buffer[ix + 6], buffer[ix + 7], buffer[ix + 8]);

        delayMicroseconds(next_tick - micros());
    }

    // Turn the raw buffer in a signal which we can the classify
    signal_t signal;
    ei_printf("Buffer primeros 9 valores:\n");
    for (int i = 0; i < 9; i++) {
        ei_printf("  [%d]: %.3f\n", i, buffer[i]);
    }
    int err = numpy::signal_from_buffer(buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
    if (err != 0) {
        ei_printf("Failed to create signal from buffer (%d)\n", err);
        return;
    }

    // Run the classifier
    ei_impulse_result_t result = { 0 };

    err = run_classifier(&signal, &result, debug_nn);
    if (err != EI_IMPULSE_OK) {
        ei_printf("ERR: Failed to run classifier (%d)\n", err);
        return;
    }

    // print the predictions
    ei_printf("Predictions ");
    ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",
        result.timing.dsp, result.timing.classification, result.timing.anomaly);
    ei_printf(": \n");
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        ei_printf("    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);
    }
#if EI_CLASSIFIER_HAS_ANOMALY == 1
    ei_printf("    anomaly score: %.3f\n", result.anomaly);
#endif

    // Encuentra la clase con mayor confianza y enciende el LED del color correspondiente
    float  best_val   = 0.0f;
    const char* best_label = "idle";

    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        if (result.classification[ix].value > best_val) {
            best_val   = result.classification[ix].value;
            best_label = result.classification[ix].label;
        }
    }

    ei_printf(">>> Clase detectada: %s (%.3f)\n", best_label, best_val);

    if (best_val >= CONFIDENCE_THRESH) {
        setColorForClass(best_label);
    } else {
        ledOff();  // confianza baja → LED apagado
    }
}

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_FUSION
#error "Invalid model for current sensor"
#endif