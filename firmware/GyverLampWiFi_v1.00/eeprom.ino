#define EEPROM_OK 0x55                     // Флаг, показывающий, что EEPROM инициализирована корректными данными 
#define EFFECT_EEPROM 150                  // начальная ячейка eeprom с параметрами эффектов

void loadSettings() {

  // Адреса в EEPROM:
  //    0 - если EEPROM_OK - EEPROM инициализировано, если другое значение - нет 
  //    1 - максимальная яркость ленты 1-255
  //    2 - автосмена режима в демо: вкл/выкл
  //    3 - время автосмены режимов
  //    4 - время бездействия до переключения в авторежим
  //    5 - использовать синхронизацию времени через NTP
  //  6,7 - период синхронизации NTP (int16_t - 2 байта)
  //    8 - time zone
  //    9 - выключать индикатор часов при выключении лампы true - выключать / false - не выключать
  //   10 - IP[0]
  //   11 - IP[1]
  //   12 - IP[2]
  //   13 - IP[3]
  //   14 - Использовать режим точки доступа
  //   15 - зарезервировано
  //   ...
  //   19 - зарезервировано
  //   20 - Будильник, дни недели
  //   21 - Будильник, продолжительность "рассвета"
  //   22 - Будильник, эффект "рассвета"
  //   23 - Будильник, использовать звук
  //   24 - Будильник, играть звук N минут после срабатывания
  //   25 - Будильник, Номер мелодии будильника (из папки 01 на SD карте)
  //   26 - Будильник, Номер мелодии рассвета (из папки 02 на SD карте) 
  //   27 - Будильник, Максимальная громкость будильника
  //   28 - зарезервировано
  //   ...
  //   32 - зарезервировано
  //   33 - Режим 1 по времени - часы
  //   34 - Режим 1 по времени - минуты
  //   35 - Режим 1 по времени - ID эффекта или -1 - выключено; 0 - случайный;
  //   36 - Режим 2 по времени - часы
  //   37 - Режим 2 по тайвременимеру - минуты
  //   38 - Режим 2 по времени - ID эффекта или -1 - выключено; 0 - случайный;
  //   40 - Будильник, время: понедельник : часы
  //   41 - Будильник, время: понедельник : минуты
  //   42 - Будильник, время: вторник : часы
  //   43 - Будильник, время: вторник : минуты
  //   44 - Будильник, время: среда : часы
  //   45 - Будильник, время: среда : минуты
  //   46 - Будильник, время: четверг : часы
  //   47 - Будильник, время: четверг : минуты
  //   48 - Будильник, время: пятница : часы
  //   49 - Будильник, время: пятница : минуты
  //   50 - Будильник, время: суббота : часы
  //   51 - Будильник, время: суббота : минуты
  //   52 - Будильник, время: воскресенье : часы
  //   53 - Будильник, время: воскресенье : минуты
  //  54-63   - имя точки доступа    - 10 байт
  //  64-79   - пароль точки доступа - 16 байт
  //  80-103  - имя сети  WiFi       - 24 байта
  //  104-119 - пароль сети  WiFi    - 16 байт
  //  120-149 - имя NTP сервера      - 30 байт
  //  150 - 150+(Nэфф*3)   - скорость эффекта
  //  151 - 150+(Nэфф*3)+1 - специальный параметр эффекта
  //  152 - 150+(Nэфф*3)+2 - эффект в авторежиме: 1 - использовать; 0 - не использовать

  // Сначала инициализируем имя сети/точки доступа, пароли и имя NTP-сервера значениями по умолчанию.
  // Ниже, если EEPROM уже инициализирован - из него будут загружены актуальные значения
  strcpy(apName, DEFAULT_AP_NAME);
  strcpy(apPass, DEFAULT_AP_PASS);
  strcpy(ssid, NETWORK_SSID);
  strcpy(pass, NETWORK_PASS);
  strcpy(ntpServerName, DEFAULT_NTP_SERVER);    

  // Инициализировано ли EEPROM
  bool isInitialized = EEPROMread(0) == EEPROM_OK;  
    
  if (isInitialized) {    
    globalBrightness = getMaxBrightness();

    autoplayTime = getAutoplayTime();
    idleTime = getIdleTime();    

    useNtp = getUseNtp();
    timeZoneOffset = getTimeZone();

    SYNC_TIME_PERIOD = getNtpSyncTime();
    AUTOPLAY = getAutoplay();
    
    alarmWeekDay = getAlarmWeekDay();
    alarmEffect = getAlarmEffect();
    alarmDuration = getAlarmDuration();

    needTurnOffClock = getTurnOffClockOnLampOff();

    for (byte i=0; i<7; i++) {
      alarmHour[i] = getAlarmHour(i+1);
      alarmMinute[i] = getAlarmMinute(i+1);
    }
 
    for (byte i=0; i<MAX_EFFECT; i++) {
      effectScaleParam[i] = getScaleForEffect(i);
    }

    globalColor = getColorInt(CHSV(getEffectSpeed(MC_FILL_COLOR), effectScaleParam[MC_FILL_COLOR], 255));

    dawnDuration = getDawnDuration();
    useAlarmSound = getUseAlarmSound();    
    alarmSound = getAlarmSound();
    dawnSound = getDawnSound();
    maxAlarmVolume = getMaxAlarmVolume();

    useSoftAP = getUseSoftAP();
    getSoftAPName().toCharArray(apName, 10);        //  54-63   - имя точки доступа    ( 9 байт макс) + 1 байт '\0'
    getSoftAPPass().toCharArray(apPass, 17);        //  64-79   - пароль точки доступа (16 байт макс) + 1 байт '\0'
    getSsid().toCharArray(ssid, 25);                //  80-103  - имя сети  WiFi       (24 байта макс) + 1 байт '\0'
    getPass().toCharArray(pass, 17);                //  104-119 - пароль сети  WiFi    (16 байт макс) + 1 байт '\0'
    getNtpServer().toCharArray(ntpServerName, 31);  //  120-149 - имя NTP сервера      (30 байт макс) + 1 байт '\0'

    if (strlen(apName) == 0) strcpy(apName, DEFAULT_AP_NAME);
    if (strlen(apPass) == 0) strcpy(apPass, DEFAULT_AP_PASS);
    if (strlen(ntpServerName) == 0) strcpy(ntpServerName, DEFAULT_NTP_SERVER);

    AM1_hour = getAM1hour();
    AM1_minute = getAM1minute();
    AM1_effect_id = getAM1effect();
    AM2_hour = getAM2hour();
    AM2_minute = getAM2minute();
    AM2_effect_id = getAM2effect();

    loadStaticIP();
    
  } else {
    globalBrightness = BRIGHTNESS;

    autoplayTime = ((long)AUTOPLAY_PERIOD * 1000L);     // секунды -> миллисек
    idleTime = ((long)IDLE_TIME * 60L * 1000L);         // минуты -> миллисек

    useNtp = true;
    timeZoneOffset = 7;

    AUTOPLAY = true;
    SYNC_TIME_PERIOD = 60;

    alarmWeekDay = 0;
    dawnDuration = 20;
    alarmEffect = MC_DAWN_ALARM;
    useSoftAP = false;
    useAlarmSound = false;
    alarmDuration = 1;
    alarmSound = 1;
    dawnSound = 1;
    needTurnOffClock = false;
    maxAlarmVolume = 30;

    for (byte i=0; i<MAX_EFFECT; i++) {
      effectScaleParam[i] = 50;
    }

    AM1_hour = 0;
    AM1_minute = 0;
    AM1_effect_id = -5;
    AM2_hour = 0;
    AM2_minute = 0;
    AM2_effect_id = -5;    
  }

  idleTimer.setInterval(idleTime);  
  ntpSyncTimer.setInterval(1000 * 60 * SYNC_TIME_PERIOD);
  
  // После первой инициализации значений - сохранить их принудительно
  if (!isInitialized) {
    saveDefaults();
    saveSettings();
  }
}

void saveDefaults() {

  EEPROMwrite(1, globalBrightness);

  EEPROMwrite(2, AUTOPLAY ? 1 : 0);
  EEPROMwrite(3, autoplayTime / 1000L);
  EEPROMwrite(4, constrain(idleTime / 60L / 1000L, 0, 255));

  EEPROMwrite(5, useNtp ? 1 : 0);
  EEPROM_int_write(6, SYNC_TIME_PERIOD);
  EEPROMwrite(8, (byte)timeZoneOffset);
  EEPROMwrite(9, false);

  EEPROMwrite(10, IP_STA[0]);
  EEPROMwrite(11, IP_STA[1]);
  EEPROMwrite(12, IP_STA[2]);
  EEPROMwrite(13, IP_STA[3]);

  EEPROMwrite(14, 0);    // Использовать режим точки доступа: 0 - нет; 1 - да
  
  saveAlarmParams(alarmWeekDay,dawnDuration,alarmEffect,alarmDuration);
  for (byte i=0; i<7; i++) {
      setAlarmTime(i+1, alarmHour[i], alarmMinute[i]);
  }

  EEPROMwrite(33, AM1_hour);            // Режим 1 по времени - часы
  EEPROMwrite(34, AM1_minute);          // Режим 1 по времени - минуты
  EEPROMwrite(35, (byte)AM1_effect_id); // Режим 1 по времени - действие: -3 - выключено (не используется); -2 - выключить матрицу (черный экран); -1 - огонь, 0 - случайный, 1 и далее - эффект EFFECT_LIST
  EEPROMwrite(36, AM2_hour);            // Режим 2 по времени - часы
  EEPROMwrite(37, AM2_minute);          // Режим 2 по времени - минуты
  EEPROMwrite(38, (byte)AM2_effect_id); // Режим 2 по времени - действие: -3 - выключено (не используется); -2 - выключить матрицу (черный экран); -1 - огонь, 0 - случайный, 1 и далее - эффект EFFECT_LIST
  
  // Настройки по умолчанию для эффектов
  int addr = EFFECT_EEPROM;
  for (int i = 0; i < MAX_EFFECT; i++) {
    saveEffectParams(i, effectSpeed, 50, true);
  }
    
  strcpy(apName, DEFAULT_AP_NAME);
  strcpy(apPass, DEFAULT_AP_PASS);
  setSoftAPName(String(apName));
  setSoftAPPass(String(apPass));
  
  strcpy(ntpServerName, DEFAULT_NTP_SERVER);
  setNtpServer(String(ntpServerName));

  eepromModified = true;
}

void saveSettings() {

  saveSettingsTimer.reset();
  
  if (!eepromModified) return;
  
  // Поставить отметку, что EEPROM инициализировано параметрами эффектов
  EEPROMwrite(0, EEPROM_OK);
  
  EEPROM.commit();
  Serial.println(F("Настройки сохранены в EEPROM"));
  
  eepromModified = false;
}

void saveEffectParams(byte effect, int speed, byte value, boolean use) {
  const int addr = EFFECT_EEPROM;  
  EEPROMwrite(addr + effect*3, constrain(map(speed, D_EFFECT_SPEED_MIN, D_EFFECT_SPEED_MAX, 0, 255), 0, 255));        // Скорость эффекта
  EEPROMwrite(addr + effect*3 + 1, value);                       // Параметр эффекта  
  EEPROMwrite(addr + effect*3 + 2, use ? 1 : 0);                 // По умолчанию эффект доступен в демо-режиме  
  eepromModified = true;
}

void saveEffectSpeed(byte effect, int speed) {
  if (speed != getEffectSpeed(effect)) {
    const int addr = EFFECT_EEPROM;  
    EEPROMwrite(addr + effect*3, constrain(map(speed, D_EFFECT_SPEED_MIN, D_EFFECT_SPEED_MAX, 0, 255), 0, 255));        // Скорость эффекта
    eepromModified = true;
  }
}

byte getEffectSpeed(byte effect) {
  const int addr = EFFECT_EEPROM;
  return map(EEPROMread(addr + effect*3),0,255,D_EFFECT_SPEED_MIN,D_EFFECT_SPEED_MAX);
}

void saveEffectUsage(byte effect, boolean use) {
  if (use != getEffectUsage(effect)) {
    const int addr = EFFECT_EEPROM;  
    EEPROMwrite(addr + effect*3 + 2, use ? 1 : 0);             // По умолчанию оверлей часов для эффекта отключен  
    eepromModified = true;
  }
}

boolean getEffectUsage(byte effect) {
  const int addr = EFFECT_EEPROM;
  return EEPROMread(addr + effect*3 + 2) == 1;
}

void setScaleForEffect(byte effect, byte value) {
  if (value != getScaleForEffect(effect)) {
    const int addr = EFFECT_EEPROM;
    EEPROMwrite(addr + effect*3 + 1, value);
    eepromModified = true;
  }  
}

byte getScaleForEffect(byte effect) {
  const int addr = EFFECT_EEPROM;
  return EEPROMread(addr + effect*3 + 1);
}

byte getMaxBrightness() {
  return EEPROMread(1);
}

void saveMaxBrightness(byte brightness) {
  if (brightness != getMaxBrightness()) {
    EEPROMwrite(1, brightness);
    eepromModified = true;
  }
}

void saveAutoplay(boolean value) {
  if (value != getAutoplay()) {
    EEPROMwrite(2, value);
    eepromModified = true;
  }
}

bool getAutoplay() {
  return EEPROMread(2) == 1;
}

void saveAutoplayTime(long value) {
  if (value != getAutoplayTime()) {
    EEPROMwrite(3, constrain(value / 1000L, 0, 255));
    eepromModified = true;
  }
}

long getAutoplayTime() {
  long time = EEPROMread(3) * 1000L;  
  if (time == 0) time = ((long)AUTOPLAY_PERIOD * 1000);
  return time;
}

void saveIdleTime(long value) {
  if (value != getIdleTime()) {
    EEPROMwrite(4, constrain(value / 60L / 1000L, 0, 255));
    eepromModified = true;
  }
}

long getIdleTime() {
  long time = EEPROMread(4) * 60 * 1000L;  
  time = ((long)IDLE_TIME * 60L * 1000L);
  return time;
}

void saveUseNtp(boolean value) {
  if (value != getUseNtp()) {
    EEPROMwrite(5, value);
    eepromModified = true;
  }
}

bool getUseNtp() {
  return EEPROMread(5) == 1;
}

void saveNtpSyncTime(uint16_t value) {
  if (value != getNtpSyncTime()) {
    EEPROM_int_write(6, SYNC_TIME_PERIOD);
    eepromModified = true;
  }
}

uint16_t getNtpSyncTime() {
  uint16_t time = EEPROM_int_read(6);  
  if (time == 0) time = 60;
  return time;
}

void saveTimeZone(int8_t value) {
  if (value != getTimeZone()) {
    EEPROMwrite(8, (byte)value);
    eepromModified = true;
  }
}

int8_t getTimeZone() {
  return (int8_t)EEPROMread(8);
}

void saveAlarmParams(byte alarmWeekDay, byte dawnDuration, byte alarmEffect, byte alarmDuration) {  
  if (alarmWeekDay != getAlarmWeekDay()) {
    EEPROMwrite(20, alarmWeekDay);
    eepromModified = true;
  }
  if (dawnDuration != getDawnDuration()) {
    EEPROMwrite(21, dawnDuration);
    eepromModified = true;
  }
  if (alarmEffect != getAlarmEffect()) {
    EEPROMwrite(22, alarmEffect);
    eepromModified = true;
  }
  //   24 - Будильник, длительность звука будильника, минут
  if (alarmDuration != getAlarmDuration()) {
    EEPROMwrite(24, alarmDuration);
    eepromModified = true;
  }
}

byte getAlarmHour(byte day) { 
  return constrain(EEPROMread(40 + 2 * (day - 1)), 0, 23);
}

byte getAlarmMinute(byte day) { 
  return constrain(EEPROMread(40 + 2 * (day - 1) + 1), 0, 59);
}

void setAlarmTime(byte day, byte hour, byte minute) { 
  if (hour != getAlarmHour(day)) {
    EEPROMwrite(40 + 2 * (day - 1), constrain(hour, 0, 23));
    eepromModified = true;
  }
  if (hour != minute != getAlarmMinute(day)) {
    EEPROMwrite(40 + 2 * (day - 1) + 1, constrain(minute, 0, 59));
    eepromModified = true;
  }
}

byte getAlarmWeekDay() { 
  return EEPROMread(20);
}

byte getDawnDuration() { 
  return constrain(EEPROMread(21),1,59);
}

void saveAlarmSounds(boolean useSound, byte maxVolume, int8_t alarmSound, int8_t dawnSound) {
  //   23 - Будильник звук: вкл/выкл 1 - вкл; 0 -выкл
  //   25 - Будильник, мелодия будильника
  //   26 - Будильник, мелодия рассвета
  //   27 - Будильник, максимальная громкость
  if (alarmEffect != getAlarmEffect()) {
    EEPROMwrite(22, alarmEffect);
    eepromModified = true;
  }
  if (useSound != getUseAlarmSound()) {
    EEPROMwrite(23, useSound ? 1 : 0);
    eepromModified = true;
  }
  if (alarmSound != getAlarmSound()) {
    EEPROMwrite(25, (byte)alarmSound);
    eepromModified = true;
  }
  if (dawnSound != getDawnSound()) {
    EEPROMwrite(26, (byte)dawnSound);
  }
  if (maxVolume != getMaxAlarmVolume()) {
    EEPROMwrite(27, maxVolume);
    eepromModified = true;
  }
}

byte getAlarmEffect() { 
  return EEPROMread(22);
}

bool getUseAlarmSound() { 
  return EEPROMread(23) == 1;
}

byte getAlarmDuration() { 
  return constrain(EEPROMread(24),1,10);
}

byte getMaxAlarmVolume() { 
  return constrain(EEPROMread(27),0,30);
}

int8_t getAlarmSound() { 
  return constrain((int8_t)EEPROMread(25),-1,127);
}

int8_t getDawnSound() { 
  return constrain((int8_t)EEPROMread(26),-1,127);
}

bool getUseSoftAP() {
  return EEPROMread(14) == 1;
}

void setUseSoftAP(boolean use) {  
  if (use != getUseSoftAP()) {
    EEPROMwrite(14, use ? 1 : 0);
    eepromModified = true;
  }
}

String getSoftAPName() {
  return EEPROM_string_read(54, 10);
}

void setSoftAPName(String SoftAPName) {
  if (SoftAPName != getSoftAPName()) {
    EEPROM_string_write(54, SoftAPName);
    eepromModified = true;
  }
}

String getSoftAPPass() {
  return EEPROM_string_read(64, 16);
}

void setSoftAPPass(String SoftAPPass) {
  if (SoftAPPass != getSoftAPPass()) {
    EEPROM_string_write(64, SoftAPPass);
    eepromModified = true;
  }
}

String getSsid() {
  return EEPROM_string_read(80, 24);
}

void setSsid(String Ssid) {
  if (Ssid != getSsid()) {
    EEPROM_string_write(80, Ssid);
    eepromModified = true;
  }
}

String getPass() {
  return EEPROM_string_read(104, 16);
}

void setPass(String Pass) {
  if (Pass != getPass()) {
    EEPROM_string_write(104, Pass);
    eepromModified = true;
  }
}

String getNtpServer() {
  return EEPROM_string_read(120, 30);
}

void setNtpServer(String server) {
  if (server != getNtpServer()) {
    EEPROM_string_write(120, server);
    eepromModified = true;
  }
}

void setAM1params(byte hour, byte minute, int8_t effect) { 
  setAM1hour(hour);
  setAM1minute(minute);
  setAM1effect(effect);
}

byte getAM1hour() { 
  byte hour = EEPROMread(33);
  if (hour>23) hour = 0;
  return hour;
}

void setAM1hour(byte hour) {
  if (hour != getAM1hour()) {
    EEPROMwrite(33, hour);
    eepromModified = true;
  }
}

byte getAM1minute() {
  byte minute = EEPROMread(34);
  if (minute > 59) minute = 0;
  return minute;
}

void setAM1minute(byte minute) {
  if (minute != getAM1minute()) {
    EEPROMwrite(34, minute);
    eepromModified = true;
  }
}

int8_t getAM1effect() {
  return (int8_t)EEPROMread(35);
}

void setAM1effect(int8_t effect) {
  if (effect != getAM1minute()) {
    EEPROMwrite(35, (byte)effect);
    eepromModified = true;
  }
}

void setAM2params(byte hour, byte minute, int8_t effect) { 
  setAM2hour(hour);
  setAM2minute(minute);
  setAM2effect(effect);
}

byte getAM2hour() { 
  byte hour = EEPROMread(36);
  if (hour>23) hour = 0;
  return hour;
}

void setAM2hour(byte hour) {
  if (hour != getAM2hour()) {
    EEPROMwrite(36, hour);
    eepromModified = true;
  }
}

byte getAM2minute() {
  byte minute = EEPROMread(37);
  if (minute > 59) minute = 0;
  return minute;
}

void setAM2minute(byte minute) {
  if (minute != getAM2minute()) {
    EEPROMwrite(37, minute);
    eepromModified = true;
  }
}

int8_t getAM2effect() {
  return (int8_t)EEPROMread(38);
}

void setAM2effect(int8_t effect) {
  if (effect != getAM2minute()) {
    EEPROMwrite(38, (byte)effect);
    eepromModified = true;
  }
}

void loadStaticIP() {
  IP_STA[0] = EEPROMread(10);
  IP_STA[1] = EEPROMread(11);
  IP_STA[2] = EEPROMread(12);
  IP_STA[3] = EEPROMread(13);
}

void saveStaticIP(byte p1, byte p2, byte p3, byte p4) {
  if (IP_STA[0] != p1 || IP_STA[1] != p2 || IP_STA[2] != p3 || IP_STA[3] != p4) {
    IP_STA[0] = p1;
    IP_STA[1] = p2;
    IP_STA[2] = p3;
    IP_STA[3] = p4;
    EEPROMwrite(10, p1);
    EEPROMwrite(11, p2);
    EEPROMwrite(12, p3);
    EEPROMwrite(13, p4);
    eepromModified = true;  
  }
}

bool getTurnOffClockOnLampOff() {
  return EEPROMread(9) == 1;
}

void setTurnOffClockOnLampOff(bool flag) {
  if (flag != getTurnOffClockOnLampOff()) {
    EEPROMwrite(9, flag ? 1 : 0);
    eepromModified = true;
  }  
}

// ----------------------------------------------------------

byte EEPROMread(byte addr) {    
  return EEPROM.read(addr);
}

void EEPROMwrite(byte addr, byte value) {    
  EEPROM.write(addr, value);
}

// чтение uint16_t
uint16_t EEPROM_int_read(byte addr) {    
  byte raw[2];
  for (byte i = 0; i < 2; i++) raw[i] = EEPROMread(addr+i);
  uint16_t &num = (uint16_t&)raw;
  return num;
}

// запись uint16_t
void EEPROM_int_write(byte addr, uint16_t num) {
  byte raw[2];
  (uint16_t&)raw = num;
  for (byte i = 0; i < 2; i++) EEPROMwrite(addr+i, raw[i]);
}

// чтение стоки (макс 32 байта)
String EEPROM_string_read(byte addr, byte len) {
   if (len>32) len = 32;
   char buffer[len+1];
   memset(buffer,'\0',len+1);
   byte i = 0;
   while (i < len) {
     byte c = EEPROMread(addr+i);
     if (isAlphaNumeric(c) || isPunct(c))
        buffer[i++] = c;
     else
       break;
   }
   return String(buffer);
}

// запись строки (макс 32 байта)
void EEPROM_string_write(byte addr, String buffer) {
   uint16_t len = buffer.length();
   if (len>32) len = 32;
   byte i = 0;
   while (i < len) {
     EEPROMwrite(addr+i, buffer[i++]);
   }
   if (i < 32) EEPROMwrite(addr+i,0);
}

// ----------------------------------------------------------
