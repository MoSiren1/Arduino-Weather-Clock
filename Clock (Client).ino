#include <SPI.h>
#include <Ethernet.h>
#include <LiquidCrystal.h>
#include <EthernetClient.h>

// Ethernet and NTP client setup
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // MAC address. If you have several Make sure you use the proper MAC.

// Initialize LCD
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

EthernetClient client;

// Menu system
enum MenuState { WEATHER, TIME };
MenuState currentMenu = WEATHER;

// Weather and time data placeholders
String currentTemp = "Loading...";
String weatherConditions = "Loading...";
String windSpeed = "Loading...";
String windDirection = "Loading...";
String currentTime = "Loading...";
String currentDate = "Loading...";

// Timing variables
unsigned long lastWeatherUpdate = 0;
const unsigned long weatherUpdateInterval = 60000;  // Fetch weather every 60 seconds

// Function prototypes
void fetchWeatherAndTimeData();
void displayWeather();
void scrollText(const String& text, int row);
void displayTime();
void handleMenu();

void setup() {
  //Serial.begin(9600);  // Initialize serial communication for Debugging
  Ethernet.begin(mac);
  lcd.begin(16, 2);

  lcd.print("Initializing...");
  delay(750);

  //Serial.println("Ethernet initialized."); // Debugging
  
  // Fetch initial weather and time data
  fetchWeatherAndTimeData();
}

void loop() {
  handleMenu();
  delay(200);
}

// Handle the menu switching and updating of data accordingly
void handleMenu() {
  // Simulate a menu button press with a basic state change (replace this with real button logic)
  static unsigned long lastButtonPress = 0;
  if (millis() - lastButtonPress > 4500) {
    currentMenu = (currentMenu == WEATHER) ? TIME : WEATHER;
    lastButtonPress = millis();
  }

  // Display content based on the current menu
  switch (currentMenu) {
    case WEATHER:
      if (millis() - lastWeatherUpdate > weatherUpdateInterval) {
        fetchWeatherAndTimeData();
        lastWeatherUpdate = millis();
      }
      displayWeather();
      break;

    case TIME:
      displayTime();
      break;
  }
}

void fetchWeatherAndTimeData() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Fetching Info...");

  Serial.println("Connecting to server...");

  if (client.connect("192.168.2.55", 5000)) {  // Replace with your Python server IP
    Serial.println("Connected to server.");
    client.println("GET /current_weather HTTP/1.1");
    client.println("Host: 192.168.2.55");  // Replace with your Python server IP
    client.println("Connection: close");
    client.println();

    while (client.connected() && !client.available()) {
      delay(10);
    }
    //Serial.println("Receiving response..."); //Debugging
    String response = "";
    while (client.available()) {
      response += client.readStringUntil('\n');
    }

    Serial.println("Raw response:"); //Debugging
    Serial.println(response);

    // Parse each data field with refined methods
    int tempIndex = response.indexOf("\"temp\":");
    int conditionIndex = response.indexOf("\"condition\":");
    int windSpeedIndex = response.indexOf("\"wind_speed\":");
    int windDirectionIndex = response.indexOf("\"wind_direction\":");
    int timeIndex = response.indexOf("\"time\":");
    int dateIndex = response.indexOf("\"date\":");

    if (tempIndex != -1) {
      currentTemp = response.substring(tempIndex + 7, response.indexOf(",", tempIndex));
    }
    if (conditionIndex != -1) {
      weatherConditions = response.substring(conditionIndex + 13, response.indexOf("\"", conditionIndex + 13));
    }
    if (windSpeedIndex != -1) {
      windSpeed = response.substring(windSpeedIndex + 14, response.indexOf("\"", windSpeedIndex + 14));
    }
    if (windDirectionIndex != -1) {
      windDirection = response.substring(windDirectionIndex + 18, response.indexOf("\"", windDirectionIndex + 18));
    }
    if (timeIndex != -1) {
      currentTime = response.substring(timeIndex + 8, response.indexOf("\"", timeIndex + 8));
    }
    if (dateIndex != -1) {
      currentDate = response.substring(dateIndex + 8, response.indexOf("\"", dateIndex + 8));
    }

   // Serial.println("Parsed data:");                 //Debugging
   // Serial.println("Temperature: " + currentTemp);
   // Serial.println("Condition: " + weatherConditions);
   // Serial.println("Wind Speed: " + windSpeed);
   // Serial.println("Wind Direction: " + windDirection);
   // Serial.println("Time: " + currentTime);
   // Serial.println("Date: " + currentDate);

    client.stop();
    Serial.println("Connection closed."); // Debugging
  } else {
    currentTemp = "Error";
    weatherConditions = "Error";
    windSpeed = "Error";
    windDirection = "Error";
    currentTime = "Error";
    currentDate = "Error";
    Serial.println("Failed to connect to server."); // Debugging
  }
}



// Display weather data on the LCD
void displayWeather() {
  lcd.clear();

  // Prepare the display string
  String displayString = "Temp: " + currentTemp + "F " + weatherConditions;

  // Scroll text if it's too long for the LCD display
  if (displayString.length() > 16) {
    scrollText(displayString, 0);
  } else {
    lcd.setCursor(0, 0);
    lcd.print(displayString);
  }

  // Prepare wind information
  String windInfo = "Wind:" + windDirection + " @ " + windSpeed;
  lcd.setCursor(0, 1);
  lcd.print(windInfo);
}

// Function to scroll text on the LCD
int scrollDelay = 250;  // Delay between scrolls in milliseconds
int endPause = 850;    // Pause duration at end of scroll in milliseconds

// Replace your existing scrollText function with this updated version
void scrollText(const String& text, int row) {
  static int scrollPosition = 0;
  static unsigned long lastScrollTime = 0;

  // If the text has reached the end, pause before resetting
  if (scrollPosition > text.length()) {
    delay(endPause);
    scrollPosition = 0;  // Reset the scroll position
  }

  // Scroll text based on delay interval
  if (millis() - lastScrollTime >= scrollDelay) {
    String visibleText = text.substring(scrollPosition, scrollPosition + 16);  // Adjust for LCD width
    lcd.setCursor(0, row);
    lcd.print(visibleText);

    scrollPosition++;
    lastScrollTime = millis();  // Update last scroll time
  }
}
// Display time on the LCD
void displayTime() {
  lcd.clear();

  // Display the current time on the first row
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  lcd.print(currentTime);

  // Display the current date on the second row
  lcd.setCursor(0, 1);
  lcd.print(currentDate);
}
