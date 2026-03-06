#ifndef _constantes_h
#define _constantes_h


//Sensor LDR
int valor;
int maximo =300;
int minimo =100;
int rawValue = 0;
int mappedValue = 0;

//Sensor de Humedad
int soilMoistureValue = 0;
int percentage=0;

//Potenciometro ajuste de Luz
int leerajuluz=0;
int mapajuluz=0;

//Potenciometro ajuste humedad
int leerajuhum=0;
int mapajuhum=0;



//valores para sensor de Ultrasonido 
long duration; 
float distanceCm; 
float distanceInch;


//Parametros Para el Boton de Encendido
long interval=5000;
unsigned long debounceTime = 50;
unsigned long lastDebounceTime = 0;  // the last time the button was pushed
int value=0;
unsigned long timer=0;
uint8_t sequence=0;
int pattern=0;
unsigned long timePattern=0;
int x=0;




//Pines Analogicos 
#define valorLDR A0
#define sensorHum A1
#define ajuste_luz A2
#define ajuste_hum A3
//Pines de Led RGB Usando Pines PWM
#define RED_PIN 2
#define GREEN_PIN 3
#define BLUE_PIN 4
//Pines Digitales para el Modulo Relay y Botones
#define DHTPIN 36
#define BUTTON_PIN 34
#define botonencender 38
#define LED_BUTTON_PIN 32  // New button pin for toggling LED
#define DHTTYPE DHT11
#define pinRELAY 26
#define BUZZER_PIN 22
#define Bombapin 24
//Pines para sensor de ultrasonido
#define trigPin  28 
#define echoPin  30

//Constantes para calcular la Distancia 
#define SOUND_SPEED 0.034 
#define CM_TO_INCH 0.393701


// Define the number of steps per revolution
const int stepsPerRevolution = 2048;





// Simple watchdog timer
unsigned long lastActivityTime = 0;
const unsigned long WATCHDOG_TIMEOUT = 10000; // 10 seconds

// Global variables (minimized)
bool displayInCelsius = true;
bool lastButtonState = HIGH;
bool lastLedButtonState = HIGH;
bool wasNormal = true;
uint8_t currentQuote = 0;
bool ledEnabled = true;  // New variable to track LED state

// Temperature tracking
float lastTemp = 0;
uint8_t trendSymbol = 0; // 0=stable, 1=up, 2=down

// For trend monitoring over time
const unsigned long TREND_INTERVAL = 300000; // 5 minutes in milliseconds
unsigned long lastTrendTime = 0;
float trendBaseTemp = 0;

// Quote timing
unsigned long lastQuoteChangeTime = 0;


const unsigned long QUOTE_CHANGE_INTERVAL = 5000; // 15 seconds between quote changes
// Quotes array (stored in program memory to save RAM)
const char quote0[] PROGMEM = "5, 4, 3, 2, 1....Va";
const char quote1[] PROGMEM = "No hay futuro.";
const char quote2[] PROGMEM = "Mantenerlo lejos.";
const char quote3[] PROGMEM = "Finalizado.";
const char quote4[] PROGMEM = "Veni Vici Vidi";
const char quote5[] PROGMEM = "Buen Dia :).";
const char quote6[] PROGMEM = "Dia a Dia";

const char* const quotes[] PROGMEM = {
  quote0, quote1, quote2, quote3, quote4, quote5, quote6
};
const uint8_t quoteCount = sizeof(quotes) / sizeof(quotes[0]);
#endif

