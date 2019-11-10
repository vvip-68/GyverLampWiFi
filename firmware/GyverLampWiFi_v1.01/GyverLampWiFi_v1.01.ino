// Скетч к проекту "Лампа с 3D эффектами"
// Гайд по постройке матрицы: https://alexgyver.ru/matrix_guide/
// Страница проекта на GitHub: https://github.com/vvip-68/GyverLampWiFi
// Автор: AlexGyver Technologies, 2019
// Дальнейшее развитие: vvip, 2019
// https://AlexGyver.ru/

// http://arduino.esp8266.com/stable/package_esp8266com_index.json
// https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json


// ************************ WIFI ЛАМПА *************************

#define FIRMWARE_VER F("\n\nGyverLamp-WiFi v.1.01.2019.1108")
#define FASTLED_INTERRUPT_RETRY_COUNT 0
#define FASTLED_ALLOW_INTERRUPTS 0

// Подключение используемых библиотек
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif

#if defined(ESP32)
  #include <WiFi.h>
#ifndef min
  #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
  #define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#endif

#include <WiFiUdp.h>
#include <TimeLib.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>          // Установите в менеджере библиотек "EspSoftwareSerial" для ESP8266/ESP32 https://github.com/plerup/espsoftwareserial/
#include "FastLED.h"                 // Установите в менеджере библиотек стандартную библиотеку FastLED
#include "TM1637Display.h"
#include "DFRobotDFPlayerMini.h"     // Установите в менеджере библиотек стандартную библиотеку DFRobotDFPlayerMini ("DFPlayer - A Mini MP3 Player For Arduino" )
#include "timerMinim.h"
#include "GyverButton.h"
#include "fonts.h"

#define BRIGHTNESS 32         // стандартная маскимальная яркость (0-255)
uint16_t CURRENT_LIMIT=5000;  // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит

// ****************** ПИНЫ ПОДКЛЮЧЕНИЯ *******************

// Внимание!!! При использовании платы микроконтроллера Wemos D1 (xxxx) и выбранной в менеджере плат платы "Wemos D1 (xxxx)"
// прошивка скорее всего нормально работать не будет. 
// Наблюдались следующие сбои у разных пользователей:
// - нестабильная работа WiFi (постоянные отваливания и пропадание сети) - попробуйте варианты с разным значением параметров компиляции IwIP в Arduino IDE
// - прекращение вывода контрольной информации в Serial.print() - в монитор порта не выводится
// - настройки в EEPROM не сохраняются
// Думаю все эти проблемы из за некорректной работы ядра ESP8266 для платы (варианта компиляции) Wemos D1 (xxxx) в версии ядра ESP8266 2.5.2
// Диод на ногу питания Wemos как нарисовано в схеме от Alex Gyver не ставить (!!!), конденсатор по питанию для NodeMCU - желателен, для Wemos - обязателен (!!!)
// Пины подписаны согласно pinout платы, а не надписям на пинах!

/*
 * NodeMCU v1.0 (ESP-12E)
 * Физическое подключение:
 * Матрица зигзаг, левый нижний угол, направление вапрво
 * Пин ленты    - D2
 * Пин кнопки   - D6
 * MP3Player    - D3 к RX, D4 к TX плеера 
 * TM1637       - D5 к DIO, D7 к CLK
 * В менеджере плат выбрано NodeMCU v1.0 (ESP-12E)
 */
#if defined(ESP8266)
#define WIDTH 19              // ширина матрицы
#define HEIGHT 9              // высота матрицы
#define SEGMENTS 1            // диодов в одном "пикселе" (для создания матрицы из кусков ленты)
#define DEVICE_TYPE 0         // Использование матрицы: 0 - свернута в трубу для лампы; 1 - плоская матрица в рамке   
#define MATRIX_TYPE 0         // тип матрицы: 0 - зигзаг, 1 - параллельная
#define CONNECTION_ANGLE 0    // угол подключения: 0 - левый нижний, 1 - левый верхний, 2 - правый верхний, 3 - правый нижний
#define STRIP_DIRECTION 0     // направление ленты из угла: 0 - вправо, 1 - вверх, 2 - влево, 3 - вниз
#define USE_MP3 1             // поставьте 0, если у вас нет звуковой карты MP3 плеера

#define LED_PIN 2             // пин ленты, физически подключена к пину D2 на плате 
#define SRX D4                // D4 is RX of ESP8266, connect to TX of DFPlayer
#define STX D3                // D3 is TX of ESP8266, connect to RX of DFPlayer module
#define PIN_BTN D6            // кнопка подключена сюда (PIN --- КНОПКА --- GND)
#define DIO D5                // TM1637 display DIO pin
#define CLK D7                // TM1637 display CLK pin
#endif

/*
 * NodeMCU ESP32
 * Физическое подключение:
 * Матрица зигзаг, левый нижний угол, направление вапрво
 * Пин ленты    - 2
 * Пин кнопки   - 4
 * MP3Player    - 17 к RX, 16 к TX плеера 
 * TM1637       - 23 к DIO, 22 к CLK
 * В менеджере плат выбрано "ESP32 Dev Module"
 */ 
#if defined(ESP32)
#define WIDTH 16              // ширина матрицы
#define HEIGHT 16             // высота матрицы
#define SEGMENTS 1            // диодов в одном "пикселе" (для создания матрицы из кусков ленты)
#define DEVICE_TYPE 0         // Использование матрицы: 0 - свернута в трубу для лампы; 1 - плоская матрица в рамке   
#define MATRIX_TYPE 0         // тип матрицы: 0 - зигзаг, 1 - параллельная
#define CONNECTION_ANGLE 0    // угол подключения: 0 - левый нижний, 1 - левый верхний, 2 - правый верхний, 3 - правый нижний
#define STRIP_DIRECTION 0     // направление ленты из угла: 0 - вправо, 1 - вверх, 2 - влево, 3 - вниз
#define USE_MP3 1             // поставьте 0, если у вас нет звуковой карты MP3 плеера

#define LED_PIN (2U)          // пин ленты, физически подключена к пину D2 на плате
#define SRX (16U)             // 16 is RX of ESP32, connect to TX of DFPlayer
#define STX (17U)             // 17 is TX of ESP32, connect to RX of DFPlayer module
#define PIN_BTN (4U)          // кнопка подключена сюда (PIN --- КНОПКА --- GND)
#define DIO (23U)             // TM1637 display DIO pin
#define CLK (22U)             // TM1637 display CLK pin
#endif

#define COLOR_ORDER GRB       // порядок цветов на ленте. Если цвет отображается некорректно - меняйте. Начать можно с RGB

// ******************** ЭФФЕКТЫ И РЕЖИМЫ ********************

#define D_EFFECT_SPEED 80     // скорость эффектов по умолчанию (мс)
#define D_EFFECT_SPEED_MIN 0
#define D_EFFECT_SPEED_MAX 255

#define D_CLOCK_SPEED 100      // скорость перемещения эффекта часов по умолчанию (мс)
#define D_CLOCK_SPEED_MIN 10
#define D_CLOCK_SPEED_MAX 255


#define AUTOPLAY_PERIOD 60    // время между авто сменой режимов (секунды)
#define IDLE_TIME 30          // время бездействия (в минутах) после которого запускается автосмена режимов
#define SMOOTH_CHANGE 0       // плавная смена режимов через чёрный

// ******************************** ДЛЯ РАЗРАБОТЧИКОВ ********************************

#define DEBUG 0
#define NUM_LEDS WIDTH * HEIGHT * SEGMENTS

CRGB leds[NUM_LEDS];

// ID эффектов
#define MC_FILL_COLOR            0
#define MC_SNOW                  1
#define MC_BALL                  2
#define MC_RAINBOW_HORIZ         3
#define MC_PAINTBALL             4
#define MC_FIRE                  5
#define MC_MATRIX                6
#define MC_BALLS                 7
#define MC_STARFALL              8
#define MC_SPARKLES              9
#define MC_RAINBOW_DIAG         10
#define MC_TEXT                 11
#define MC_NOISE_MADNESS        12
#define MC_NOISE_CLOUD          13
#define MC_NOISE_LAVA           14
#define MC_NOISE_PLASMA         15
#define MC_NOISE_RAINBOW        16
#define MC_NOISE_RAINBOW_STRIP  17
#define MC_NOISE_ZEBRA          18
#define MC_NOISE_FOREST         19
#define MC_NOISE_OCEAN          20
#define MC_COLORS               21
#define MC_RAINBOW_VERT         22
#define MC_LIGHTERS             23
#define MC_CLOCK                24
#define MC_DAWN_ALARM           25

#define MAX_EFFECT              26         // количество эффектов, определенных в прошивке
#define MAX_SPEC_EFFECT         11         // количество эффектов быстрого доступа определенных в прошивке -> 0..10

// ---------------------------------
#define MC_DAWN_ALARM_SPIRAL 253           // Специальный режим, вызывается из DEMO_DAWN_ALARM для ламп на круговой матрице - огонек по спирали
#define MC_DAWN_ALARM_SQUARE 254           // Специальный режим, вызывается из DEMO_DAWN_ALARM для плоских матриц - огонек по спирали на плоскости матрицы
// ---------------------------------

// Список и порядок эффектов и игр, передаваймый в приложение на смартфоне. Данные списки попадают в комбобокс выбора, 
// чей индекс передается из приложения в контроллер матрицы для выбора, поэтому порядок должен соответствовать 
// спискам эффектов и игр, определенному выше
#define EFFECT_LIST F("Лампа,Снегопад,Шарик,Радуга вертикальная,Пейнтбол,Огонь,The Matrix,Шарики,Звездопад,Конфетти,Радуга диагональная,Часы с датой,Цветной шум,Облака,Лава,Плазма,Радужные переливы,Полосатые переливы,Зебра,Шумящий лес,Морской прибой,Смена цвета,Радуга горизонтальная,Светлячки,Часы,Рассвет")

#if (SMOOTH_CHANGE == 1)
  byte fadeMode = 4;
  boolean modeDir;
#endif

// Макрос центрирования отображения часов на матрице
#define CLOCK_X_H (byte((WIDTH - (4*3 + 3*1)) / 2.0 + 0.51))  // 4 цифры * (шрифт 3 пикс шириной) 3 + пробела между цифрами), /2 - в центр 
#define CLOCK_Y_H (byte((HEIGHT - (1*5)) / 2.0 + 0.51))       // Одна строка цифр 5 пикс высотой  / 2 - в центр
#define CLOCK_X_V (byte((WIDTH - (2*3 + 1)) / 2.0 + 0.51))    // 2 цифры * (шрифт 3 пикс шириной) 1 + пробел между цифрами) /2 - в центр
#define CLOCK_Y_V (byte((HEIGHT - (2*5 + 1)) / 2.0 + 0.51))   // Две строки цифр 5 пикс высотой + 1 пробел между строкми / 2 - в центр

// Позиции отображения
byte CLOCK_X = CLOCK_X_H;     // Для вертикальных часов CLOCK_X_V и CLOCK_Y_V
byte CLOCK_Y = CLOCK_Y_H;

int8_t CLOCK_XC = CLOCK_X;                    // Текущая позиция часов оверлея по оси Х 
byte   CLOCK_MOVE_DIVIDER = 10;               // Делитель такта таймера эффектов для движения часов оверлея
byte   CLOCK_MOVE_CNT = CLOCK_MOVE_DIVIDER;   // Текущее значения счетчика сдвига 

// Ориентация отображения часов
byte CLOCK_ORIENT = 0;       // 0 горизонтальные, 1 вертикальные


byte COLOR_MODE = 0;         // Режим цвета часов
//                               0 - монохром
//                               1 - радужная смена (каждая цифра)
//                               2 - радужная смена (часы, точки, минуты)
//                               3 - заданные цвета (часы, точки, минуты) - HOUR_COLOR, DOT_COLOR, MIN_COLOR в clock.ino

byte COLOR_TEXT_MODE = 0;    // Режим цвета часов бегущей строки
//                               0 - монохром
//                               1 - радужная смена (каждая цифра)
//                               2 - каждая цифра свой цвет

bool useSoftAP = false;            // использовать режим точки доступа
bool wifi_connected = false;       // true - подключение к wifi сети выполнена  
bool ap_connected = false;         // true - работаем в режиме точки доступа;
bool wifi_print_ip = false;        // Включен режим отображения текущего IP на индикаторе
byte wifi_print_idx = 0;           // Индекс отображаемой тетрады IP адреса 
String wifi_current_ip = "";       // Отображаемый в бегущей строке IP адрес лампы

uint32_t globalColor = 0xffffff;      // цвет рисования при запуске белый
uint32_t globalClockColor = 0xffffff; // цвет часов в режиме MC_COLOR - режим цвета "Монохром"
uint32_t globalTextColor = 0xffffff;  // цвет часов бегущей строки MC_TEXT  в режиме цвета "Монохром"

bool AUTOPLAY = false;             // false выкл / true вкл автоматическую смену режимов (откл. можно со смартфона)
bool specialMode = false;          // Спец.режим, включенный вручную со смартфона или с кнопок быстрого включения режима
bool specialClock = false;         // Отображение часов в спец.режиме
byte specialBrightness = 1;        // Яркость в спец.режиме
bool isTurnedOff = false;          // Включен черный экран (т.е всё выключено) 
bool isNightClock = false;         // Включен режим ночных часов
int8_t specialModeId = -1;         // Номер текущего спецрежима
int8_t manualModeId = -1;          // Номер текущего режима
bool useRandomSequence = true;     // Флаг: случайный выбор режима
byte formatClock = 0;              // Формат часов в бегущей строке: 0 - только часы; 1 - часы и дата кратко; 2 0 часы и дата строкой
bool overlayEnabled = true;        // Доступность оверлея часов поверх эффектов

bool saveSpecialMode = specialMode;
int8_t saveSpecialModeId = specialModeId;
byte saveMode;

bool isAlarming = false;           // Сработал будильник "рассвет"
bool isPlayAlarmSound = false;     // Сработал настоящий будильник - играется звук будильника
bool isAlarmStopped = false;       // Сработавший будильник "рассвет" остановлен пользователем

byte alarmWeekDay = 0;             // Битовая маска дней недели будильника
byte alarmDuration = 1;            // Проигрывать звук будильнике N минут после срабатывания (по окончанию рассвета)

byte alarmHour[7]   = {0,0,0,0,0,0,0};  // Часы времени срабатывания будильника
byte alarmMinute[7] = {0,0,0,0,0,0,0};  // Минуты времени срабатывания будильника

int8_t dawnHour = 0;               // Часы времени начала рассвета
int8_t dawnMinute = 0;             // Минуты времени начала рассвета
byte dawnWeekDay = 0;              // День недели времени начала рассвета (0 - выключено, 1..7 - пн..вс)
byte dawnDuration = 0;             // Продолжительность "рассвета" по настройкам
byte realDawnDuration = 0;         // Продолжительность "рассвета" по вычисленому времени срабатывания будильника
bool useAlarmSound = false;        // Использовать звуки в будильнике
int8_t alarmSound = 0;             // Звук будильника - номер файла в папке SD:/01 [-1 не использовать; 0 - случайный; 1..N] номер файла
int8_t dawnSound = 0;              // Звук рассвета   - номер файла в папке SD:/02 [-1 не использовать; 0 - случайный; 1..N] номер файла
byte maxAlarmVolume = 30;          // Максимальная громкость будильника (1..30)
byte alarmEffect = MC_DAWN_ALARM;  // Какой эффект используется для будильника "рассвет". Могут быть обычные эффекты - их яркость просто будет постепенно увеличиваться
bool needTurnOffClock = false;     // Выключать индикатор часов при выключении лампы
byte nightClockColor = 0;          // Цвет ночных часов: 0 - R; 1 - G; 2 - B; 3 - C; 4 - M; 5 - Y;

// Сервер не может инициировать отправку сообщения клиенту - только в ответ на запрос клиента
// Следующие две переменные хранят сообщения, формируемые по инициативе сервира и отправляются в ответ на ближайший запрос от клиента,
// например в ответ на периодический ping - в команде sendAcknowledge();
String cmd95 = "";                 // Строка, формируемая sendPageParams(95) для отправки по инициативе сервера
String cmd96 = "";                 // Строка, формируемая sendPageParams(96) для отправки по инициативе сервера

static const byte maxDim = max(WIDTH, HEIGHT);
byte globalBrightness = BRIGHTNESS;
bool brightDirection = false;      // true - увеличение яркости; false - уменьшение яркости

int effectSpeed = D_EFFECT_SPEED;  // скрость изменения эффекта (по умолчанию)
bool BTcontrol = false;            // флаг: true - ручное управление эффектами; false - в режиме Autoplay
bool loadingFlag = true;           // флаг: инициализация параметров эффекта
bool idleState = true;             // флаг холостого режима работы
int8_t thisMode = 0;               // текущий режим
byte modeCode;                     // тип текущего эффекта: 0 бегущая строка, 1 часы, 2 игры, 3 нойс маднесс и далее, 21 гифка или картинка,
int8_t hrs = 0, mins = 0, secs = 0, aday = 1, amnth = 1;
int16_t ayear = 1970;
bool dotFlag;                      // флаг: в часах рисуется точки 
bool eepromModified = false;       // флаг: EEPROM изменен, требует сохранения
boolean fullTextFlag = false;      // флаг: текст бегущей строки показан полностью (строка убежала)

bool aDirection = true;
byte aCounter = 0;
byte bCounter = 0;
unsigned long fade_time;

byte effectScaleParam[MAX_EFFECT]; // Динамический параметр эффекта


uint32_t idleTime = ((long)IDLE_TIME * 60 * 1000);      // минуты -> миллисек
uint32_t autoplayTime = ((long)AUTOPLAY_PERIOD * 1000); // секунды -> миллисек
uint32_t autoplayTimer;

timerMinim effectTimer(D_EFFECT_SPEED);  // Таймер скорости эффекта (шага выполнения эффекта)
timerMinim clockTimer(D_EFFECT_SPEED);   // Таймер смещения оверлея часов
timerMinim changeTimer(70);              // Таймер шага плавной смены режима - Fade
timerMinim halfsecTimer(500);            // Полусекундный таймер точек часов
timerMinim idleTimer(idleTime);          // Таймер бездействия ручного управлениядля автоперехода а демо-режим 
timerMinim dawnTimer(1000);              // Таймер шага рассвета для будильника "рассвет" 
timerMinim alarmSoundTimer(4294967295);  // Таймер выключения звука будильника
timerMinim fadeSoundTimer(4294967295);   // Таймер плавного включения / выключения звука
timerMinim saveSettingsTimer(15000);     // Таймер отложенного сохранения настроек

// ---------------------------------------------------------------

                                         // Внимание!!! Если вы меняете эти значения ПОСЛЕ того, как прошивка уже хотя бы раз была загружена в плату и выполнялась,
                                         // чтобы изменения вступили в силу нужно также изменить значение константы EEPROM_OK в первой строке в файле eeprom.ino 
                                         // 

#define DEFAULT_NTP_SERVER "ru.pool.ntp.org" // NTP сервер по умолчанию "time.nist.gov"
#define DEFAULT_AP_NAME "LampAP"         // Имя точки доступа по умолчанию 
#define DEFAULT_AP_PASS "12341234"       // Пароль точки доступа по умолчанию
#define NETWORK_SSID ""                  // Имя WiFi сети
#define NETWORK_PASS ""                  // Пароль для подключения к WiFi сети

// ---------------------------------------------------------------
                                         // к длине +1 байт на \0 - терминальный символ. Это буферы для загрузки имен/пароля из EEPROM. Значения задаются в defiine выше
char apName[11] = "";                    // Имя сети в режиме точки доступа
char apPass[17]  = "";                   // Пароль подключения к точке доступа
char ssid[25] = "";                      // SSID (имя) вашего роутера (конфигурируется подключением через точку доступа и сохранением в EEPROM)
char pass[17] = "";                      // пароль роутера

WiFiUDP udp;
unsigned int localPort = 2390;           // local port to listen for UDP packets
byte IP_STA[] = {192, 168, 0, 116};      // Статический адрес в локальной сети WiFi

IPAddress timeServerIP;
#define NTP_PACKET_SIZE 48               // NTP время в первых 48 байтах сообщения
uint16_t SYNC_TIME_PERIOD = 60;          // Период синхронизации в минутах
byte packetBuffer[NTP_PACKET_SIZE];      // буфер для хранения входящих и исходящих пакетов NTP

int8_t timeZoneOffset = 7;               // смещение часового пояса от UTC
long ntp_t = 0;                          // Время, прошедшее с запроса данных с NTP-сервера (таймаут)
byte ntp_cnt = 0;                        // Счетчик попыток получить данные от сервера
bool init_time = false;                  // Флаг false - время не инициализировано; true - время инициализировано
bool refresh_time = true;                // Флаг true - пришло время выполнить синхронизацию часов с сервером NTP
bool useNtp = true;                      // Использовать синхронизацию времени с NTP-сервером
char ntpServerName[31] = "";             // Используемый сервер NTP

timerMinim ntpSyncTimer(1000 * 60 * SYNC_TIME_PERIOD);            // Сверяем время с NTP-сервером через SYNC_TIME_PERIOD минут

SoftwareSerial mp3Serial;

DFRobotDFPlayerMini dfPlayer; 
bool isDfPlayerOk = false;
int16_t alarmSoundsCount = 0;        // Кол-во файлов звуков в папке '01' на SD-карте
int16_t dawnSoundsCount = 0;         // Кол-во файлов звуков в папке '02' на SD-карте
byte soundFolder = 0;
byte soundFile = 0;
int8_t fadeSoundDirection = 1;       // направление изменения громкости звука: 1 - увеличение; -1 - уменьшение
byte fadeSoundStepCounter = 0;       // счетчик шагов изменения громкости, которое осталось сделать

//GButton butt(PIN_BTN, LOW_PULL, NORM_OPEN); // Для сенсорной кнопки
GButton butt(PIN_BTN, HIGH_PULL, NORM_OPEN);  // Для обычной кнопки

bool isButtonHold = false;           // Кнопка нажата и удерживается

bool AM1_running = false;            // Режим 1 по времени - работает
byte AM1_hour = 0;                   // Режим 1 по времени - часы
byte AM1_minute = 0;                 // Режим 1 по времени - минуты
int8_t AM1_effect_id = -3;           // Режим 1 по времени - ID эффекта или -3 - выключено (не используется); -2 - выключить матрицу (черный экран); -1 - огонь, 0 - случайный, 1 и далее - эффект EFFECT_LIST
bool AM2_running = false;            // Режим 2 по времени - работает
byte AM2_hour = 0;                   // Режим 2 по времени - часы
byte AM2_minute = 0;                 // Режим 2 по времени - минуты
int8_t AM2_effect_id = -3;           // Режим 2 по времени - ID эффекта или -3 - выключено (не используется); -2 - выключить матрицу (черный экран); -1 - огонь, 0 - случайный, 1 и далее - эффект EFFECT_LIST

TM1637Display display(CLK, DIO);

void setup() {
#if defined(ESP8266)
  ESP.wdtEnable(WDTO_8S);
#endif
  Serial.begin(115200);
  delay(10);
  
  Serial.println(FIRMWARE_VER);
  
  EEPROM.begin(384);
  loadSettings();

  randomSeed(analogRead(0) ^ millis());    // пинаем генератор случайных чисел

  // Первый этап инициализации плеера - подключение и основные настройки
  #if (USE_MP3 == 1)
    InitializeDfPlayer1();
  #endif
     
  #if defined(ESP8266)
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
  #endif

  connectToNetwork();

  // UDP-клиент на указанном порту
  udp.begin(localPort);

  butt.setStepTimeout(100);
  butt.setClickTimeout(500);

  display.setBrightness(7);
  display.displayByte(_empty, _empty, _empty, _empty);
  
  // Таймер бездействия
  if (idleTime == 0) // Таймер Idle  отключен
    idleTimer.setInterval(4294967295);
  else  
    idleTimer.setInterval(idleTime);
  
  dawnTimer.setInterval(4294967295);
  
  // настройки ленты
  FastLED.addLeds<WS2812, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(globalBrightness);
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.clear();
  FastLED.show();

  // Второй этап инициализации плеера - проверка наличия файлов звуков на SD карте
  #if (USE_MP3 == 1)
    InitializeDfPlayer2();
    if (!isDfPlayerOk) {
      Serial.println(F("MP3 плеер недоступен."));
    }
  #endif

  // Проверить соответствие позиции вывода часов размерам матрицы
  checkClockOrigin();
  
  // Если был задан спец.режим во время предыдущего сеанса работы матрицы - включить его
  // Номер спец-режима запоминается при его включении и сбрасывается при включении обычного режима или игры
  // Это позволяет в случае внезапной перезагрузки матрицы (например по wdt), когда был включен спец-режим (например ночные часы или выкл. лампы)
  // снова включить его, а не отображать случайный обычный после включения матрицы
  int8_t spc_mode = getCurrentSpecMode();
  if (spc_mode >= 0 && spc_mode < MAX_SPEC_EFFECT) {
    setSpecialMode(spc_mode);
    isTurnedOff = spc_mode == 0;
    isNightClock = spc_mode == 8;
  } else {
    thisMode = getCurrentManualMode();
    if (thisMode < 0 || AUTOPLAY) {
      setRandomMode2();
    } else {
      while (1) {
        // Если режим отмечен флагом "использовать" - используем его, иначе берем следующий (и проверяем его)
        if (getEffectUsage(thisMode)) break;
        thisMode++;
        if (thisMode >= MAX_EFFECT) {
          thisMode = 0;
          break;
        }
      }
      setEffect(thisMode);        
    }
  }
  autoplayTimer = millis();
}

void loop() {
  process();
}

// -----------------------------------------

void startWiFi() { 
  
  WiFi.disconnect(true);
  wifi_connected = false;
  WiFi.mode(WIFI_STA);
 
  // Пытаемся соединиться с роутером в сети
  if (strlen(ssid) > 0) {
    Serial.print(F("\nПодключение к "));
    Serial.print(ssid);

    if (IP_STA[0] + IP_STA[1] + IP_STA[2] + IP_STA[3] > 0) {
      WiFi.config(IPAddress(IP_STA[0], IP_STA[1], IP_STA[2], IP_STA[3]),  // 192.168.0.116
                  IPAddress(IP_STA[0], IP_STA[1], IP_STA[2], 1),          // 192.168.0.1
                  IPAddress(255, 255, 255, 0));
    }              
    WiFi.begin(ssid, pass);
  
    // Проверка соединения (таймаут 5 секунд)
    for (int j = 0; j < 10; j++ ) {
      wifi_connected = WiFi.status() == WL_CONNECTED; 
      if (wifi_connected) {
        // Подключение установлено
        Serial.println();
        Serial.print(F("WiFi подключен. IP адрес: "));
        Serial.println(WiFi.localIP());
        break;
      }
      delay(500);
      Serial.print(".");
    }
    Serial.println();
    
    if (!wifi_connected)
      Serial.println(F("Не удалось подключиться к сети WiFi."));
  }  
}

void startSoftAP() {
  WiFi.softAPdisconnect(true);
  ap_connected = false;

  Serial.print(F("Создание точки доступа "));
  Serial.print(apName);
  
  ap_connected = WiFi.softAP(apName, apPass);

  for (int j = 0; j < 10; j++ ) {    
    if (ap_connected) {
      Serial.println();
      Serial.print(F("Точка доступа создана. Сеть: '"));
      Serial.print(apName);
      // Если пароль совпадает с паролем по умолчанию - печатать для информации,
      // если был изменен пользователем - не печатать
      if (strcmp(apPass, "12341234") == 0) {
        Serial.print(F("'. Пароль: '"));
        Serial.print(apPass);
      }
      Serial.println(F("'."));
      Serial.print(F("IP адрес: "));
      Serial.println(WiFi.softAPIP());
      break;
    }    
    
    WiFi.enableAP(false);
    WiFi.softAPdisconnect(true);
    delay(500);
    
    Serial.print(".");
    ap_connected = WiFi.softAP(apName, apPass);
  }  
  Serial.println();  
  
  if (!ap_connected) 
    Serial.println(F("Не удалось создать WiFi точку доступа."));
}

void printNtpServerName() {
  Serial.print(F("NTP-сервер "));
  Serial.print(ntpServerName);
  Serial.print(F(" -> "));
  Serial.println(timeServerIP);
}

void connectToNetwork() {
  // Подключиться к WiFi сети
  startWiFi();

  // Если режим точки тоступане используется и к WiFi сети подключиться не удалось - создать точку доступа
  if (!wifi_connected){
    WiFi.mode(WIFI_AP);
    startSoftAP();
  }

  if (useSoftAP && !ap_connected) startSoftAP();    

  // Сообщить UDP порт, на который ожидаются подключения
  if (wifi_connected || ap_connected) {
    Serial.print(F("UDP-сервер на порту "));
    Serial.println(localPort);
  }
}
