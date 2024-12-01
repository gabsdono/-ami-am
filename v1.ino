#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>
#include "DFRobotDFPlayerMini.h"

// Declaración de los pines
const int RST_pin = 9;  // Pin para resetear el módulo RFID
const int redLED = 3;   // LED rojo
const int greenLED = 2; // LED verde
const int DFPlayerTX = 4;
const int DFPlayerRX = 5;

// Inicialización del RFID y el MP3
MFRC522 mfrc522(10, RST_pin);  // SDA en el pin 10
SoftwareSerial mySerial(DFPlayerRX, DFPlayerTX);
DFRobotDFPlayerMini myDFPlayer;

// Listas paralelas para UIDs y nombres de tarjetas
byte cardUIDs[][4] = {
  {0x50, 0x2B, 0xD0, 0xA6}, // sanduche
  {0x83, 0xB6, 0x63, 0x06}, // ensalada
  {0xBC, 0x82, 0x1C, 0x4A}, // pan
  {0x6C, 0xAA, 0x27, 0x49}, // pan
  {0x9C, 0x5F, 0xF0, 0x37}  // queso
};

String cardNames[] = {
  "sanduche",
  "ensalada",
  "pan",
  "pan",
  "queso"
};

int numCards = sizeof(cardUIDs) / sizeof(cardUIDs[0]);

// Pasos de las recetas
String sanducheSteps[] = {"pan", "queso", "pan"};
String ensaladaSteps[] = {"ensalada", "pan", "queso"};
String* currentRecipeSteps = nullptr;  // Puntero a los pasos de la receta seleccionada
int currentStep = 0;
bool recipeSelected = false;  // Indica si se ha seleccionado una receta

void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();
  
  // Configuración de LEDs
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  
  // Inicialización del DFPlayer Mini
  mySerial.begin(9600);
  if (!myDFPlayer.begin(mySerial)) {
    Serial.println(F("DFPlayer Mini no inicializado. Verifica conexiones."));
    while (true);  // Detiene el código si el DFPlayer falla
  }
  myDFPlayer.volume(30);  // Ajusta el volumen
  
  Serial.println(F("Sistema listo para seleccionar una receta."));
}

void loop() {
  // Verifica si hay una tarjeta cerca del lector
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;  // Sale si no hay tarjeta
  }

  int cardIndex = getCardIndex(mfrc522.uid.uidByte);
  if (cardIndex != -1) {
    String scannedCard = cardNames[cardIndex];
    Serial.print("Tarjeta detectada: ");
    Serial.println(scannedCard);

    // Selección de receta si no hay receta seleccionada
    if (!recipeSelected) {
      if (scannedCard == "sanduche") {
        currentRecipeSteps = sanducheSteps;
        myDFPlayer.play(1);  // Audio para seleccionar sánduche
        Serial.println("Receta seleccionada: sanduche");
        recipeSelected = true;
      } else if (scannedCard == "ensalada") {
        currentRecipeSteps = ensaladaSteps;
        myDFPlayer.play(5);  // Audio para seleccionar ensalada
        Serial.println("Receta seleccionada: ensalada");
        recipeSelected = true;
      }
      delay(2000);  // Pausa para el audio de selección de receta
    } 
    // Proceso de ingredientes para la receta seleccionada
    else {
      // Verifica si el ingrediente es correcto para el paso actual
      if (scannedCard == currentRecipeSteps[currentStep]) {
        myDFPlayer.play(4);  // Audio de confirmación
        digitalWrite(greenLED, HIGH);  // Enciende el LED verde para confirmar
        Serial.println("Ingrediente correcto.");
        delay(2000);  // Espera para que el audio se reproduzca
        digitalWrite(greenLED, LOW);   // Apaga el LED verde
        currentStep++;  // Avanza al siguiente paso

        // Verifica si se han completado todos los pasos
        if (currentStep == sizeof(sanducheSteps) / sizeof(sanducheSteps[0])) {
          Serial.println("Receta completada.");
          myDFPlayer.play(2);  // Reproduce audio de receta completada (audio 13)
          delay(2000);  // Pausa para el audio final
          currentStep = 0;       // Reinicia el paso
          recipeSelected = false;  // Reinicia la selección de receta
          currentRecipeSteps = nullptr;  // Limpia la receta actual
        }
      } else {
        myDFPlayer.play(3);  // Audio de error (audio 14)
        digitalWrite(redLED, HIGH);  // Enciende el LED rojo para indicar error
        Serial.println("Ingrediente incorrecto. Intenta de nuevo.");
        delay(2000);  // Espera para que el audio se reproduzca
        digitalWrite(redLED, LOW);   // Apaga el LED rojo
      }
    }
  } else {
    Serial.println("Tarjeta no reconocida");
  }

  mfrc522.PICC_HaltA();  // Detiene la lectura de la tarjeta actual
}

// Función para obtener el índice de la tarjeta si es válida
int getCardIndex(byte *uid) {
  for (int i = 0; i < numCards; i++) {
    if (memcmp(uid, cardUIDs[i], 4) == 0) {
      return i;  // Retorna el índice de la tarjeta válida
    }
  }
  return -1;  // Retorna -1 si la tarjeta no es válida
}
