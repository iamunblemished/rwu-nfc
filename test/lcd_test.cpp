/*
 * LCD Test Program - Use this to verify LCD wiring
 * Upload this first to test if LCD is working correctly
 */

#include <Arduino.h>
#include <LiquidCrystal.h>

// LCD pins
#define LCD_RS  4
#define LCD_EN  5
#define LCD_D4  6
#define LCD_D5  7
#define LCD_D6  8
#define LCD_D7  9

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

void setup() {
  Serial.begin(9600);
  Serial.println(F("LCD Test"));
  
  // Initialize LCD
  lcd.begin(16, 2);
  delay(50);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LCD Test");
  lcd.setCursor(0, 1);
  lcd.print("Working!");
  
  Serial.println(F("If you see 'LCD Test' and 'Working!' on the LCD, it's OK!"));
  Serial.println(F("If not:"));
  Serial.println(F("1. Check wiring"));
  Serial.println(F("2. Adjust contrast pot (pin V0)"));
  Serial.println(F("3. Verify 5V and GND"));
}

void loop() {
  static uint32_t counter = 0;
  static unsigned long lastUpdate = 0;
  
  if (millis() - lastUpdate > 1000) {
    lastUpdate = millis();
    
    lcd.setCursor(0, 1);
    lcd.print("Count: ");
    lcd.print(counter++);
    lcd.print("    ");
    
    Serial.print(F("Counter: "));
    Serial.println(counter);
  }
}
