#include <FlashStorage.h>

struct ToStoreData {
    String  UserName;
    int     UserAge;
}

FlashStorage(onBoardMemory, ToStoreData);

void setup() {
    Serial.begin(9600);
    ToStoreData dataToWrite = {"Sin Kiat", 20};
    onBoardMemory.write(dataToWrite);
}

void loop() {
    ToStoreData readData = onBoardMemory.read();
    Serial.print("Name: ");
    Serial.println(readData.UserName);
    Serial.print("Age: ");
    Serial.println(readData.UserAge);
}

