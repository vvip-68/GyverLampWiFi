// ************************ НАСТРОЙКИ ************************

/*
   Эффекты:
    sparklesRoutine();    // случайные цветные гаснущие вспышки
    snowRoutine();        // снег
    matrixRoutine();      // "матрица"
    starfallRoutine();    // звездопад (кометы)
    ballRoutine();        // квадратик
    ballsRoutine();       // шарики
    rainbowRoutine();     // радуга во всю матрицу горизонтальная
    rainbowDiagonalRoutine();   // радуга во всю матрицу диагональная
    fireRoutine();        // огонь

  Крутые эффекты "шума":
    madnessNoise();       // цветной шум во всю матрицу
    cloudNoise();         // облака
    lavaNoise();          // лава
    plasmaNoise();        // плазма
    rainbowNoise();       // радужные переливы
    rainbowStripeNoise(); // полосатые радужные переливы
    zebraNoise();         // зебра
    forestNoise();        // шумящий лес
    oceanNoise();         // морская вода
*/

// ************************* СВОЙ СПИСОК РЕЖИМОВ ************************
// список можно менять, соблюдая его структуру. Можно удалять и добавлять эффекты, ставить их в
// любой последовательности или вообще оставить ОДИН. Удалив остальные case и break. Cтруктура оч простая:
// case <номер>: <эффект>;
//  break;

void customRoutine(byte aMode) {

  if (effectTimer.isReady() || aMode > MAX_EFFECT) {  
    switch (aMode) {    
      case MC_NOISE_MADNESS:       madnessNoise(); break;
      case MC_NOISE_CLOUD:         cloudNoise(); break;
      case MC_NOISE_LAVA:          lavaNoise(); break;
      case MC_NOISE_PLASMA:        plasmaNoise(); break;
      case MC_NOISE_RAINBOW:       rainbowNoise(); break;
      case MC_NOISE_RAINBOW_STRIP: rainbowStripeNoise(); break;
      case MC_NOISE_ZEBRA:         zebraNoise(); break;
      case MC_NOISE_FOREST:        forestNoise(); break;
      case MC_NOISE_OCEAN:         oceanNoise(); break;
      case MC_SNOW:                snowRoutine(); break;
      case MC_SPARKLES:            sparklesRoutine(); break;
      case MC_MATRIX:              matrixRoutine(); break;
      case MC_STARFALL:            starfallRoutine(); break;
      case MC_BALL:                ballRoutine(); break;
      case MC_BALLS:               ballsRoutine(); break;
      case MC_RAINBOW_HORIZ:       rainbowHorizontal(); break;
      case MC_RAINBOW_VERT:        rainbowVertical(); break;
      case MC_RAINBOW_DIAG:        rainbowDiagonalRoutine(); break;
      case MC_FIRE:                fireRoutine(); break;
      case MC_FILL_COLOR:          fillColorProcedure(); break;
      case MC_COLORS:              colorsRoutine(); break;
      case MC_LIGHTERS:            lightersRoutine(); break;
      case MC_DAWN_ALARM:          dawnProcedure(); break;
  
      // Специальные режимы - доступные только для вызова из эффекта рассвета - dawnProcedure()
      case MC_DAWN_ALARM_SPIRAL:   dawnLampSpiral(); break;
      case MC_DAWN_ALARM_SQUARE:   dawnLampSquare(); break;
    }
    FastLED.show();  
  }
}

// ********************* ОСНОВНОЙ ЦИКЛ РЕЖИМОВ *******************

static void nextMode() {
#if (SMOOTH_CHANGE == 1)
  fadeMode = 0;
  modeDir = true;
#else
  nextModeHandler();
#endif
}

static void prevMode() {
#if (SMOOTH_CHANGE == 1)
  fadeMode = 0;
  modeDir = false;
#else
  prevModeHandler();
#endif
}

void nextModeHandler() {
  byte aCnt = 0;
  byte curMode = thisMode;

  while (aCnt < MAX_EFFECT) {
    // Берем следующий режим по циклу режимов
    aCnt++; thisMode++;  
    if (thisMode >= MAX_EFFECT) thisMode = 0;

    // Если новый режим отмечен флагом "использовать" - используем его, иначе берем следующий (и проверяем его)
    if (getEffectUsage(thisMode)) break;
    
    // Если перебрали все и ни у адного нет флага "использовать" - не обращаем внимание на флаг, используем следующий
    if (aCnt >= MAX_EFFECT) {
      thisMode = curMode++;
      if (thisMode >= MAX_EFFECT) thisMode = 0;
      break;
    }    
  }
  
  loadingFlag = true;
  autoplayTimer = millis();
  setTimersForMode(thisMode);
  
  FastLED.clear();
  FastLED.show();
  FastLED.setBrightness(globalBrightness);
}

void prevModeHandler() {
  byte aCnt = 0;
  byte curMode = thisMode;

  while (aCnt < MAX_EFFECT) {
    // Берем предыдущий режим по циклу режимов
    aCnt++; thisMode--;  
    if (thisMode < 0) thisMode = MAX_EFFECT - 1;

    // Если новый режим отмечен флагом "использовать" - используем его, иначе берем следующий (и проверяем его)
    if (getEffectUsage(thisMode)) break;
    
    // Если перебрали все и ни у адного нет флага "использовать" - не обращаем внимание на флаг, используем следующий
    if (aCnt >= MAX_EFFECT) {
      thisMode = curMode--;
      if (thisMode < 0) thisMode = MAX_EFFECT - 1;
      break;
    }    
  }
  
  loadingFlag = true;
  autoplayTimer = millis();
  setTimersForMode(thisMode);
  
  FastLED.clear();
  FastLED.show();
  FastLED.setBrightness(globalBrightness);
}

void setTimersForMode(byte aMode) {
  effectSpeed = getEffectSpeed(aMode);
  effectTimer.setInterval(effectSpeed);
}

int fadeBrightness;
int fadeStepCount = 10;     // За сколько шагов убирать/добавлять яркость при смене режимов
int fadeStepValue = 5;      // Шаг убавления яркости

#if (SMOOTH_CHANGE == 1)
void modeFader() {
  if (fadeMode == 0) {
    fadeMode = 1;
    fadeStepValue = fadeBrightness / fadeStepCount;
    if (fadeStepValue < 1) fadeStepValue = 1;
  } else if (fadeMode == 1) {
    if (changeTimer.isReady()) {
      fadeBrightness -= fadeStepValue;
      if (fadeBrightness < 0) {
        fadeBrightness = 0;
        fadeMode = 2;
      }
      FastLED.setBrightness(fadeBrightness);
    }
  } else if (fadeMode == 2) {
    fadeMode = 3;
    if (modeDir) nextModeHandler();
    else prevModeHandler();
  } else if (fadeMode == 3) {
    if (changeTimer.isReady()) {
      fadeBrightness += fadeStepValue;
      if (fadeBrightness > globalBrightness) {
        fadeBrightness = globalBrightness;
        fadeMode = 4;
      }
      FastLED.setBrightness(fadeBrightness);
    }
  }
}
#endif

void checkIdleState() {

#if (SMOOTH_CHANGE == 1)
  modeFader();
#endif
  
  if (idleState) {
    if (millis() - autoplayTimer > autoplayTime && AUTOPLAY) {    // таймер смены режима
      autoplayTimer = millis();
      nextMode();
    }
  } else {
    if (idleTimer.isReady()) {      // таймер холостого режима. Если время наступило - включить автосмену режимов 
      idleState = true;                                     
      autoplayTimer = millis();
      loadingFlag = true;
      AUTOPLAY = true;      
      FastLED.clear();
      FastLED.show();
    }
  }  
}

void setRandomMode() {
    String s_tmp = String(EFFECT_LIST);    
    uint32_t cnt = CountTokens(s_tmp, ','); 
    byte ef = random(0, cnt - 1); 
    setEffect(ef);
}
