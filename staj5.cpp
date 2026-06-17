#include <stdio.h>
#include <stdlib.h>                      // bunlar c dili içim kullanýlýyor
#include <stdint.h>
#include <time.h>
#include <math.h>
#ifdef _WIN32
#include <windows.h>
#define DELAY_MS(ms) Sleep(ms)
#else
#include <unistd.h>
#define DELAY_MS(ms) usleep(ms * 1000)                    // þu kýsýmlarý chatcbt den yardým aldým staj zamanýnda mentorum bu uygulamadan yapmamý istemiþti
#endif

#define SIZE 3          // Buffer boyutu olna burasý
#define SAMPLE_COUNT 10 // Her BLE gönderimi öncesi alýnacak örnek sayýsýdýr

typedef enum {
    SENSOR_TEMP,
    SENSOR_HUM,                        // bunlar benden iistenilen sensorlerdir
    SENSOR_CO2
} sensor_t;

typedef struct {
    float buffer[SIZE];
    int index;
} sensor_data_t;
                                                  
sensor_data_t temp_sensor = {{0}, 0};
sensor_data_t hum_sensor = {{0}, 0};                      // Global sensör yapýlarý vardýr onlarý ekledim zaten statik durumlarý da içeriyor
sensor_data_t co2_sensor = {{0}, 0};

float read_sensor(sensor_t type);
void buffer_add(sensor_data_t *s, float value);
float median(float arr[], int n);
void sort(float arr[], int n);
void calculate_stats(sensor_data_t *s, float *min, float *max, float *med);
void ble_send(void);

float read_sensor(sensor_t type) {
    switch(type) {
        case SENSOR_TEMP: return (float)(rand() % 40);
        case SENSOR_HUM:  return (float)(rand() % 100);                               // Rastgele veri üreten sensör okuma fonksiyonudur 
        case SENSOR_CO2:  return (float)(rand() % 1000);
        default: return 0.0f;
    }
}


void buffer_add(sensor_data_t *s, float value) {
    if (s == NULL) return;                              // Güvenlik kontrolü burada yapýlýyor
    s->buffer[s->index] = value;
    s->index = (s->index + 1) % SIZE;
}

void sort(float arr[], int n) {
    for(int i = 0; i < n - 1; i++) {
        for(int j = 0; j < n - i - 1; j++) {                    // Kabarcýk sýralama burada ve chatden yardým aldým
            if(arr[j] > arr[j + 1]) {
                float t = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = t;
            }
        }
    }
}

float median(float arr[], int n) {
    float temp[SIZE];
    for(int i = 0; i < n; i++) temp[i] = arr[i];
    sort(temp, n);                                              // Medyan hesaplama tam ollarak burada yapýlýyor
    if(n % 2 == 0) return (temp[n/2 - 1] + temp[n/2]) / 2.0f;
    else return temp[n/2];
}

void calculate_stats(sensor_data_t *s, float *min, float *max, float *med) {
    *min = s->buffer[0];
    *max = s->buffer[0];
    for(int i = 0; i < SIZE; i++) {
        if(s->buffer[i] < *min) *min = s->buffer[i];
        if(s->buffer[i] > *max) *max = s->buffer[i];
    }
    *med = median(s->buffer, SIZE);
}

void ble_send(void) {
    float t_min, t_max, t_med;
    float h_min, h_max, h_med;
    float c_min, c_max, c_med;

    calculate_stats(&temp_sensor, &t_min, &t_max, &t_med);
    calculate_stats(&hum_sensor, &h_min, &h_max, &h_med);
    calculate_stats(&co2_sensor, &c_min, &c_max, &c_med);

    printf("\n--- BLE ---\n");
    printf("TEMP | Min: %.1f | Max: %.1f | Med: %.1f\n", t_min, t_max, t_med);
    printf("HUM  | Min: %.1f | Max: %.1f | Med: %.1f\n", h_min, h_max, h_med);
    printf("CO2  | Min: %.1f | Max: %.1f | Med: %.1f\n", c_min, c_max, c_med);
    printf("----------------------------\n");
}

int main() {
    srand((unsigned int)time(NULL));
    int sample_counter = 0;

    printf("Sistem baslatildi...\n");

    while(1) {
        
        buffer_add(&temp_sensor, read_sensor(SENSOR_TEMP));
        buffer_add(&hum_sensor, read_sensor(SENSOR_HUM));                                       // Sensörlerden veriler burada okunuyor ve buffer'a ekleniyor
        buffer_add(&co2_sensor, read_sensor(SENSOR_CO2));

        sample_counter++;

                                                                          
        if(sample_counter >= SAMPLE_COUNT) {                                       // Her 10 örnekte bir  BLE gönderimi yapýlýyor
            ble_send();
            sample_counter = 0;
        }

        DELAY_MS(1000); //           istenilen 1 saniye bekleme buraya ekledim  
    }
    return 0;
}

