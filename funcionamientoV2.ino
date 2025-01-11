#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>
#include "DFRobotDFPlayerMini.h"

// Declaración de los pines
const int RST_pin = 9;   // Pin para resetear el módulo RFID
const int SS_pin = 10;   //Pin de selección del módulo/esclavo. También llamado SDA.
const int greenLED = 2;  // LED verde
const int DFPlayerTX = 4;
const int DFPlayerRX = 5;

// Inicialización del RFID y el MP3
MFRC522 mfrc522(SS_pin, RST_pin);
SoftwareSerial mySerial(DFPlayerRX, DFPlayerTX);
DFRobotDFPlayerMini myDFPlayer;

// Listas paralelas para UIDs y nombres de tarjetas
byte cardUIDs[][4] = {
  //TARJETAS
  { 0x50, 0x7D, 0x21, 0xDF },  // RECETA 1 Sanduche con queso
  { 0x56, 0x0A, 0x1F, 0xDF },  // RECETA 2 Choclo con queso
  { 0x00, 0xFF, 0x77, 0xA7 },  // RECETA 3 Maduro con queso
  { 0x59, 0xFC, 0x21, 0xDF },  // RECETA 4 Bolón de verde con queso
  { 0xB9, 0x17, 0x06, 0x14 },  // RECETA 5 Tostada con aguacate
  { 0xD9, 0x67, 0x21, 0xDF },   // Pan 1
  { 0x1C, 0x54, 0x1F, 0xDF },  // Pan 2
  { 0x0C, 0xCA, 0x21, 0xDF },  // Queso cuadrado
  //LLAVEROS
  { 0x32, 0x8A, 0xAD, 0x3B },  // queso en barra
  { 0xA5, 0x91, 0xAD, 0x3B },  // choclo
  { 0xD3, 0xFF, 0x17, 0x85 },  // maduro
  { 0xA1, 0xDF, 0xD8, 0x1D },  // bolón
  { 0x57, 0x1F, 0x83, 0x63 }  // aguacate
};

String cardNames[] = {
  "Receta1",  //Sanduche con queso
  "Receta2",  //Choclo con queso
  "Receta3",  //Maduro con queso
  "Receta4",  //Bolón de verde con queso
  "Receta5",  //Tostada con aguacate
  "pan1",
  "pan2",
  "quesoCuadrado",
  "quesoBarra",
  "choclo",
  "maduro",
  "bolón",
  "aguacate",
};

//AUDIOS
int AudiosBienvenidas[] = {
  1,  // Empecemos a cocinar con ÑamiÑam
  2,  // Cocinemos con ÑamiÑam
  3   // Ayúdame a cocinar
};

int AudiosPositivos[] = {
  21,  // Wuju! Acertaste
  22,  // Wuju! Eres un gran chef
  23   // Ñam Ñam! Muy bien!
};

int AudiosNegativos[] = {
  24,  // ¿Estás seguro?
  25,  // Oh Oh ¡Intenta con otro ingrediente!
  26   // Oh Oh ¡Inténtalo de nuevo!
};

int Receta1[] = {
  //SANDUCHE DE QUESO
  5,  // mmm...Sanduche de queso ¡Vamos a prepararlo!
  6,  // Primero, necesitamos una rebanada de pan. Acércalo a mi boca, por favor
  7,  // Ahora, necesito un queso
  8,  // Finalmente, necesito una rebana de pan
};
int Receta2[] = {
  //CHOCLO CON QUESO
  9,   // mmm...Choclo con queso ¡Vamos a prepararlo!
  10,  // Primero, necesitamos choclo hervido. Acércalo a mi boca, por favor
  11   // Finalmente, necesito el queso encima
};
int Receta3[] = {
  //MADURO CON QUESO
  12,  // mmm...Maduro con queso ¡Vamos a prepararlo!
  13,  // Primero, necesitamos maduro asado. Acércalo a mi boca, por favor
  14   //Finalmente, necesito el queso encima
};
int Receta4[] = {
  //BOLÓN DE QUESO
  15,  // mmm...Bolón de queso ¡Vamos a prepararlo!
  16,  // Primero, necesitamos el bolón hervido. Acércalo a mi boca, por favor”
  17,  // Ahora, colócale los cubitos de queso encima
};
int Receta5[] = {
  //PAN CON AGUACATE
  18,  // mmm...Pan tostado con aguacate ¡Vamos a prepararlo!
  19,  // Primero, necesitamos una rebanada de pan. Acércalo a mi boca, por favor
  20   // Finalmente, necesitamos una rebanada de aguacate.
};
int* RecetaAudios[] = { Receta1, Receta2, Receta3, Receta4, Receta5 };  //Puntero de las listas de audios
int RecetaAudiosLengths[] = { 4, 3, 3, 3, 3 };                          // Longitudes de los audios de cada receta

// Pasos de las recetas
String Receta1Steps[] = { "pan1|pan2", "quesoCuadrado", "pan1|pan2" };                             //Sanduche de queso
String Receta2Steps[] = { "choclo", "quesoBarra" };                                                //Choclo con queso
String Receta3Steps[] = { "maduro", "quesoBarra" };                                                //Maduro con queso
String Receta4Steps[] = { "bolón", "quesoBarra" };                                                 //Bolón  con queso
String Receta5Steps[] = { "pan1|pan2", "aguacate" };                                               //Tostada con aguacate
String* RecetaSteps[] = { Receta1Steps, Receta2Steps, Receta3Steps, Receta4Steps, Receta5Steps };  //Puntero de las listas de los pasos
int RecetaStepsLengths[] = { 3, 2, 2, 2, 2 };                                                      // Longitudes de los pasos de cada receta

//Variables globales
int currentRecipeIndex = -1;           // Índice de la receta seleccionada
String* currentRecipeSteps = nullptr;  // Puntero a los pasos de la receta seleccionada
int currentStep = 0;
bool recipeSelected = false;  // Indica si se ha seleccionado una receta

void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();

  // Configuración de LED
  pinMode(greenLED, OUTPUT);

  // Inicialización del DFPlayer Mini
  mySerial.begin(9600);
  if (!myDFPlayer.begin(mySerial)) {
    Serial.println(F("DFPlayer Mini no inicializado. Verifica conexiones."));
    while (true)
      ;  // Detiene el código si el DFPlayer falla
  }
  myDFPlayer.volume(30);  // Ajusta el volumen
  myDFPlayer.play(AudiosBienvenidas[random(0, 3)]);  // Audios bienvenida
  delay(5000);
  Serial.println(F("Sistema listo para seleccionar una receta."));

  //AUDIO PARA ESCOGER RECETA
  myDFPlayer.play(4);  // ¿Qué quieres cocinar? Elige una receta y acércala a mi boca por favor.
  Serial.print("Reproducido");
}

void loop() {
  // Verifica si hay una tarjeta cerca del lector
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;  // Sale si no hay tarjeta
  }

  // Obtención del nombre de la tarjeta escaneada
  String scannedCard = getScannedCardName(mfrc522.uid.uidByte);
  if (scannedCard != "") {
    Serial.print("Tarjeta detectada: ");
    Serial.println(scannedCard);

    // Selección de receta si no hay receta seleccionada
    if (!recipeSelected) {
      selectRecipe(scannedCard);
    } else {
      processStep(scannedCard);
    }
  } else {
    Serial.println("Tarjeta no reconocida");
  }
  mfrc522.PICC_HaltA();
}

// Selección de receta
void selectRecipe(String scannedCard) {
  for (int i = 0; i < 5; i++) {  // Sabemos que hay 5 recetas
    if (scannedCard == cardNames[i]) {
      currentRecipeIndex = i;
      currentRecipeSteps = RecetaSteps[i];
      currentStep = 0;
      myDFPlayer.play(RecetaAudios[i][0]);
      delay(5000);
      myDFPlayer.play(RecetaAudios[i][1]);
      recipeSelected = true;
      return;
    }
  }
}

// Proceso de pasos de la receta
void processStep(String scannedCard) {
  if (isIngredientValid(scannedCard, currentRecipeSteps[currentStep])) {
    myDFPlayer.play(AudiosPositivos[random(0, 3)]);  // Audio positivo
    digitalWrite(greenLED, HIGH);                    // Enciende LED verde
    delay(2000);
    digitalWrite(greenLED, LOW);  // Apaga LED verde
    currentStep++;
    //myDFPlayer.play(RecetaAudios[currentRecipeIndex][currentStep+1]);
    if (currentStep == RecetaStepsLengths[currentRecipeIndex]) {
      Serial.println("Receta completada.");
      //AUDIO PARA TERMINAR RECETA Y VOLVER A JUGAR
      myDFPlayer.play(27);
      resetRecipe();
    }
  } else {
    myDFPlayer.play(AudiosNegativos[random(0, 3)]);  // Audio negativo
    Serial.println("Ingrediente incorrecto. Intenta de nuevo.");
  }
}

// Validación de ingredientes (maneja opciones múltiples)
bool isIngredientValid(String scannedCard, String validStep) {
  int separatorIndex = validStep.indexOf("|");
  if (separatorIndex != -1) {
    // Ingrediente con opciones múltiples
    String option1 = validStep.substring(0, separatorIndex);
    String option2 = validStep.substring(separatorIndex + 1);
    return (scannedCard == option1 || scannedCard == option2);
  }
  return (scannedCard == validStep);
}

// Reinicia la selección de receta
void resetRecipe() {
  currentRecipeSteps = nullptr;
  currentStep = 0;
  recipeSelected = false;
}

// Obtiene el nombre de la tarjeta escaneada
String getScannedCardName(byte* uid) {
  for (int i = 0; i < 13; i++) {             // Hay 13 elementos en cardUIDs
    if (memcmp(uid, cardUIDs[i], 4) == 0) {  // Compara el UID escaneado con cada UID conocido
      return cardNames[i];                   // Retorna el nombre asociado
    }
  }
  return "";  // No reconocido
}
