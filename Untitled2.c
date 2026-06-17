#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>                              // þimdi bunlarýn zaten çalýþmasýný saðlayanaraçlardýr bunlar olmadan iþlem yapýlamaz// 
#include <time.h>
#include <windows.h>
#include <math.h>

#define SIZE 3                             // her sensör 4 veri yakalýyor//
#define BLE_INTERVAL 10                // her 10 saniyede veri gönderiyor//


typedef enum {
    SENSOR_TEMP,
    SENSOR_HUM,                         // burada bende istenilen pdf de üç tane temp,hum,ve co2 var sensör olarak//
    SENSOR_CO2
} sensor_t;


typedef struct {
    float buffer[SIZE];     // burada buffer bana 4 ölçümü veriyor ama index nereye ne þekilde yazacaðýmý fösteriyor//
    int index;
} sensor_data_t;

sensor_data_t temp_sensor;                      
sensor_data_t hum_sensor;                      // burada 3 tane sensörü ekledim çünkü  sensörün veri yapýsýný tutan bunlar ve gerçekten varmýþ gibi davranýyor//
sensor_data_t co2_sensor;

int sample_counter = 0;

float read_sensor(sensor_t type) {                     //Sensörün türüne göre rastgele deðer üretiyor ve ayrýca orada yani altýnda switch var ya orada ise hangi veriden sensör istiyorsun demek//
    switch(type) {
        case SENSOR_TEMP:
            return rand() % 40;             // max sýcaklýk deðerini ekledim//     
        case SENSOR_HUM:
            return rand() % 100;              // ayný þekilde buna da//
        case SENSOR_CO2:
            return rand() % 1000;             // ve buna da//
        default:
            return 0;
    }
}

void sort(float arr[], int n) {               // burayý küçükten büyüðe göre ekledim //
    int i, j;    //kaç kere dönmesi gerekiyor//
    for(i = 0; i < n - 1; i++) {           // tur kýsmý burada baþlýyor//
        for(j = 0; j < n - i - 1; j++) {
            if(arr[j] > arr[j + 1]) {        // soldaki saðdan büyük mü bunu belirledim//             
                float t = arr[j];      // bu ve altta iki satýr swap olarak kullandým yqni yer deðiþtirmek için//
                arr[j] = arr[j + 1];         
                arr[j + 1] = t;
            }
        }
    }
}


float median(float arr[], int n) {             // median ortanca deðerini bulsun diye var//
    float temp[SIZE];
    int i;

    for(i = 0; i < n; i++)
        temp[i] = arr[i];

    sort(temp, n);

    if(n % 2 == 0)           // cift mi tek mi soruyor //
    return (temp[n/2 - 1] + temp[n/2]) / 2;        // þimdi burasý neden var sayý eðer yani size tek deðilde cift gelirse burada iþlem görecek//
else
    return temp[n/2];
}

void buffer_add(sensor_data_t *s, float value) {            // burada ise veriyi ekleme yapar//
    s->buffer[s->index] = value;                // veriyi yazýyor burada//
    s->index = (s->index + 1) % SIZE;       // þurasý döngüsel hafýza//
}

float mean(float arr[], int n) {
    float sum = 0;
    int i;                            
    for(i = 0; i < n; i++)
        sum += arr[i];
    return sum / n;
}

float stddev(float arr[], int n) {
    float m = mean(arr, n);
    float sum = 0;                       // burada standart sapma ekledim//
    int i;

    for( i = 0; i < n; i++) {
        float diff = arr[i] - m;
        sum += diff * diff;
    }

    return sqrt(sum / n);
}

float lc2(float arr[], int n) {
    float sum = 0;
    int i;

    for(i = 0; i < n; i++) {
        sum += arr[i] * arr[i];
    }

    return sqrt(sum);
}
void calculate_stats(sensor_data_t *s, float *min, float *max, float *med) {
    int i;
    float sum = 0;

    *min = s->buffer[0];
    *max = s->buffer[0];

    for(i = 0; i < SIZE; i++) {
        float v = s->buffer[i];
        sum += v;

        if(v < *min) *min = v;
        if(v > *max) *max = v;          //buradakiler  aslýnda bizim çýktýlarýmýz oluyor max ve min deðerlerimiz iþte med de//
    }

    *med = median(s->buffer, SIZE);
}
                                    // burada iki iþsaret var birincisi & adrese gönder demek ikincisi ise * de þu yani o adresi kullan demek//
void ble_send() {
    float t_min, t_max, t_med;
    float h_min, h_max, h_med;
    float c_min, c_max, c_med;

    calculate_stats(&temp_sensor, &t_min, &t_max, &t_med);
    calculate_stats(&hum_sensor, &h_min, &h_max, &h_med);          
    calculate_stats(&co2_sensor, &c_min, &c_max, &c_med);

    printf("\n================ BLE kýsmý burada ================\n");

    printf("TEMP -> min: %.1f max: %.1f median: %.1f\n", t_min, t_max, t_med);
    printf("HUM  -> min: %.1f max: %.1f median: %.1f\n", h_min, h_max, h_med);
    printf("CO2  -> min: %.1f max: %.1f median: %.1f\n", c_min, c_max, c_med);

    printf("===========================================\n");
}

int main() {
    srand((unsigned int)time(NULL));

    while(1) {
        float t = read_sensor(SENSOR_TEMP);
        float h = read_sensor(SENSOR_HUM);
        float c = read_sensor(SENSOR_CO2);

        buffer_add(&temp_sensor, t);
        buffer_add(&hum_sensor, h);
        buffer_add(&co2_sensor, c);

        printf("RAW -> T: %.1f H: %.1f CO2: %.1f\n", t, h, c);

        sample_counter++;

        if(sample_counter >= BLE_INTERVAL) {
            ble_send();
            sample_counter = 0;
        }

        Sleep(1000); 
    }

    return 0;
}

