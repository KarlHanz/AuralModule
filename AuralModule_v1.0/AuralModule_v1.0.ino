/** Demonstrate how to play a file by it's (FAT table) index number.
   https://alexgyver.ru/lessons/serial/
  Данный алгоритм позволяет получить через Serial пачку значений, и раскидать
  их в массив.
  1) В PARSE_AMOUNT указывается, какое количество значений мы хотим принять.
  2) Пакет данных должен иметь вид:
  Начало - символ L - индефикатор (адрес)
  Разделитель - ;
  Завершающий символ - $
  Пример пакета: 4.59;0.05;2.32;0.00;1.11$
  будет раскидан в массив floatData согласно порядку слева направо
*/

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <JQ6500_Serial.h>

JQ6500_Serial mp3(8, 9);

#define PARSE_AMOUNT 2         // число значений в массиве, который хотим получить
int intData[PARSE_AMOUNT];     // массив с названием intData значений после парсинга
boolean recievedFlag;
boolean getStarted;
byte index;
String string_convert = "";

byte n = 2;
byte m = 0;
bool startFlag = 1;



void parsing() {
  if (Serial.available() > 0) {
    char incomingByte = Serial.read();        // обязательно ЧИТАЕМ входящий символ
    if (getStarted) {                         // если приняли начальный символ (парсинг разрешён)
      if (incomingByte != ';' && incomingByte != '$') {   // если это не ; И не конец $
        string_convert += incomingByte;       // складываем в строку
      } else {                                // если это пробел или ; конец пакета
        intData[index] = string_convert.toInt();  // преобразуем строку в Int и кладём в массив intData
        string_convert = "";                  // очищаем строку
        index++;                              // переходим к парсингу следующего элемента массива
      }
    }
    if (incomingByte == 'L') {                // если это L
      getStarted = true;                      // поднимаем флаг, что можно парсить
      index = 0;                              // сбрасываем индекс
      string_convert = "";                    // очищаем строку
    }
    if (incomingByte == '$') {                // если таки приняли $ - конец парсинга
      getStarted = false;                     // сброс
      recievedFlag = true;                    // флаг на принятие
    }
  }
}


void setup() {
  Serial.begin(9600);

  pinMode(4, OUTPUT); // управление модулем RS-485
  digitalWrite(4, LOW); // управление модулем RS-485   // // включаем прием, МЫ готовимся отправить команду

  pinMode(5, OUTPUT); // управление усилителем
  digitalWrite(5, HIGH); // Выключить звук

  pinMode(3, OUTPUT); // GPIO статус
  digitalWrite(3, LOW); // Выключить статус

  mp3.begin(9600);
  mp3.reset();
  mp3.setVolume(30);
  mp3.setEqualizer(MP3_EQ_BASS);
  mp3.setLoopMode(MP3_LOOP_NONE);

}


void loop() {

  parsing();

  if (recievedFlag) {                           // если получены данные
    recievedFlag = false;
    startFlag = 1;
    n = intData[0]; //РАСКИДАТЬ ПОЛУЧЕННЫЙ МАССИВ по переменным
    m = intData[1];
    digitalWrite(5, LOW); // Включить звук
    digitalWrite(3, HIGH); // Включить статус
    digitalWrite(4, HIGH); // управление модулем RS-485 TX
    Serial.print("L"); // индефикатор
    Serial.print(n);
    Serial.print(";");
    Serial.print(m);
    Serial.print("$"); //конец передачи
    Serial.flush(); //ждём окончания передачи всех данных
    digitalWrite(4, LOW); // управление модулем RS-485
    digitalWrite(3, LOW); // Выключить статус

    mp3.setVolume(m);
    mp3.playFileByIndexNumber(n);

  }


}
