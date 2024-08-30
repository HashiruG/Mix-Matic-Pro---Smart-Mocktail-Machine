#if 1

#include <Adafruit_GFX.h>
#include <Riscduino_MCUFRIEND_kbv.h>
Riscduino_MCUFRIEND_kbv tft;
#include <TouchScreen.h>
#include <ArduinoJson.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeMono12pt7b.h>

#define MINPRESSURE 10
#define MAXPRESSURE 1000


#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define BLACK      0x0000
#define DARK_NAVY  0x000080
#define DARK_GREY  0x2F4F4F
#define DARK_OLIVE 0x556B2F
#define DARK_RED   0x8B0000
#define LIGHT_GREY 0xD3D3D3

#define BUTTONBORDERCLR     WHITE
#define BUTTONBACKGROUNDCLR DARK_NAVY
#define BUTTONFONTCLR       WHITE
#define HEADINGFONTCLR      WHITE
#define BACKGROUNDCLR       DARK_RED

// Define pump pins

#define SODAPUMP 45
#define SEVENUPPUMP 47
#define SPRITEPUMP 49
#define SUGARPUMP 51
#define FLAVOURPUMP 53


// Define ultrasonic sensor pins
#define TRIGSUGAR 23
#define SUGARLEVEL 25
#define TRIGSODA 27
#define SODALEVEL 29
#define TRIG7UP 31
#define SEVENUPLEVEL 33
#define TRIGSPRITE 35
#define SPRITELEVEL 37

// Define IR sensor and buzzer pins
#define IRSENSOR 39 
#define BUZZER 41  
 
// Define Push button pins
#define Pushbtn1 32
#define Pushbtn2 38

const int XP = 9, XM = A3, YP = A2, YM = 8; // 320x480 ID=0x9488
const int TS_LEFT = 124, TS_RT = 908, TS_TOP = 79, TS_BOT = 928;


const int maxRecipes = 3;  
String recipeNames[maxRecipes];
float recipePrices[maxRecipes];


TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

struct INTERFACE {
    const char* label;
    int x, y, w, h;
    Adafruit_GFX_Button button;
};

INTERFACE INTERFACE1[] = {
        {nullptr, 62, 80, 200, 100},
        {nullptr, 62, 190, 200, 100},
        {nullptr, 62, 300, 200, 100}
};

const int numButtons = sizeof(INTERFACE1) / sizeof(INTERFACE1[0]);

INTERFACE INTERFACE2[] = {
        {"Lite", 62, 80, 200, 90},
        {"Regular", 62, 190, 200, 90},
        {"Sweet", 62, 300, 200, 90}
};

const int numNewButtons = sizeof(INTERFACE2) / sizeof(INTERFACE2[0]);

INTERFACE INTERFACE3[] = {
        {"Soda", 62, 80, 200, 90},
        {"Sprite", 62, 190, 200, 90},
        {"7 UP", 62, 300, 200, 90}
};

const int numThirdButtons = sizeof(INTERFACE3) / sizeof(INTERFACE3[0]);

Adafruit_GFX_Button backButton;
Adafruit_GFX_Button cancelButton;
Adafruit_GFX_Button mixButton;

int pixel_x, pixel_y; // Touch_getXY() updates global vars

bool Touch_getXY(void) {
    TSPoint p = ts.getPoint();
    pinMode(YP, OUTPUT); 
    pinMode(XM, OUTPUT); 
    bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
    if (pressed) {
        pixel_x = map(p.x, TS_LEFT, TS_RT, 0, tft.width());
        pixel_y = map(p.y, TS_TOP, TS_BOT, 0, tft.height());
    }
    return pressed;
}


enum DisplayState {
    HOME,
    STATE_INTERFACE1,
    STATE_INTERFACE2,
    STATE_INTERFACE3,
    PROCEED,
    MIXING
};

DisplayState displayState = HOME;
int selectedFirstButton = -1;
int selectedSecondButton = -1;
int selectedThirdButton = -1;
float price;


int ultrasonic(int trig, int echo) ;
void buzz(int f, int t);
bool isCupAvailable();
bool isIngredientLevelSufficient();
void makeMocktail(int sugarLevel, int soda) ;
void clean();


void clean(){


    tft.fillScreen(BACKGROUNDCLR);
    tft.setFont(&FreeSans18pt7b);
    tft.setCursor(90, 150);
    tft.setTextColor(HEADINGFONTCLR);
    tft.setTextSize(1);
    tft.print("Cleaning...");
    tft.setFont();
    delay(500);


    digitalWrite(FLAVOURPUMP,HIGH);
    delay(5000);
    digitalWrite(FLAVOURPUMP,LOW);
    setup();

}

void getToInitialPoint(){
    digitalWrite(SUGARPUMP,LOW);
    digitalWrite(SODAPUMP,LOW);
    digitalWrite(SEVENUPPUMP,LOW);
    digitalWrite(SPRITEPUMP,LOW);
    delay(1500);
    digitalWrite(SUGARPUMP,HIGH);
    digitalWrite(SODAPUMP,HIGH);
    digitalWrite(SEVENUPPUMP,HIGH);
    digitalWrite(SPRITEPUMP,HIGH);
    
};


void setup(void) {



    // Initialize pumps
    pinMode(SUGARPUMP, OUTPUT);
    pinMode(SODAPUMP, OUTPUT);
    pinMode(SEVENUPPUMP, OUTPUT);
    pinMode(SPRITEPUMP, OUTPUT);
    pinMode(FLAVOURPUMP,OUTPUT);

    // Initialize ultrasonic sensors
    pinMode(TRIGSUGAR, OUTPUT);
    pinMode(SUGARLEVEL, INPUT);
    pinMode(TRIGSODA, OUTPUT);
    pinMode(SODALEVEL, INPUT);
    pinMode(TRIG7UP, OUTPUT);
    pinMode(SEVENUPLEVEL, INPUT);
    pinMode(TRIGSPRITE, OUTPUT);
    pinMode(SPRITELEVEL, INPUT);

    // Initialize IR sensor and buzzer
    pinMode(IRSENSOR, INPUT); 
    pinMode(BUZZER, OUTPUT);  


    pinMode(Pushbtn1,INPUT_PULLUP);
    pinMode(Pushbtn2,INPUT_PULLUP);

    digitalWrite(SUGARPUMP, HIGH);
    digitalWrite(SODAPUMP, HIGH);
    digitalWrite(SEVENUPPUMP, HIGH);
    digitalWrite(SPRITEPUMP, HIGH);
    digitalWrite(BUZZER,HIGH);



//Toch display configurations

#if defined(arm) || defined(ESP32)
    analogReadResolution(10);
#endif
    Serial.begin(9600);
    uint16_t ID = tft.readID();
    Serial.print("TFT ID = 0x");
    Serial.println(ID, HEX);
    Serial.println("Calibrate for your Touch Panel");
    if (ID == 0xD3D3) ID = 0x9486;
    tft.begin(ID);
    tft.setRotation(0);
    tft.fillScreen(BACKGROUNDCLR);

    tft.setFont(&FreeSans18pt7b);
    tft.setCursor(90, 150);
    tft.setTextColor(HEADINGFONTCLR);
    tft.setTextSize(1);
    tft.print("Welcome");
    tft.setCursor(150, 200);
    tft.print("to");
    tft.setCursor(65, 250);
    tft.print("MixMaticPro!");
    tft.setFont();

    tft.setFont(&FreeMono12pt7b);
    tft.setCursor(70, 420);
    tft.print("Tap me to mix");
    tft.setCursor(88, 450); //320,500
    tft.print("a mocktail");
    tft.setFont();
    displayState = HOME;



}

void loop(void) {


  int buttonState1=0,buttonState2=0;
    buttonState1 = digitalRead(Pushbtn1);
    buttonState2 = digitalRead(Pushbtn2);
    if (buttonState1 == LOW) {
        Serial.println("Button 1 Pressed - Cleaning flavour");
        clean();
    }
    else if(buttonState2 == LOW){
        Serial.println("Button 2 Pressed - Getting to initial point");
        getToInitialPoint();
    }


// Seriel Communication with ESP8266 to retrieve data recieved to ESP8266 via the web app
  if (Serial.available()) {
        String jsonString = Serial.readStringUntil('\n');
        Serial.println("Received: " + jsonString);


        DynamicJsonDocument doc(1024);

        DeserializationError error = deserializeJson(doc, jsonString);

        if (error) {
            Serial.println("Failed to parse JSON");
            return;
        
        bool success = doc["success"];

        if (success) {
            JsonArray dataArray = doc["data"];

            // Reset the recipe data before updating
            for (int i = 0; i < maxRecipes; i++) {
                recipeNames[i] = "";
                recipePrices[i] = 0;
            }

            // Updating global arrays to match recieved data from ESP8266
            int index = 0;
            for (JsonObject item : dataArray) {
                if (index < maxRecipes) {
                    recipeNames[index] = item["recipeName"].as<String>();
                    recipePrices[index] = item["price"];
                    index++;
                }
            }

           
            updateInitialButtonLabels();
            
            setup();

        } else {
            Serial.println("Request was not successful");
        }
    }


//getting the touch coordinates
    bool down = Touch_getXY();

    if (displayState == HOME) {
        if (down) {
            tft.fillScreen(BACKGROUNDCLR);
            displayState = STATE_INTERFACE1;

            tft.setFont(&FreeSans18pt7b);
            tft.setCursor(20, 40);
            tft.setTextColor(HEADINGFONTCLR);
            tft.setTextSize(1);
            tft.print("Palette of Flavours");
            tft.setFont(&FreeSans9pt7b);
            tft.setFont();

            for (int i = 0; i < numButtons; i++) {
                INTERFACE1[i].label = recipeNames[i].c_str(); // Convert String to const char*
                INTERFACE1[i].button.initButtonUL(&tft, INTERFACE1[i].x, INTERFACE1[i].y, INTERFACE1[i].w, INTERFACE1[i].h, BUTTONBORDERCLR, BUTTONBACKGROUNDCLR, BUTTONFONTCLR, INTERFACE1[i].label, 2, 2);
                INTERFACE1[i].button.drawButton(false);
            }




            backButton.initButtonUL(&tft, 10, 420, 100, 50, BUTTONBORDERCLR, BUTTONBACKGROUNDCLR, BUTTONFONTCLR, "Back", 2, 2);
            backButton.drawButton(false);
        }
    } else if (displayState == STATE_INTERFACE1) {
        for (int i = 0; i < numButtons; i++) {
            INTERFACE1[i].button.press(down && INTERFACE1[i].button.contains(pixel_x, pixel_y));

            if (INTERFACE1[i].button.justPressed()) {
                handleInitialButtonPress(i);
            }
        }
        backButton.press(down && backButton.contains(pixel_x, pixel_y));
        if (backButton.justPressed()) {
            displayState = HOME;
            tft.fillScreen(BACKGROUNDCLR);
            setup();
        }
    } else if (displayState == STATE_INTERFACE2) {
        for (int i = 0; i < numNewButtons; i++) {
            INTERFACE2[i].button.press(down && INTERFACE2[i].button.contains(pixel_x, pixel_y));

            if (INTERFACE2[i].button.justPressed()) {
                handleSecondButtonPress(i);
            }
        }
        backButton.press(down && backButton.contains(pixel_x, pixel_y));
        if (backButton.justPressed()) {
            displayState = HOME;
            tft.fillScreen(BACKGROUNDCLR);
            setup();
        }
    } else if (displayState == STATE_INTERFACE3) {
        for (int i = 0; i < numThirdButtons; i++) {
            INTERFACE3[i].button.press(down && INTERFACE3[i].button.contains(pixel_x, pixel_y));

            if (INTERFACE3[i].button.justPressed()) {
                handleThirdButtonPress(i);
            }
        }
        backButton.press(down && backButton.contains(pixel_x, pixel_y));
        if (backButton.justPressed()) {
            displayState = HOME;
            tft.fillScreen(BACKGROUNDCLR);
            setup();
        }
    } else if (displayState == PROCEED) {
        mixButton.press(down && mixButton.contains(pixel_x, pixel_y));
        if (mixButton.justPressed()) {
            displayState = MIXING;
            tft.fillScreen(BACKGROUNDCLR);
            tft.setFont(&FreeSans18pt7b);
            tft.setCursor(40, 200);
            tft.setTextColor(HEADINGFONTCLR);
            tft.setTextSize(1);
            tft.print("Your beverage \n      is on the way...");
            callMixFunction();

            displayState = HOME;
            tft.fillScreen(BACKGROUNDCLR);
            tft.setFont();
            setup();
        }
        cancelButton.press(down && cancelButton.contains(pixel_x, pixel_y));
        if (cancelButton.justPressed()) {
            displayState = HOME;
            tft.fillScreen(BACKGROUNDCLR);
            setup();
        }
    }
}

void handleInitialButtonPress(int buttonIndex) {
    tft.fillScreen(BACKGROUNDCLR);
    selectedFirstButton = buttonIndex;
    tft.setFont(&FreeSans18pt7b);
    tft.setCursor(25, 40);
    tft.setTextColor(HEADINGFONTCLR);
    tft.setTextSize(1);
    tft.print("Sweetness Profile");
    tft.setFont();
    for (int j = 0; j < numNewButtons; j++) {
        INTERFACE2[j].button.initButtonUL(&tft, INTERFACE2[j].x, INTERFACE2[j].y, INTERFACE2[j].w, INTERFACE2[j].h, BUTTONBORDERCLR, BUTTONBACKGROUNDCLR, BUTTONFONTCLR, INTERFACE2[j].label, 2, 2);
        INTERFACE2[j].button.drawButton(false);
    }
    displayState = STATE_INTERFACE2;

    backButton.initButtonUL(&tft, 10, 420, 100, 50, BUTTONBORDERCLR, BUTTONBACKGROUNDCLR, BUTTONFONTCLR, "Back", 2, 2);
    backButton.drawButton(false);
}

void handleSecondButtonPress(int buttonIndex) {
    tft.fillScreen(BACKGROUNDCLR);
    selectedSecondButton = buttonIndex;
    tft.setFont(&FreeSans18pt7b);
    tft.setCursor(45, 40);
    tft.setTextColor(HEADINGFONTCLR);
    tft.setTextSize(1);
    tft.print("Soda Selection");
    tft.setFont();
    for (int j = 0; j < numThirdButtons; j++) {

        INTERFACE3[j].button.initButtonUL(&tft, INTERFACE3[j].x, INTERFACE3[j].y, INTERFACE3[j].w, INTERFACE3[j].h, BUTTONBORDERCLR, BUTTONBACKGROUNDCLR, BUTTONFONTCLR, INTERFACE3[j].label, 2,2);
        INTERFACE3[j].button.drawButton(false);
    }
    displayState = STATE_INTERFACE3;

    backButton.initButtonUL(&tft, 10, 420, 100, 50, BUTTONBORDERCLR, BUTTONBACKGROUNDCLR, BUTTONFONTCLR, "Back", 2, 2);
    backButton.drawButton(false);
}

void handleThirdButtonPress(int buttonIndex) {
    selectedThirdButton = buttonIndex;
    displaySelections();
    displayState = PROCEED;

    // Initialize and draw the "Cancel" button
    cancelButton.initButtonUL(&tft, 10, 400, 100, 50, BUTTONBORDERCLR, BUTTONBACKGROUNDCLR, BUTTONFONTCLR, "Cancel", 2, 2);
    cancelButton.drawButton(false);

    // Initialize and draw the "Mix" button
    mixButton.initButtonUL(&tft, 210, 400, 100, 50, BUTTONBORDERCLR, BUTTONBACKGROUNDCLR, BUTTONFONTCLR, "Mix", 2, 2);
    mixButton.drawButton(false);
}

void displaySelections() {
    tft.fillScreen(BACKGROUNDCLR);
    tft.setFont(&FreeSans18pt7b);
    tft.setCursor(20, 50);
    tft.setTextColor(HEADINGFONTCLR);
    tft.setTextSize(1);
    tft.print("Cart");

    tft.setFont(&FreeSerif12pt7b);
    tft.setCursor(20, 100);
    tft.print("Drink: ");
    tft.print(INTERFACE1[selectedFirstButton].label);

    tft.setCursor(20, 150);
    tft.print("Sugar Level: ");
    tft.print(INTERFACE2[selectedSecondButton].label);

    tft.setCursor(20, 200);
    tft.print("Soda Type: ");
    tft.print(INTERFACE3[selectedThirdButton].label);



    for(int i=0;i<maxRecipes;i++){
        if(selectedFirstButton==i){
            price=recipePrices[i];
        }
    }

    tft.setCursor(20, 250);
    tft.print("Price: Rs ");
    tft.print(price);
    tft.setFont();
}

void callMixFunction() {

    if (selectedSecondButton == 0 && selectedThirdButton == 0) {
        makeMocktail(1, 1);
    } else if (selectedSecondButton == 0 && selectedThirdButton == 1) {
        makeMocktail(1, 2);
    } else if (selectedSecondButton == 0 && selectedThirdButton == 2) {
        makeMocktail(1, 3);
    } else if (selectedSecondButton == 1 && selectedThirdButton == 0) {
        makeMocktail(2, 1);
    } else if (selectedSecondButton == 1 && selectedThirdButton == 1) {
        makeMocktail(2, 2);
    } else if (selectedSecondButton == 1 && selectedThirdButton == 2) {
        makeMocktail(2, 3);
    } else if (selectedSecondButton == 2 && selectedThirdButton == 0) {
        makeMocktail(3, 1);
    } else if (selectedSecondButton == 2 && selectedThirdButton == 1) {
        makeMocktail(3, 2);
    } else if (selectedSecondButton == 2 && selectedThirdButton == 2) {
        makeMocktail(3, 3);
    }
}






#endif


int ultrasonic(int trig, int echo) {
    digitalWrite(trig, LOW);
    delayMicroseconds(2);
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);
    long t = pulseIn(echo, HIGH);
    
    long cm = t / 29 / 2;
    return cm;
}

void buzz(int f, int t) {
    tone(BUZZER, f, t);
}

bool isCupAvailable() {
   
    return digitalRead(IRSENSOR) == LOW; 
}

bool isIngredientLevelSufficient( int soda) {


    if(ultrasonic(TRIGSUGAR, SUGARLEVEL) < 20){
        if(soda==1)
            return ultrasonic(TRIGSODA, SODALEVEL) < 20;
        else if(soda==2)
            return ultrasonic(TRIGSPRITE, SPRITELEVEL) < 20;
        else
            return ultrasonic(TRIG7UP, SEVENUPLEVEL) < 20;
    }
    else{
        Serial.println("Sugar level is not sufficient.");
        return 0;
    }


}



void ingredientLevelLowPrint(int ingredient){
    tft.fillScreen(BACKGROUNDCLR);
    tft.setFont(&FreeSans18pt7b);
    tft.setCursor(10, 50);
    tft.setTextColor(HEADINGFONTCLR);
    tft.setTextSize(1);

    if(ingredient==1){
        tft.print("Soda is Low");
    }
    else if(ingredient==2){
        tft.print("Sprite is Low");
    }
    else if(ingredient==3){
        tft.print("7UP is Low");
    }
    else{
        tft.print("Sugar Syrup is Low");
    }
    delay(3000);
}

void cupIsNotAvailablePrint(){
    tft.fillScreen(BACKGROUNDCLR);
    tft.setFont(&FreeSans18pt7b);
    tft.setCursor(10, 250);
    tft.setTextColor(HEADINGFONTCLR);
    tft.setTextSize(1);
    tft.print("Cup is not available!");
    delay(2000);
}



void makeMocktail(int sugarLevel, int soda) {
  
    if(isIngredientLevelSufficient(soda)){
        if(isCupAvailable()){

            int sugarDelay;
            int sodaPump;

            if(soda==1 && sugarLevel==1){
                sugarDelay = 2000;
                sodaPump=SODAPUMP;
                Serial.println("Sugar Level: Light");
                Serial.print("Soda Type: Soda");

            }
            else if(soda==1 && sugarLevel==2){
                sugarDelay = 3000;
                sodaPump=SODAPUMP;
                Serial.println("Sugar Level: Regular");
                Serial.print("Soda Type: Soda");
            }

            else if(soda==1 && sugarLevel==3){
                sugarDelay = 4000;
                sodaPump=SODAPUMP;
                Serial.println("Sugar Level: Sweet");
                Serial.print("Soda Type: Soda");
            }

            else if(soda==2 && sugarLevel==1){
                sugarDelay = 0000;
                sodaPump=SEVENUPPUMP;
                Serial.println("Sugar Level: Lite");
                Serial.print("Soda Type: 7UP");
            }

            else if(soda==2 && sugarLevel==2){
                sugarDelay = 1000;
                sodaPump=SEVENUPPUMP;
                Serial.println("Sugar Level: Regular");
                Serial.print("Soda Type: 7UP");
            }

            else if(soda==2 && sugarLevel==3){
                sugarDelay = 1500;
                sodaPump=SEVENUPPUMP;
                Serial.println("Sugar Level: Sweet");
                Serial.print("Soda Type: 7UP");
            }

            else if(soda==3 && sugarLevel==1){
                sugarDelay = 0000;
                sodaPump=SPRITEPUMP;
                Serial.println("Sugar Level: Lite");
                Serial.print("Soda Type: Sprite");
            }

            else if(soda==3 && sugarLevel==2){
                sugarDelay = 200;
                sodaPump=SPRITEPUMP;
                Serial.println("Sugar Level: Regular");
                Serial.print("Soda Type: Sprite");
            }

            else if(soda==3 && sugarLevel==3){
                sugarDelay = 500;
                sodaPump=SPRITEPUMP;
                Serial.println("Sugar Level: Sweet");
                Serial.print("Soda Type: Sprite");
            }
            else {
                Serial.println("Error in soda and sugar level selection :line 657");
            }


           
            digitalWrite(FLAVOURPUMP,HIGH);
            digitalWrite(SUGARPUMP, LOW);
            digitalWrite(sodaPump, LOW);
            delay(sugarDelay);
            digitalWrite(SUGARPUMP, HIGH);
            delay(4500 - sugarDelay);
            digitalWrite(sodaPump, HIGH);
            digitalWrite(FLAVOURPUMP,LOW);

            delay(200);
            buzz(1500,500);

        }
        else{
            Serial.print("Place the cup\n");
            buzz(1500, 1000);
            cupIsNotAvailablePrint();
        }
    }
    else{
        Serial.print("Ingredient level is low!\n");
        buzz(1000, 1000);
        ingredientLevelLowPrint(soda);
    }
}

void updateInitialButtonLabels() {
  
    for (int i = 0; i < numButtons; i++) {
        if (i < maxRecipes) {
            tft.setFont();
            INTERFACE1[i].label = const_cast<char*>(recipeNames[i].c_str());
        } else {
            INTERFACE1[i].label = nullptr; 
        }
    }
}






