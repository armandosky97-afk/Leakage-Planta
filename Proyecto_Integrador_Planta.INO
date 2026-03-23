#include <Arduino.h>
#include <Wire.h>
#include "DHT.h"
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
#include <qrcode.h>
#include <RTClib.h>  // Include libraries

//Crear Objeto del Sensor DHT11
DHT dht(DHTPIN, DHTTYPE);


// Initialize the stepper library on pins 8 through 11
Stepper myStepper(stepsPerRevolution, 8, 10, 9, 11);



RTC_DS1307 rtc;

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
  
  // Iniciar dispositivos 
  digitalWrite(pinRELAY, HIGH);
  digitalWrite(Bombapin, HIGH); 
  Wire.begin();
  g_init_display();
  //oled.begin();
  coloraqua();
  tone(BUZZER_PIN, 2000, 200);
  delay(500);

  displayMessage("Iniciando.....");
  delay(5000);
  displayMultilineText("Proyecto Integrador Proyecto Leakage de Planta");
  delay(5000);
  mostrarlaptop();

  // Display the QR code for "Marios Id"
  generateQRCode("https://www.youtube.com/watch?v=0kJdWJXxF3Y");
  delay(30000);
  display.invertDisplay(0);


  sleep();
  launch_animation_with_index(WAKEUP);
  launch_animation_with_index(BLINK_LONG);
  launch_animation_with_index(HAPPY);
  launch_animation_with_index(SLEEP);
  launch_animation_with_index(WAKEUP);
  launch_animation_with_index(MOVE_LEFT_BIG);
  launch_animation_with_index(MOVE_RIGHT_BIG);
  launch_animation_with_index(WAKEUP);
  // Note: Font/text drawing would also need to be abstracted if used outside of setup.
  g_update_display();
  // Send the "READY" signal BEFORE the delay to allow the host to sync up.
  
  

  

 
  /*
  
  // Show startup message
  oled.firstPage();
  do {
    oled.setFont(u8g2_font_7x14_tf);
    //display.setFont(u8g2_font_5x7_tr);
    oled.drawStr(SCREEN_WIDTH/6, (SCREEN_HEIGHT/2), "Buen Dia :)...");
    



    } 
  while (oled.nextPage());
  delay(500);
  
  */
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
    mostrartemperatura();
    
    dormirarduino();
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


void girar_motor()
{
  revoluciones = analogRead(ajuste_vel);
  velocidad = map(revoluciones, 0, 1023, 0, 255);
  analogWrite(Motor_PIN, velocidad);



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
  /*
  // Verify sensor readings
  if (isnan(humidity) || isnan(temperature)) {
    // Failed reading, show error message and try again next loop
    oled.firstPage();
    do {
      oled.setFont(u8g2_font_7x14_tf);
      oled.drawStr(5, 30, "Sensor Error!");
    } while (oled.nextPage());
    
    delay(1000);
    return;
  }
  */
  
  
  /*
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

    
       
    /* Quote display (moved up to where LED status was)
    oled.setFont(u8g2_font_6x12_tf);
    getQuote(buffer, currentQuote);
    uint8_t quoteX = (128 - oled.getStrWidth(buffer)) / 2;
    oled.drawStr(quoteX, 64, buffer);
    
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

*/
  

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
  display.clearDisplay();  // Bootup screen
  display.setCursor(0, 0);
  display.println(F("Mini Computer!"));
  display.println(F("Marzo 18, 2026"));
  display.drawBitmap(47, 31, icon, 32, 32, WHITE);  // The laptop icon
  display.display();
  delay(2500);
  

 }

 


void displayMessage(String msg) {
  
  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.getTextBounds(msg, 0, 0, &x1, &y1, &width, &height);
  display.setCursor((SCREEN_WIDTH - width) / 2, (SCREEN_HEIGHT - height) / 2);
  display.println(msg);
  display.display();
}



void displayMultilineText(String text) {
  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);
  display.setCursor((SCREEN_WIDTH - width) / 2, (SCREEN_HEIGHT - height) / 2);

  int maxCharsPerLine = 21;
  int lineHeight = 10;
  int y = 0;

  while (text.length() > 0 && y < SCREEN_HEIGHT) {
    String line = text.substring(0, maxCharsPerLine);
    text = text.substring(min((unsigned int)maxCharsPerLine, text.length()));
    display.setCursor((SCREEN_WIDTH - width) / 2, ((SCREEN_HEIGHT - height) / 2)+y);
    display.println(line);
    y += lineHeight;
  }

  display.display();
}

void mostrartemperatura()
{
      // Reset the hardware watchdog
  wdt_reset();

  // Reset watchdog at start of loop
  kickWatchdog();
  // Read sensor data - add delay before reading to stabilize
  delay(50);
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();



 
  // Convert temperature if needed
  float displayTemp = temperature;
  if (!displayInCelsius) {
    displayTemp = (temperature * 9.0 / 5.0) + 32.0;
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







  display.clearDisplay();  // Refresh display
  display.setCursor(0, 0);

    // Revisas estado de ambos botones
  bool unitChanged = checkButton();
  checkLedButton();
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
    
    display.print(F("Tempertura: "));  // Temperature
    display.print(displayTemp, 1);
    display.println(F("oC"));
    display.drawRect(0, 10, 127, 5, WHITE); // A bar that shows the temprature. Range: 0-40oC
    display.fillRect(0, 10, (displayTemp / 100.0) * 127, 5, WHITE);
    

    display.print(F("\nHumedad Aire: "));  // Humidity
    display.print(humidity, 1);
    display.println(F("%"));
    display.drawRect(0, 25, 127, 5, WHITE); // A bar that shows the humidity. Range: 0-100%
    display.fillRect(0, 25, (humidity / 100.0) * 127, 5, WHITE);

    display.print(F("\nHumedad Suelo: "));  // Humidity
    display.print(percentage, 1);
    display.println(F("%"));
    display.drawRect(0, 40, 127, 5, WHITE); // A bar that shows the humidity. Range: 0-100%
    display.fillRect(0, 40, (percentage / 100.0) * 127, 5, WHITE);

    display.print(F("\nIluminacion: "));  // Humidity
    display.print(mappedValue, 1);
    display.println(F("%"));
    display.drawRect(0, 58, 127, 5, WHITE); // A bar that shows the humidity. Range: 0-100%
    display.fillRect(0, 58, (mappedValue / 100.0) * 127, 5, WHITE);


    display.display();




    if (isnan(humidity) || isnan(temperature)) {
    // Failed reading, show error message and try again next loop
    displayMessage("Error Senor DTH");
    delay(1000);
    return;
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
 

int calculate_safe_radius(int r, int w, int h) {
    // Adhere to U8g2's drawRBox constraints: r must be smaller than half of the width/height.
    // For integer arithmetic, this means r <= (dimension / 2) - 1.
    if (w < 2 * (r + 1)) {
        r = (w / 2) - 1;
    }
    if (h < 2 * (r + 1)) {
        r = (h / 2) - 1;
    }
    return (r < 0) ? 0 : r; // Ensure radius is not negative
}

void draw_eyes()
{
    int r_left = calculate_safe_radius(corner_radius, left_eye.width, left_eye.height);
    int x_left = int(left_eye.x - left_eye.width / 2);
    int y_left = int(left_eye.y - left_eye.height / 2);
    g_draw_filled_round_rect(x_left, y_left, left_eye.width, left_eye.height, r_left, G_COLOR_WHITE);

    int r_right = calculate_safe_radius(corner_radius, right_eye.width, right_eye.height);
    int x_right = int(right_eye.x - right_eye.width / 2);
    int y_right = int(right_eye.y - right_eye.height / 2);
    g_draw_filled_round_rect(x_right, y_right, right_eye.width, right_eye.height, r_right, G_COLOR_WHITE);
}

void draw_frame() {
    g_clear_display();
    draw_eyes();
    g_update_display();
}


void reset_eyes(bool update=true)
{
  //move eyes to the center of the display, defined by SCREEN_WIDTH, SCREEN_HEIGHT
  left_eye.height = REF_EYE_HEIGHT;
  left_eye.width = REF_EYE_WIDTH;
  right_eye.height = REF_EYE_HEIGHT;
  right_eye.width = REF_EYE_WIDTH;
  
  left_eye.x = SCREEN_WIDTH / 2 - REF_EYE_WIDTH / 2 - REF_SPACE_BETWEEN_EYE / 2;
  left_eye.y = SCREEN_HEIGHT / 2;
  right_eye.x = SCREEN_WIDTH / 2 + REF_EYE_WIDTH / 2 + REF_SPACE_BETWEEN_EYE / 2;
  right_eye.y = SCREEN_HEIGHT / 2;

  corner_radius = REF_CORNER_RADIUS;
  
  if (update) {
    draw_frame();
  }
}

void blink(int speed=12)
{
  reset_eyes(false); // Ensure starting from a clean state
  for(int i=0;i<3;i++)
  {
    left_eye.height -= speed;
    right_eye.height -= speed;

     // Dynamically adjust corner_radius during blink
    int current_h = left_eye.height;
    // Map current height (from min_blink_height to REF_EYE_HEIGHT) to a radius (from 1 to REF_CORNER_RADIUS)
    int mapped_radius = map(current_h, 4, REF_EYE_HEIGHT, 1, REF_CORNER_RADIUS); // Assuming min height is 4
    corner_radius = min(mapped_radius, current_h / 2);

    left_eye.width += 3;
    right_eye.width += 3;
    draw_frame();
    delay(1);
  }
  for(int i=0;i<3;i++)
  {
    left_eye.height += speed;
    right_eye.height += speed;

    // Dynamically adjust corner_radius during blink
    int current_h = left_eye.height;
    int mapped_radius = map(current_h, 4, REF_EYE_HEIGHT, 1, REF_CORNER_RADIUS);
    corner_radius = min(mapped_radius, current_h / 2);

    left_eye.width -= 3;
    right_eye.width -= 3;
    draw_frame();
    delay(1);
  }
  reset_eyes();
}



void sleep()
{
  reset_eyes(false);

  left_eye.height = 2;
  left_eye.width = REF_EYE_WIDTH;
  right_eye.height = 2;
  right_eye.width = REF_EYE_WIDTH;

  
  corner_radius = 0;
  draw_frame();
}
void wakeup()
{
  reset_eyes(false);

  for(int h = 2; h <= REF_EYE_HEIGHT; h += 2)
  {
    left_eye.height = h;
    right_eye.height = h;    
    int mapped_radius = map(h, 2, REF_EYE_HEIGHT, 1, REF_CORNER_RADIUS);
    // The corner radius can't be more than half the height, otherwise it distorts.
    corner_radius = min(mapped_radius, h / 2);
    draw_frame();
  }
  

}


void happy_eye()
{
  reset_eyes(true); // Draw the initial eyes to the screen
  int offset = REF_EYE_HEIGHT / 2;
  for(int i=0;i<10;i++)
  {
    // Draw the "happy" lower eyelid shape for both eyes
    g_draw_filled_triangle(left_eye.x - left_eye.width / 2 - 1, left_eye.y + offset, left_eye.x + left_eye.width / 2 + 1, left_eye.y + 5 + offset, left_eye.x - left_eye.width / 2 - 1, left_eye.y + left_eye.height + offset, G_COLOR_BLACK);    
    g_draw_filled_triangle(right_eye.x + right_eye.width / 2 + 1, right_eye.y + offset, right_eye.x - right_eye.width / 2 - 2, right_eye.y + 5 + offset, right_eye.x + right_eye.width / 2 + 1, right_eye.y + right_eye.height + offset, G_COLOR_BLACK);
    offset -= 2;
    g_update_display();
    delay(1);
  }
  delay(1000);
  reset_eyes();
}

void saccade(int direction_x, int direction_y)
{
  //quick movement of the eye, no size change. stay at position after movement, will not move back,  call again with opposite direction
  //direction == -1 :  move left
  //direction == 1 :  move right
  
  const int MOVEMENT_AMPLITUDE_X = 8;
  const int MOVEMENT_AMPLITUDE_Y = 6;
  const int BLINK_AMPLITUDE = 8;

  // Animate the saccade in a single loop for simplicity
  for(int i = 1; i <= 2; i++)
  {
    left_eye.x += MOVEMENT_AMPLITUDE_X * direction_x;
    right_eye.x += MOVEMENT_AMPLITUDE_X * direction_x;
    left_eye.y += MOVEMENT_AMPLITUDE_Y * direction_y;
    right_eye.y += MOVEMENT_AMPLITUDE_Y * direction_y;

    // Eyes narrow during the first half of movement, then widen back
    int height_change = (i == 1) ? -BLINK_AMPLITUDE : BLINK_AMPLITUDE;
    right_eye.height += height_change;
    left_eye.height += height_change;

    draw_frame();
    delay(1);
  }
  
}

void move_right_big_eye()
{
  move_big_eye(1);
}
void move_left_big_eye()
{
  move_big_eye(-1);
}

void move_big_eye(int direction)
{
  //direction == -1 :  move left
  //direction == 1 :  move right
  reset_eyes(false);

  const int OVERSIZE_AMOUNT = 1;
  const int MOVEMENT_AMPLITUDE = 2;
  const int BLINK_AMPLITUDE = 5;

  // --- Move eyes out ---
  for(int i=0;i<3;i++)
  {
    left_eye.x += MOVEMENT_AMPLITUDE * direction;
    right_eye.x += MOVEMENT_AMPLITUDE * direction;    
    right_eye.height -= BLINK_AMPLITUDE;
    left_eye.height -= BLINK_AMPLITUDE;
    
    EyeState* target_eye = (direction > 0) ? &right_eye : &left_eye;
    target_eye->height += OVERSIZE_AMOUNT;
    target_eye->width += OVERSIZE_AMOUNT;

    draw_frame();
    delay(1);
  }
  for(int i=0;i<3;i++)
  {
    left_eye.x += MOVEMENT_AMPLITUDE * direction;
    right_eye.x += MOVEMENT_AMPLITUDE * direction;
    right_eye.height += BLINK_AMPLITUDE;
    left_eye.height += BLINK_AMPLITUDE;

    EyeState* target_eye = (direction > 0) ? &right_eye : &left_eye;
    target_eye->height += OVERSIZE_AMOUNT;
    target_eye->width += OVERSIZE_AMOUNT;

    draw_frame();
    delay(1);
  }

  delay(1000);

  // --- Move eyes back ---
  for(int i=0;i<3;i++)
  {
    left_eye.x -= MOVEMENT_AMPLITUDE * direction;
    right_eye.x -= MOVEMENT_AMPLITUDE * direction;    
    right_eye.height -= BLINK_AMPLITUDE;
    left_eye.height -= BLINK_AMPLITUDE;

    EyeState* target_eye = (direction > 0) ? &right_eye : &left_eye;
    target_eye->height -= OVERSIZE_AMOUNT;
    target_eye->width -= OVERSIZE_AMOUNT;

    draw_frame();
    delay(1);
  }
  for(int i=0;i<3;i++)
  {
    left_eye.x -= MOVEMENT_AMPLITUDE * direction;
    right_eye.x -= MOVEMENT_AMPLITUDE * direction;    
    right_eye.height += BLINK_AMPLITUDE;
    left_eye.height += BLINK_AMPLITUDE;

    EyeState* target_eye = (direction > 0) ? &right_eye : &left_eye;
    target_eye->height -= OVERSIZE_AMOUNT;
    target_eye->width -= OVERSIZE_AMOUNT;

    draw_frame();
    delay(1);
  }

  reset_eyes();
}


void launch_animation_with_index(int animation_index)
{

  switch(animation_index)
      {
        case WAKEUP:
          wakeup();
          break;
        case RESET:
          reset_eyes(true);
          break;
        case MOVE_RIGHT_BIG:
          move_right_big_eye();
          break;
        case MOVE_LEFT_BIG:
          move_left_big_eye();
          break;
        case BLINK_LONG:      
          blink(12);
          delay(1000);
          break;
        case BLINK_SHORT:
          blink(12);
          break; // This was missing, causing fall-through to HAPPY
        case HAPPY:
          happy_eye();      
          break;
        case SLEEP:
          sleep();
          break;
        case SACCADE_RANDOM:
          reset_eyes(true);
          for(int i=0;i<20;i++)
          { 
            int dir_x = random(-1, 2);
            int dir_y = random(-1, 2);
            saccade(dir_x,dir_y);
            delay(1);
            saccade(-dir_x,-dir_y);
            delay(1);     
          }
          // BUG FIX: Moved the 'break' to the end of the case.
          break;
        default:
          // Do nothing for unknown animations
          break;
          
      }
}





void generateQRCode(const char* text) {
  // Create a QR code object
  QRCode qrcode;
  
  // Define the size of the QR code (1-40, higher means bigger size)
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 3, 0, text);

  // Clear the display
  display.clearDisplay();

  // Calculate the scale factor
  int scale = min(SCREEN_WIDTH / qrcode.size, SCREEN_HEIGHT / qrcode.size);
  
  // Calculate horizontal shift
  int shiftX = (SCREEN_WIDTH - qrcode.size*scale)/2;
  
  // Calculate horizontal shift
  int shiftY = (SCREEN_HEIGHT - qrcode.size*scale)/2;

  // Draw the QR code on the display
  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        display.fillRect(shiftX+x * scale, shiftY + y*scale, scale, scale, WHITE);
      }
    }
  }

  // Update the display
  display.display();
  display.invertDisplay(1);
  
}







