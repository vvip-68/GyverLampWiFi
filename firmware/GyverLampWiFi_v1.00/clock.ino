// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address) {
  Serial.print(F("Отправка NTP пакета на сервер "));
  Serial.println(ntpServerName);
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

void parseNTP() {
  Serial.println(F("Разбор пакета NTP"));
  ntp_t = 0; ntp_cnt = 0; init_time = true; refresh_time = false;
  unsigned long highWord = word(incomeBuffer[40], incomeBuffer[41]);
  unsigned long lowWord = word(incomeBuffer[42], incomeBuffer[43]);
  // combine the four bytes (two words) into a long integer
  // this is NTP time (seconds since Jan 1 1900):
  unsigned long secsSince1900 = highWord << 16 | lowWord;
  // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
  unsigned long seventyYears = 2208988800UL ;
  time_t t = secsSince1900 - seventyYears + (timeZoneOffset) * 3600;
  Serial.print(F("Секунд с 1970: "));
  Serial.println(t);
  setTime(t);
  calculateDawnTime();
}

void getNTP() {
  if (!wifi_connected) return;
  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  ntp_t = millis();
}

void clockTicker() {  

  if (isTurnedOff && needTurnOffClock) {
    disp.displayByte(_empty, _empty, _empty, _empty);
    disp.point(false);
    return;
  }
  
  if (isButtonHold || bCounter > 0) {
    // Удержание кнопки - изменение яркости + 2 сек после того как кнопка отпущена - 
    // отображать показание текущего значения яркости в процентах 0..99
    if (isButtonHold) bCounter = 4;
    if (!isButtonHold && bCounter > 0 && halfsecTimer.isReady()) bCounter--;
    byte prcBrightness = map(globalBrightness,0,255,0,99);
    byte m10 = getByteForDigit(prcBrightness / 10);
    byte m01 = getByteForDigit(prcBrightness % 10);
    disp.displayByte(_b, _r, m10, m01);
    disp.point(false);
  } else if (wifi_print_ip && halfsecTimer.isReady()) {
    // Четырехкратное нажатие кнопки запускает отображение по частям текущего IP лампы  
    dotFlag = !dotFlag;
    if (dotFlag) {
      int value = atoi(GetToken(WiFi.localIP().toString(), wifi_print_idx + 1, '.').c_str()); 
      disp.displayInt(value);
      disp.point(false);
      wifi_print_idx++;
      if (wifi_print_idx>3) {
        wifi_print_idx = 0; 
        wifi_print_ip = false;
      }
    }
  } else {
    // Отображение часов - разделительное двоеточие...
    bool halfSec = halfsecTimer.isReady();
    if (halfSec) {
      dotFlag = !dotFlag;
      disp.point(dotFlag);
    }
    // Если время еще не получено - отображать прочерки
    if (!init_time) {
      if (halfSec) disp.displayByte(_dash, _dash, _dash, _dash);
    } else if (!isAlarmStopped && (isPlayAlarmSound || isAlarming)) {
      // Сработал будильник (звук) - плавное мерцание текущего времени      
      if (halfSec) disp.displayClock(hour(),minute());
      if (millis() - fade_time > 50) {
        fade_time = millis();
        disp.brightness(aCounter);
        if (aDirection) aCounter++; else aCounter--;
        if (aCounter > 7) {
          aDirection = false;
          aCounter = 7;
        }
        if (aCounter == 0) {
          aDirection = true;
        }
      }
    } else {
      // Время получено - отображать часы:минуты
      if (halfSec) disp.displayClock(hour(),minute());
    }
  }
}

// расчет времени начала рассвета
void calculateDawnTime() {
  byte alrmHour;
  byte alrmMinute;
  
  dawnWeekDay = 0;
  if (!init_time || alarmWeekDay == 0) return;       // Время инициализировано? Хотя бы на один день будильник включен?

  int8_t alrmWeekDay = weekday()-1;                  // day of the week, Sunday is day 0   
  if (alrmWeekDay == 0) alrmWeekDay = 7;             // Sunday is day 7, Monday is day 1;

  // Текущее время и день недели
  byte h = hour();
  byte m = minute();
  byte w = weekday()-1;
  if (w == 0) w = 7;
  
  byte cnt = 0;
  while (cnt < 2) {
    cnt++;
    while ((alarmWeekDay & (1 << (alrmWeekDay - 1))) == 0) {
      alrmWeekDay++;
      if (alrmWeekDay == 8) alrmWeekDay = 1;
    }
      
    alrmHour = alarmHour[alrmWeekDay-1];
    alrmMinute = alarmMinute[alrmWeekDay-1];
  
    // "Сегодня" время будильника уже прошло? 
    if (alrmWeekDay == w && (h * 60L + w > alrmHour * 60L + alrmMinute)) {
      alrmWeekDay++;
    }
  }
  
  // расчёт времени рассвета
  if (alrmMinute > dawnDuration) {                  // если минут во времени будильника больше продолжительности рассвета
    dawnHour = alrmHour;                            // час рассвета равен часу будильника
    dawnMinute = alrmMinute - dawnDuration;         // минуты рассвета = минуты будильника - продолж. рассвета
    dawnWeekDay = alrmWeekDay;
  } else {                                          // если минут во времени будильника меньше продолжительности рассвета
    dawnWeekDay = alrmWeekDay;
    dawnHour = alrmHour - 1;                        // значит рассвет будет часом раньше
    if (dawnHour < 0) {
      dawnHour = 23;     
      dawnWeekDay--;
      if (dawnWeekDay == 0) dawnWeekDay = 7;                           
    }
    dawnMinute = 60 - (dawnDuration - alrmMinute);  // находим минуту рассвета в новом часе
  }

  Serial.print(String(F("Следующий рассвет в "))+String(dawnHour)+ F(":") + String(dawnMinute));
  switch(dawnWeekDay) {
    case 1: Serial.println(F(", понедельник")); break;
    case 2: Serial.println(F(", вторник")); break;
    case 3: Serial.println(F(", среда")); break;
    case 4: Serial.println(F(", четверг")); break;
    case 5: Serial.println(F(", пятница")); break;
    case 6: Serial.println(F(", суббота")); break;
    case 7: Serial.println(F(", воскресенье")); break;
    default: Serial.println(); break;
  }  
}

// Проверка времени срабатывания будильника
void checkAlarmTime() {

  byte h = hour();
  byte m = minute();
  byte w = weekday()-1;
  if (w == 0) w = 7;

  // Будильник включен?
  if (init_time && dawnWeekDay > 0) {

    // Время срабатывания будильника после завершения рассвета
    byte alrmWeekDay = dawnWeekDay;
    byte alrmHour = dawnHour;
    byte alrmMinute = dawnMinute + dawnDuration;
    if (alrmMinute >= 60) {
      alrmMinute = 60 - alrmMinute;
      alrmHour++;
    }
    if (alrmHour > 23) {
      alrmHour = 24 - alrmHour;
      alrmWeekDay++;
    }
    if (alrmWeekDay > 7) alrmWeekDay = 1;

    // Текущий день недели совпадает с вычисленным днём недели рассвета?
    if (w == dawnWeekDay) {
       // Часы / минуты начала рассвета наступили? Еще не запущен рассвет? Еще не остановлен пользователем?
       if (!isAlarming && !isAlarmStopped && ((w * 1000L + h * 60L + m) >= (dawnWeekDay * 1000L + dawnHour * 60L + dawnMinute)) && ((w * 1000L + h * 60L + m) < (alrmWeekDay * 1000L + alrmHour * 60L + alrmMinute))) {
         // Сохранить параметры текущего режима для восстановления после завершения работы будильника
         saveSpecialMode = specialMode;
         saveSpecialModeId = specialModeId;
         saveMode = thisMode;
         // Включить будильник
         specialMode = false;
         specialModeId = -1;
         isAlarming = true;
         isAlarmStopped = false;
         loadingFlag = true;         
         thisMode = MC_DAWN_ALARM;
         // Реальная продолжительность рассвета
         realDawnDuration = (alrmHour * 60L + alrmMinute) - (dawnHour * 60L + dawnMinute);
         if (realDawnDuration > dawnDuration) realDawnDuration = dawnDuration;
         // Отключмить таймер автоперехода в демо-режим
         idleTimer.setInterval(4294967295);
         if (useAlarmSound) PlayDawnSound();
         sendPageParams(95);  // Параметры, статуса IsAlarming (AL:1), чтобы изменить в смартфоне отображение активности будильника
         Serial.println(String(F("Рассвет ВКЛ в "))+String(h)+ ":" + String(m));
       }
    }
    
    delay(0); // Для предотвращения ESP8266 Watchdog Timer
    
    // При наступлении времени срабатывания будильника, если он еще не выключен пользователем - запустить режим часов и звук будильника
    if (alrmWeekDay == w && alrmHour == h && alrmMinute == m && isAlarming) {
      Serial.println(String(F("Рассвет Авто-ВЫКЛ в "))+String(h)+ ":" + String(m));
      isAlarming = false;
      isAlarmStopped = false;
      isPlayAlarmSound = true;
      setSpecialMode(1);      
      alarmSoundTimer.setInterval(alarmDuration * 60000L);
      alarmSoundTimer.reset();
      // Играть звук будильника
      // Если звук будильника не используется - просто запустить таймер.
      // До окончания таймера индикатор TM1637 будет мигать, лампа гореть ярко белым.
      if (useAlarmSound) {
        PlayAlarmSound();
      }
      sendPageParams(95);  // Параметры, статуса IsAlarming (AL:1), чтобы изменить в смартфоне отображение активности будильника
    }

    delay(0); // Для предотвращения ESP8266 Watchdog Timer

    // Если рассвет начинался и остановлен пользователем и время начала рассвета уже прошло - сбросить флаги, подготовив их к следующему циклу
    if (isAlarmStopped && ((w * 1000L + h * 60L + m) > (alrmWeekDay * 1000L + alrmHour * 60L + alrmMinute + alarmDuration))) {
      isAlarming = false;
      isAlarmStopped = false;
      StopSound(0);
    }
  }
  
  // Подошло время отключения будильника - выключить, вернуться в предыдущий режим
  if (alarmSoundTimer.isReady()) {

    // Во время работы будильника индикатор плавно мерцает.
    // После завершения работы - восстановить яркость индикатора
    disp.brightness (7);

    Serial.println(String(F("Будильник Авто-ВЫКЛ в "))+String(h)+ ":" + String(m));
    
    alarmSoundTimer.setInterval(4294967295);
    isPlayAlarmSound = false;
    StopSound(1000);   

    resetModes();  
    
    BTcontrol = false;
    AUTOPLAY = true;

    if (saveSpecialMode){
       setSpecialMode(saveSpecialModeId);
    } else {
       setEffect(saveMode);
    }

    sendPageParams(95);  // Параметры, статуса IsAlarming (AL:1), чтобы изменить в смартфоне отображение активности будильника
  }

  delay(0); // Для предотвращения ESP8266 Watchdog Timer

  // Плавное изменение громкости будильника
  if (fadeSoundTimer.isReady()) {
    if (fadeSoundDirection > 0) {
      // увеличение громкости
      dfPlayer.volumeUp();
      fadeSoundStepCounter--;
      if (fadeSoundStepCounter <= 0) {
        fadeSoundDirection = 0;
        fadeSoundTimer.setInterval(4294967295);
      }
    } else if (fadeSoundDirection < 0) {
      // Уменьшение громкости
      dfPlayer.volumeDown();
      fadeSoundStepCounter--;
      if (fadeSoundStepCounter <= 0) {
        isPlayAlarmSound = false;
        fadeSoundDirection = 0;
        fadeSoundTimer.setInterval(4294967295);
        StopSound(0);
      }
    }
  }
  
  delay(0); // Для предотвращения ESP8266 Watchdog Timer    
}

void stopAlarm() {
  
  if ((isAlarming || isPlayAlarmSound) && !isAlarmStopped) {
    Serial.println(String(F("Рассвет ВЫКЛ в ")) + String(hour())+ ":" + String(minute()));
    isAlarming = false;
    isAlarmStopped = true;
    isPlayAlarmSound = false;
    cmd95 = "";

    alarmSoundTimer.setInterval(4294967295);

    // Во время работы будильника индикатор плавно мерцает.
    // После завершения работы - восстановить яркость индикатора
    disp.brightness (7);
    StopSound(1000);

    resetModes();  

    BTcontrol = false;
    AUTOPLAY = false;

    if (saveSpecialMode){
       setSpecialMode(saveSpecialModeId);
    } else {
       setEffect(saveMode);
    }
    
    // setRandomMode();

    delay(0);    
    sendPageParams(95);  // Параметры, статуса IsAlarming (AL:1), чтобы изменить в смартфоне отображение активности будильника
  }
}

// Проверка необходимости включения режима 1 по установленному времени
// -2 - не используется; -1 - выключить матрицу; 0 - включить случайный с автосменой; 1 - номер режима из спписка EFFECT_LIST
void checkAutoMode1Time() {
  if (AM1_effect_id == -2 || !init_time) return;
  
  hrs = hour();
  mins = minute();

  // Режим по времени включен (enable) и настало врема активации режима - активировать
  if (!AM1_running && AM1_hour == hrs && AM1_minute == mins) {
    AM1_running = true;
    SetAutoMode(1);
  }

  // Режим активирован и время срабатывания режима прошло - сбросить флаг для подготовки к следующему циклу
  if (AM1_running && (AM1_hour != hrs || AM1_minute != mins)) {
    AM1_running = false;
  }
}

// Проверка необходимости включения режима 2 по установленному времени
// -2 - не используется; -1 - выключить матрицу; 0 - включить случайный с автосменой; 1 - номер режима из спписка EFFECT_LIST
void checkAutoMode2Time() {

  // Действие отличается от "Нет действия" и время установлено?
  if (AM2_effect_id == -2 || !init_time) return;

  // Если сработал будильник - рассвет - режим не переключать - остаемся в режими обработки будильника
  if ((isAlarming || isPlayAlarmSound) && !isAlarmStopped) return;

  hrs = hour();
  mins = minute();

  // Режим по времени включен (enable) и настало врема активации режима - активировать
  if (!AM2_running && AM2_hour == hrs && AM2_minute == mins) {
    AM2_running = true;
    SetAutoMode(2);
  }

  // Режим активирован и время срабатывания режима прошло - сбросить флаг для подготовки к следующему циклу
  if (AM2_running && (AM2_hour != hrs || AM2_minute != mins)) {
    AM2_running = false;
  }
}

// Выполнение включения режима 1 или 2 (amode) по установленному времени
// -2 - не используется; -1 - выключить матрицу; 0 - включить случайный с автосменой; 1 - номер режима из спписка EFFECT_LIST
void SetAutoMode(byte amode) {
  
  Serial.print(F("Авторежим "));
  Serial.print(amode);
  Serial.print(F(" ["));
  Serial.print(amode == 1 ? AM1_hour : AM2_hour);
  Serial.print(":");
  Serial.print(amode == 1 ? AM1_minute : AM2_minute);
  Serial.print(F("] - "));

  int8_t ef = (amode == 1 ? AM1_effect_id : AM2_effect_id);

  //ef: -2 - нет действия; 
  //    -1 - выключить матрицу (черный экран); 
  //     0 - случайный,
  //     1 и далее - эффект из EFFECT_LIST по списку

  // включить указанный режим
  if (ef == -1) {

    // Выключить матрицу (черный экран)
    Serial.print(F("выключение матрицы"));
    setSpecialMode(0);
    
  } else {
    Serial.print(F("включение режима "));    
    // Если режим включения == 0 - случайный режим и автосмена по кругу
    AUTOPLAY = ef == 0;
    if (!AUTOPLAY) {
      // Таймер возврата в авторежим отключен    
      idleTimer.setInterval(4294967295);
      idleTimer.reset();
    }
    
    resetModes();  

    String s_tmp = String(EFFECT_LIST);
    
    if (ef == 0) {
      // "Случайный" режим и далее автосмена
      Serial.print(F(" демонcтрации эффектов:"));
      uint32_t cnt = CountTokens(s_tmp, ','); 
      ef = random(0, cnt - 1); 
    } else {
      ef -= 1; // Приведение номера эффекта (номер с 1) к индексу в массиве ALARM_LIST (индекс c 0)
    }

    s_tmp = GetToken(s_tmp, ef+1, ',');
    Serial.print(F(" эффект "));
    Serial.print("'" + s_tmp + "'");

    // Включить указанный режим из списка доступных эффектов без дальнейшей смены
    // Значение ef может быть 0..N-1 - указанный режим из списка EFFECT_LIST (приведенное к индексу с 0)      
    setEffect(ef);
  }
  
  Serial.println();
}

byte getByteForDigit(byte digit) {
  switch (digit) {
    case 0: return _0;
    case 1: return _1;
    case 2: return _2;
    case 3: return _3;
    case 4: return _4;
    case 5: return _5;
    case 6: return _6;
    case 7: return _7;
    case 8: return _8;
    case 9: return _9;
    default: return _empty;
  }
}
