// Скетч к проекту "Лампа с 3D эффектами"
// Гайд по постройке матрицы: https://alexgyver.ru/matrix_guide/
// Страница проекта на GitHub: https://github.com/vvip-68/GyverLampWiFi
// Автор: AlexGyver Technologies, 2019
// Дальнейшее развитие: vvip, 2019
// https://AlexGyAver.ru/

// ************************ WIFI ЛАМПА *************************

#define FASTLED_INTERRUPT_RETRY_COUNT 0
#define FASTLED_ALLOW_INTERRUPTS 0

#include "FastLED.h"

#define BRIGHTNESS 32         // стандартная маскимальная яркость (0-255)
#define CURRENT_LIMIT  8000   // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит

#define WIDTH 13              // ширина матрицы
#define HEIGHT 25             // высота матрицы
#define SEGMENTS 1            // диодов в одном "пикселе" (для создания матрицы из кусков ленты)
#define DEVICE_TYPE 0         // Использование матрицы: 0 - свернута в трубу для лампы; 1 - плоская матрица в рамке   

#define COLOR_ORDER GRB       // порядок цветов на ленте. Если цвет отображается некорректно - меняйте. Начать можно с RGB

#define MATRIX_TYPE 0         // тип матрицы: 0 - зигзаг, 1 - параллельная
#define CONNECTION_ANGLE 0    // угол подключения: 0 - левый нижний, 1 - левый верхний, 2 - правый верхний, 3 - правый нижний
#define STRIP_DIRECTION 1     // направление ленты из угла: 0 - вправо, 1 - вверх, 2 - влево, 3 - вниз
                              // при неправильной настрйоке матрицы вы получите предупреждение "Wrong matrix parameters! Set to default"
                              // шпаргалка по настройке матрицы здесь! https://alexgyver.ru/matrix_guide/

// ******************** ЭФФЕКТЫ И РЕЖИМЫ ********************
#define D_EFFECT_SPEED 80     // скорость эффектов по умолчанию (мс)
#define D_EFFECT_SPEED_MIN 0
#define D_EFFECT_SPEED_MAX 255

#define AUTOPLAY_PERIOD 60    // время между авто сменой режимов (секунды)
#define IDLE_TIME 30          // время бездействия (в минутах) после которого запускается автосмена режимов
#define SMOOTH_CHANGE 0       // плавная смена режимов через чёрный
#define USE_MP3 1             // поставьте 0, если у вас нет звуковой карты MP3 плеера

// ****************** ПИНЫ ПОДКЛЮЧЕНИЯ *******************
// пины подписаны согласно pinout платы, а не надписям на пинах!
// esp8266 - плату выбирал "Node MCU v3 (SP-12E Module)"
#define SRX D4       // D3 is RX of ESP8266, connect to TX of DFPlayer
#define STX D3       // D4 is TX of ESP8266, connect to RX of DFPlayer module
#define PIN_BTN D6   // кнопка подключена сюда (PIN --- КНОПКА --- GND)
#define DIO D5       // TM1637 display DIO pin   
#define CLK D7       // TM1637 display CLK pin   

// Используйте данное определение, если у вас МК NodeMCU, в менеджере плат выбрано NodeMCU v1.0 (ESP-12E), лента физически подключена к пину D2 на плате 
#define LED_PIN 2    // пин ленты
// Используйте данное определение, если у вас МК Wemos D1, в менеджере плат выбрано NodeMCU v1.0 (ESP-12E), лента физически подключена к пину D4 на плате 
//#define LED_PIN 4  // пин ленты

// Внимание!!! При использовании платы микроконтроллера Wemos D1 (xxxx) и выбранной в менеджере плат платы "Wemos D1 (xxxx)"
// прошивка скорее всего нормально работать не будет. 
// Наблюдались следующие сбои у разных пользователей:
// - нестабильная работа WiFi (постоянные отваливания и пропадание сети)
// - прекращение вывода контрольной информации в Serial.print() - в монитор порта не выводится
// - настройки в EEPROM не сохраняются
// Думаю все эти проблемы из за некорректной работы ядра ESP8266 для платы (варианта компиляции) Wemox D1 (xxxx) в версии ядра ESP8266 2.5.2
// Диод на ногу питания Wemos как нарисовано в схеме от Alex Gyver можно не ставить, конденсатор по питанию - желателен

// ******************************** ДЛЯ РАЗРАБОТЧИКОВ ********************************
#define DEBUG 0
#define NUM_LEDS WIDTH * HEIGHT * SEGMENTS

CRGB leds[NUM_LEDS];

// ID эффектов
#define MC_FILL_COLOR            0
#define MC_SNOW                  1
#define MC_BALL                  2
#define MC_RAINBOW_HORIZ         3
#define MC_FIRE                  4
#define MC_MATRIX                5
#define MC_BALLS                 6
#define MC_STARFALL              7
#define MC_SPARKLES              8
#define MC_RAINBOW_DIAG          9
#define MC_NOISE_MADNESS        10
#define MC_NOISE_CLOUD          11
#define MC_NOISE_LAVA           12
#define MC_NOISE_PLASMA         13
#define MC_NOISE_RAINBOW        14
#define MC_NOISE_RAINBOW_STRIP  15
#define MC_NOISE_ZEBRA          16
#define MC_NOISE_FOREST         17
#define MC_NOISE_OCEAN          18
#define MC_COLORS               19
#define MC_RAINBOW_VERT         20
#define MC_LIGHTERS             21
#define MC_DAWN_ALARM           22

#define MAX_EFFECT              23         // количество эффектов, определенных в прошивке
#define MAX_SPEC_EFFECT          7         // количество эффектов быстрого доступа, определенных в прошивке

// ---------------------------------
#define MC_DAWN_ALARM_SPIRAL 253           // Специальный режим, вызывается из DEMO_DAWN_ALARM для ламп на круговой матрице - огонек по спирали
#define MC_DAWN_ALARM_SQUARE 254           // Специальный режим, вызывается из DEMO_DAWN_ALARM для плоских матриц - огонек по спирали на плоскости матрицы
// ---------------------------------

// Список и порядок эффектов и игр, передаваймый в приложение на смартфоне. Данные списки попадают в комбобокс выбора, 
// чей индекс передается из приложения в контроллер матрицы для выбора, поэтому порядок должен соответствовать 
// спискам эффектов и игр, определенному выше
#define EFFECT_LIST F("Лампа,Снегопад,Шарик,Радуга вертикальная,Огонь,The Matrix,Шарики,Звездопад,Конфетти,Радуга диагональная,Цветной шум,Облака,Лава,Плазма,Радужные переливы,Полосатые переливы,Зебра,Шумящий лес,Морской прибой,Смена цвета,Радуга горизонтальная,Светлячки,Рассвет")

#if (SMOOTH_CHANGE == 1)
  byte fadeMode = 4;
  boolean modeDir;
#endif

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
bool useSoftAP = false;            // использовать режим точки доступа
bool wifi_connected = false;       // true - подключение к wifi сети выполнена  
bool ap_connected = false;         // true - работаем в режиме точки доступа;
bool wifi_print_ip = false;        // Включен режим отображения текущего IP на индикаторе
byte wifi_print_idx = 0;           // Индекс отображаемой тетрады IP адреса 

#include <TimeLib.h>

uint32_t globalColor = 0xffffff;   // цвет рисования при запуске белый

boolean AUTOPLAY = false;          // false выкл / true вкл автоматическую смену режимов (откл. можно со смартфона)
bool specialMode = false;          // Спец.режим, включенный вручную со смартфона или с кнопок быстрого включения режима
byte specialBrightness = 1;        // Яркость в спец.режиме
bool isTurnedOff = false;          // Включен черный экран (т.е всё выключено) 
int8_t specialModeId = -1;         // Номер текущего спецрежима

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

byte dawnHour = 0;                 // Часы времени начала рассвета
byte dawnMinute = 0;               // Минуты времени начала рассвета
byte dawnWeekDay = 0;              // День недели времени начала рассвета (0 - выключено, 1..7 - пн..вс)
byte dawnDuration = 0;             // Продолжительность "рассвета" по настройкам
byte realDawnDuration = 0;         // Продолжительность "рассвета" по вычисленому времени срабатывания будильника
bool useAlarmSound = false;        // Использовать звуки в будильнике
int8_t alarmSound = 0;             // Звук будильника - номер файла в папке SD:/01 [-1 не использовать; 0 - случайный; 1..N] номер файла
int8_t dawnSound = 0;              // Звук рассвета   - номер файла в папке SD:/02 [-1 не использовать; 0 - случайный; 1..N] номер файла
byte maxAlarmVolume = 30;          // Максимальная громкость будильника (1..30)
byte alarmEffect = MC_DAWN_ALARM;  // Какой эффект используется для будильника "рассвет". Могут быть обычные эффекты - их яркость просто будет постепенно увеличиваться
bool needTurnOffClock = false;     // Выключать индикатор часов при выключении лампы

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
int8_t hrs = 0, mins = 0, secs = 0, aday = 1, amnth = 1;
int16_t ayear = 1970;
bool dotFlag;
bool eepromModified = false;

bool aDirection = true;
byte aCounter = 0;
byte bCounter = 0;
unsigned long fade_time;

byte effectScaleParam[MAX_EFFECT]; // Динамический параметр эффекта

#include <EEPROM.h>

uint32_t idleTime = ((long)IDLE_TIME * 60 * 1000);      // минуты -> миллисек
uint32_t autoplayTime = ((long)AUTOPLAY_PERIOD * 1000); // секунды -> миллисек
uint32_t autoplayTimer;

#include "timerMinim.h"
timerMinim effectTimer(D_EFFECT_SPEED);  // Таймер скорости эффекта (шага выполнения эффекта)
timerMinim changeTimer(70);              // Таймер шага плавной смены режима - Fade
timerMinim halfsecTimer(500);            // Полусекундный таймер точек часов
timerMinim idleTimer(idleTime);          // Таймер бездействия ручного управлениядля автоперехода а демо-режим 
timerMinim dawnTimer(1000);              // Таймер шага рассвета для будильника "рассвет" 
timerMinim alarmSoundTimer(4294967295);  // Таймер выключения звука будильника
timerMinim fadeSoundTimer(4294967295);   // Таймер плавного включения / выключения звука
timerMinim saveSettingsTimer(30000);     // Таймер отложенного сохранения настроек

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

#include <SoftwareSerial.h>
SoftwareSerial mp3Serial(SRX, STX);

#include "DFRobotDFPlayerMini.h"     // Установите в менеджере библиотек стандартную библиотеку DFRobotDFPlayerMini ("DFPlayer - A Mini MP3 Player For Arduino" )
DFRobotDFPlayerMini dfPlayer; 
bool isDfPlayerOk = false;
int16_t alarmSoundsCount = 0;        // Кол-во файлов звуков в папке '01' на SD-карте
int16_t dawnSoundsCount = 0;         // Кол-во файлов звуков в папке '02' на SD-карте
byte soundFolder = 0;
byte soundFile = 0;
int8_t fadeSoundDirection = 1;       // направление изменения громкости звука: 1 - увеличение; -1 - уменьшение
byte fadeSoundStepCounter = 0;       // счетчик шагов изменения громкости, которое осталось сделать

#include "GyverButton.h"
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

#include "GyverTM1637.h"
GyverTM1637 disp(CLK, DIO);

void setup() {

  ESP.wdtEnable(WDTO_8S);
  
  Serial.begin(115200);
  delay(10);
  
  Serial.println();
  
  EEPROM.begin(256);
  loadSettings();

  randomSeed(analogRead(1));    // пинаем генератор случайных чисел

  // Первый этап инициализации плеера - подключение и основные настройки
  #if (USE_MP3 == 1)
    InitializeDfPlayer1();
  #endif
     
  WiFi.setSleepMode(WIFI_NONE_SLEEP);

  connectToNetwork();

  // UDP-клиент на указанном порту
  udp.begin(localPort);

  butt.setStepTimeout(100);
  butt.setClickTimeout(500);

  disp.brightness(7);  // яркость, 0 - 7 (минимум - максимум)
  disp.displayByte(_empty, _empty, _empty, _empty);
  
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

  // Если был задан спец.режим во время предыдущего сеанса работы матрицы - включить его
  // Номер спец-режима запоминается при его включении и сбрасывается при включении обычного режима или игры
  // Это позволяет в случае внезапной перезагрузки матрицы (например по wdt), когда был включен спец-режим (например ночные часы или выкл. лампы)
  // снова включить его, а не отображать случайный обычный после включения матрицы
  int8_t spc_mode = getCurrentSpecMode();
  if (spc_mode >= 0 && spc_mode <= MAX_SPEC_EFFECT) {
    setSpecialMode(spc_mode);
    isTurnedOff = spc_mode == 0;
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
    setTimersForMode(thisMode);
    AUTOPLAY = true;
    autoplayTimer = millis();
  }
}

void loop() {
  process();
  ESP.wdtFeed();
}

// -----------------------------------------

void startWiFi() { 
  
  WiFi.disconnect(true);
  wifi_connected = false;
  WiFi.mode(WIFI_STA);
 
  // Пытаемся соединиться с роутером в сети
  if (strlen(ssid) > 0) {
    Serial.print(F("Подключение к "));
    Serial.print(ssid);

    if (IP_STA[0] + IP_STA[1] + IP_STA[2] + IP_STA[3] > 0) {
      WiFi.config(IPAddress(IP_STA[0], IP_STA[1], IP_STA[2], IP_STA[3]),  // 192.168.0.116
                  IPAddress(IP_STA[0], IP_STA[1], IP_STA[2], 1),          // 192.168.0.1
                  IPAddress(255, 255, 255, 0));
    }              
    WiFi.begin(ssid, pass);
  
    // Проверка соединения (таймаут 10 секунд)
    for (int j = 0; j < 10; j++ ) {
      wifi_connected = WiFi.status() == WL_CONNECTED; 
      if (wifi_connected) {
        // Подключение установлено
        Serial.println();
        Serial.print(F("WiFi подключен. IP адрес: "));
        Serial.println(WiFi.localIP());
        break;
      }
      ESP.wdtFeed();
      delay(1000);
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
    ESP.wdtFeed();
    delay(1000);
    
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
