#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>
#include <avr/wdt.h>
#include "pitches.h"
#include "constantes.h"
#include "display_wrapper.h"
#include "audios.h"
#include <Stepper.h>
#include "imagenes.h"
#include <LowPower.h>// https://github.com/rocketscream/Low-Power
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>






// Device instances
//U8G2_SSD1306_128X64_NONAME_1_HW_I2C oled(U8G2_R0);
//Crear Objeto del Sensor DHT11
DHT dht(DHTPIN, DHTTYPE);



// Initialize the stepper library on pins 8 through 11
Stepper myStepper(stepsPerRevolution, 8, 10, 9, 11);




void setup() 
{
  Serial.begin(9600);
  Serial.println(F("Iniciando..."));
  // Set the speed (rpm) of the motor
  myStepper.setSpeed(15);
  // Initialize pins
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_BUTTON_PIN, INPUT_PULLUP);  // Initialize new button pin
  pinMode(pinRELAY, OUTPUT);//Pin de Relay
  pinMode(Bombapin,OUTPUT);//Pin de la Bomba 
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output 
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input 
  pinMode(botonencender,INPUT_PULLUP);//Boton de Encendido
  //goToSleep();
  
  // Initialize devices
  Wire.begin();
  g_init_display();
  //oled.begin();
  coloraqua();
  tone(BUZZER_PIN, 2000, 200);
  delay(500);

  mostrarlaptop();


  

 
 
  
  // Show startup message
  oled.firstPage();
  do {
    oled.setFont(u8g2_font_7x14_tf);
    //display.setFont(u8g2_font_5x7_tr);
    oled.drawStr(SCREEN_WIDTH/6, (SCREEN_HEIGHT/2), "Buen Dia :)...");
    



    } 
  while (oled.nextPage());
  delay(500);
  
  
  // Initialize DHT sensor
  dht.begin();
  delay(1000); // Give DHT time to stabilize
  
  // Initialize watchdog
  lastActivityTime = millis();
  lastQuoteChangeTime = millis(); // Initialize quote timer
  
  // Setup a hardware watchdog
  wdt_enable(WDTO_2S);
  
  Serial.println(F("Setup complete"));
}
void loop()
{
  dormirarduino();
  




  mostrardatosenoled();
 
} 

// Get a quote from program memory
void getQuote(char* buffer, uint8_t index) 
{
  strcpy_P(buffer, (char*)pgm_read_word(&(quotes[index])));
}

// Minimal watchdog reset function
void kickWatchdog() 
{
  lastActivityTime = millis();
}

// Check if system is responsive
void checkWatchdog() 
{
  if (millis() - lastActivityTime > WATCHDOG_TIMEOUT) {
    // System hung - reset core components
    Wire.end();
    Wire.begin();
    //oled.begin();
    g_init_display();
    
    dht.begin();
    kickWatchdog();
  }
} 

// Set RGB LED color
inline void setColor(uint8_t r, uint8_t g, uint8_t b) 
{
  if (ledEnabled)
   {
    analogWrite(RED_PIN, r);
    analogWrite(GREEN_PIN, g);
    analogWrite(BLUE_PIN, b);
   
    
   } 
  else
   {
    // Turn off LED completely when disabled
    analogWrite(RED_PIN, 0);
    analogWrite(GREEN_PIN, 0);
    analogWrite(BLUE_PIN, 0);
    
  }
}

// Check temperature unit button state
bool checkButton() 
{
  bool btnState = digitalRead(BUTTON_PIN);
  bool unitChanged = false;

  


  
  if (btnState == LOW && lastButtonState == HIGH) {
    // Button pressed - debounce
    delay(50);
    
    
    // Toggle temperature unit
    displayInCelsius = !displayInCelsius;
    unitChanged = true;
    Serial.println(F("Toggled temperature unit"));
  }

  
  lastButtonState = btnState;
  return unitChanged;
}

// Check LED toggle button state
  void checkLedButton() 
  {
  bool btnState = digitalRead(LED_BUTTON_PIN);

 
  
  if (btnState == LOW && lastLedButtonState == HIGH) {
    // Button pressed - debounce
    delay(50);
    
    // Toggle LED state
    ledEnabled = !ledEnabled; 
    Serial.print(F("LED toggled: "));
    Serial.println(ledEnabled ? F("ON") : F("OFF"));
    }
   
  lastLedButtonState = btnState;

  

  }

// Update temperature trend indicator over a longer time period
void updateTrend(float currentTemp) 
{
  unsigned long currentTime = millis();
  
  // Initialize base temperature if it's the first reading
  if (trendBaseTemp == 0) 
  {
    trendBaseTemp = currentTemp;
    lastTrendTime = currentTime;
    Serial.println(F("First trend reading - initializing baseline"));
    return;
  }
  
  // Store current value
  lastTemp = currentTemp;
  
  // Only update trend indicator every TREND_INTERVAL (e.g., 5 minutes)
  if (currentTime - lastTrendTime >= TREND_INTERVAL) 
    {
    // Calculate difference from the base temperature from 5 minutes ago
    float diff = currentTemp - trendBaseTemp;
    
    // Threshold of 0.5 degrees over 5 minutes for meaningful change
    if (diff >= 0.5) {
      trendSymbol = 1; // Up arrow
      Serial.print(F("5min Trend: UP (diff: "));
      Serial.print(diff);
      Serial.println(F("?C)"));
    } 
    else if (diff <= -0.5) 
    {
      trendSymbol = 2; // Down arrow
      Serial.print(F("5min Trend: DOWN (diff: "));
      Serial.print(diff);
      Serial.println(F("?C)"));
    } 
    else 
    {
      trendSymbol = 0; // Stable
      Serial.print(F("5min Trend: STABLE (diff: "));
      Serial.print(diff);
      Serial.println(F("?C)"));
    }
    
    // Reset the base temperature and time for next interval
    trendBaseTemp = currentTemp;
    lastTrendTime = currentTime;
  }
}

// Update LED based on temperature - with correction for F scale
void updateLED(float temp, bool isCelsius)
{
  // Convert to Celsius for consistent LED color mapping if in F mode
  float tempForLED = isCelsius ? temp : (temp - 32.0) * 5.0 / 9.0;
  
  if (tempForLED < 8) {
    setColor(0, 100, 255);      // Icy Blue
  } else if (tempForLED < 12) {
    setColor(0, 120, 255);      // Cool Blue
  } else if (tempForLED < 20) {
    setColor(0, 180, 255);      // Light Blue
  } else if (tempForLED < 25) {
    setColor(0, 255, 0);        // Green
  } else if (tempForLED < 33) {
    setColor(255, 100, 0);      // Orange
  } else {
    setColor(255, 0, 0);        // Red
  }
}

// Check temperature and trigger buzzer if needed
void checkTemp(float temp, bool isCelsius)
{
  // Convert to Celsius for consistent evaluation if in F mode
  float tempForCheck = isCelsius ? temp : (temp - 32.0) * 5.0 / 9.0;
  
  bool isAbnormal = (tempForCheck >= 33 || tempForCheck < 5);
  
  if (isAbnormal && wasNormal) {
    // Warning beep
    //digitalWrite(BUZZER_PIN, HIGH);
    //delay(100);
    //digitalWrite(BUZZER_PIN, LOW);
    tone(BUZZER_PIN, 3000, 200);
    wasNormal = false;
  } else if (!isAbnormal && !wasNormal) {
    wasNormal = true;
  }
}

void encenderlapara()
{
  valor = analogRead(valorLDR);
  valor =constrain(valor,minimo,maximo);
  valor =map(valor,maximo,minimo,0,255);
  rawValue = analogRead(valorLDR);
  mappedValue = map(rawValue, 0, 1023, 0, 100);
  

  if (mappedValue<30)
  {
    
    digitalWrite(pinRELAY, LOW);
    
  }
  else
  {
    digitalWrite(pinRELAY, HIGH);   
  
  }

}

  void readSensor()
{
  soilMoistureValue = analogRead(sensorHum);
  Serial.print("Hum Suelo:");
  Serial.print(percentage);
  percentage = map(soilMoistureValue, 0, 1023, 100, 0);

  if(percentage < 10)  
  {
    Serial.println("  Regar on");
    digitalWrite(Bombapin,LOW);
  }
  if(percentage > 40)
  {
    Serial.println("  Regar off");
    digitalWrite(Bombapin,HIGH);
  }

}

  void coloraqua()
    {
      //analogWrite(RED_PIN, 255-0);
      //analogWrite(GREEN_PIN, 255-151);
      //analogWrite(BLUE_PIN, 255-156);
      setColor(0, 151, 156);
    }

  void calculardistancia()
    {
      // Clears the trigPin 
      digitalWrite(trigPin, LOW); 
      delayMicroseconds(2); // Sets the trigPin on HIGH state for 10 micro seconds 
      digitalWrite(trigPin, HIGH); 
      delayMicroseconds(10); 
      digitalWrite(trigPin, LOW); // Reads the echoPin, returns the sound wave travel time in microseconds 
      duration = pulseIn(echoPin, HIGH);
      // Calculate the distance 
      distanceCm = duration * SOUND_SPEED/2;
      // Convert to inches 
      distanceInch = distanceCm * CM_TO_INCH;
    }



    void goToSleep()
    {
      // Allow wake up pin to trigger interrupt on low.
        attachInterrupt(digitalPinToInterrupt(botonencender), wakeUp, LOW);
        
        // Enter power down state with ADC and BOD module disabled.
        // Wake up when wake up pin is low.

        LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
        
        Serial.println("Wake up!");
        
        // Disable external pin interrupt on wake up pin.
        detachInterrupt(digitalPinToInterrupt(botonencender));
        
          
    }

    void wakeUp()
    {



    }
    void rotarmotor() 
    {
    // Step one revolution in one direction:
    myStepper.step(stepsPerRevolution);
    delay(1000);

    // Step one revolution in the other direction:
    myStepper.step(-stepsPerRevolution);
    delay(1000);
    }


    void mostrardatosenoled(){
      // Reset the hardware watchdog
  wdt_reset();

  // Reset watchdog at start of loop
  kickWatchdog();
  
  // Check both buttons
  bool unitChanged = checkButton();
  checkLedButton();
  
  // Read sensor data - add delay before reading to stabilize
  delay(50);
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  
  // Verify sensor readings
  if (isnan(humidity) || isnan(temperature)) {
    // Failed reading, show error message and try again next loop
    oled.firstPage();
    do {g
      oled.setFont(u8g2_font_7x14_tf);
      oled.drawStr(5, 30, "Sensor Error!");
    } while (oled.nextPage());
    
    delay(1000);
    return;
  }
  
  // Update trend indicator
  updateTrend(temperature);
  
  // Print values to serial for debugging
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print("%, Temp: ");
  Serial.print(temperature);
  Serial.print("?C, LED: ");
  Serial.println(ledEnabled ? "ON" : "OFF");
  
  // Convert temperature if needed
  float displayTemp = temperature;
  if (!displayInCelsius) {
    displayTemp = (temperature * 9.0 / 5.0) + 32.0;
  }
  
  // Update outputs - pass original temp in Celsius for proper LED behavior
  updateLED(displayTemp, displayInCelsius);
  checkTemp(displayTemp, displayInCelsius);
  
  // Buffer for storing formatted text
  char buffer[30];
  
  // Format values to fixed decimal places
  char tempStr[6]; // Buffer for temperature string
  char humStr[6];  // Buffer for humidity string
  char humfStr[6]; //Buffer for humidity floor
  char ilumStr[6]; //Buffer de Iluminacion
  char distnStr[6]; //Buffer de Distancia 
  
  // Convert floating point to strings with 1 decimal place
  dtostrf(displayTemp, 4, 1, tempStr);
  dtostrf(humidity, 4, 1, humStr);
  dtostrf(percentage, 4, 1, humfStr);
  dtostrf(mappedValue, 4, 1, ilumStr);
  dtostrf(distanceCm, 4, 1, distnStr);
  
  // Update display
  oled.firstPage();
  do {
    // Temperature display with trend indicator
    oled.setFont(u8g2_font_7x14_tf);
    
    //oled.setFont(u8g2_font_5x7_tr);
    // Format the temperature with trend
    if (trendSymbol == 1) {
      // Up arrow
      snprintf(buffer, sizeof(buffer), "Temp: %s %c %c", tempStr, displayInCelsius ? 'C' : 'F', 24); // ASCII 24 is ?
    } else if (trendSymbol == 2) {
      // Down arrow
      snprintf(buffer, sizeof(buffer), "Temp: %s %c %c", tempStr, displayInCelsius ? 'C' : 'F', 25); // ASCII 25 is ?
    } else {
      // No trend indicator
      snprintf(buffer, sizeof(buffer), "Temp: %s %c", tempStr, displayInCelsius ? 'C' : 'F');
    }
    
    uint8_t tempX = (SCREEN_WIDTH - oled.getStrWidth(buffer)) / 2;
    oled.drawStr(tempX, 15, buffer);
    
    // Humidity display
    snprintf(buffer, sizeof(buffer), "HumAire: %s %%", humStr);
    uint8_t humX = (SCREEN_WIDTH - oled.getStrWidth(buffer)) / 2;
    oled.drawStr(humX, 26, buffer);
 
    //Humedad del Suelo
      snprintf(buffer, sizeof(buffer), "HumSuelo: %s %%", humfStr);
    uint8_t humfx = (SCREEN_WIDTH - oled.getStrWidth(buffer)) / 2;
    oled.drawStr(humfx, 40, buffer);


     //Iluminacion del Ambiente
      snprintf(buffer, sizeof(buffer), "Luz: %s %%", ilumStr);
    uint8_t ilumx = (SCREEN_WIDTH - oled.getStrWidth(buffer)) / 2;
    oled.drawStr(ilumx, 52, buffer);


       
    /*// Quote display (moved up to where LED status was)
    oled.setFont(u8g2_font_6x12_tf);
    getQuote(buffer, currentQuote);
    uint8_t quoteX = (128 - oled.getStrWidth(buffer)) / 2;
    oled.drawStr(quoteX, 64, buffer);
    */
         //Mostrar Distancia
      snprintf(buffer, sizeof(buffer), "Umbral: %s cm", distnStr);
    uint8_t distnx = (SCREEN_WIDTH - oled.getStrWidth(buffer)) / 2;
    oled.drawStr(distnx, 64, buffer);




  } while (oled.nextPage());
  
  // Check if it's time to change the quote (every QUOTE_CHANGE_INTERVAL milliseconds)
  unsigned long currentTime = millis();
  if (currentTime - lastQuoteChangeTime >= QUOTE_CHANGE_INTERVAL) {
    // Update quote
    currentQuote = (currentQuote + 1) % quoteCount;
    lastQuoteChangeTime = currentTime;
    
    // Log quote change
    Serial.print(F("Quote changed to #"));
    Serial.println(currentQuote);
  }


  // Check if system is hanging
  checkWatchdog();

    //Calcular Distancia
  calculardistancia();
  //Leer sensor de Humedad para el Riego
  readSensor();
  //Leer fotoresistencia para encender foco
  encenderlapara();


  // Wait before next update

  
  delay(1000);

    }
 void dormirarduino()
 {
  //Funcion para mandar al arduino a dormir
   if(!digitalRead(botonencender))
      { 
	    lastDebounceTime = millis();
      
      while(!digitalRead(botonencender))
      {
        

      }

	   if ((millis() - lastDebounceTime) >interval) 
      {
		    delay(500);
		    goToSleep();
      }
   }
 }

 void mostrarlaptop()
 {
  

 }

