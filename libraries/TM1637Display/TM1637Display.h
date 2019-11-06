//  Author: avishorp@gmail.com
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef __TM1637DISPLAY__
#define __TM1637DISPLAY__

#include <inttypes.h>

#define SEG_A   0b00000001
#define SEG_B   0b00000010
#define SEG_C   0b00000100
#define SEG_D   0b00001000
#define SEG_E   0b00010000
#define SEG_F   0b00100000
#define SEG_G   0b01000000

#define DEFAULT_BIT_DELAY  100

class TM1637Display {

public:
  //! Initialize a TM1637Display object, setting the clock and
  //! data pins.
  //! Инициализация объекта TM1637Display, установка тактовых импульсов и 
  //! контактов данных.
  //!
  //! @param pinClk - The number of the digital pin connected to the clock pin of the module
  //!                 Номер цифрового контакта, подключенного к контакту синхронизации модуля
  //! @param pinDIO - The number of the digital pin connected to the DIO pin of the module
  //!                 Номер цифрового контакта, подключенного к контакту DIO модуля
  //! @param bitDelay - The delay, in microseconds, between bit transition on the serial
  //!                   bus connected to the display
  //!                   Задержка в микросекундах между битовым переходом на последовательной шине,
  //!                   подключенной к дисплею
  TM1637Display(uint8_t pinClk, uint8_t pinDIO, unsigned int bitDelay = DEFAULT_BIT_DELAY);
  void display(uint8_t DispData[]);
  void display(uint8_t bit0, uint8_t bit1, uint8_t bit2, uint8_t bit3);
  void displayClock(uint8_t hrs, uint8_t mins);
  void displayInt(int value);
  // выводит часы и минуты

  //! Sets the brightness of the display.
  //! Устанавливает яркость дисплея.
  //!
  //! The setting takes effect when a command is given to change the data being
  //! displayed.
  //! Этот параметр вступает в силу после ввода команды для изменения 
  //! отображаемых данных.
  //!
  //! @param brightness A number from 0 (lowes brightness) to 7 (highest brightness)
  //!                   Число от 0 (понижает яркость) до 7 (наибольшая яркость)
  //! @param on Turn display on or off
  //!           Включение или отключение дисплея
  void setBrightness(uint8_t brightness, bool on = true);
  //! Вкл / выкл точку (POINT_ON / POINT_OFF)
  void point(bool PointFlag);

  //! Display arbitrary data on the module
  //! Отображение произвольных данных на модуле
  //!
  //! This function receives raw segment values as input and displays them. The segment data
  //! is given as a byte array, each byte corresponding to a single digit. Within each byte,
  //! bit 0 is segment A, bit 1 is segment B etc.
  //! The function may either set the entire display or any desirable part on its own. The first
  //! digit is given by the @ref pos argument with 0 being the leftmost digit. The @ref length
  //! argument is the number of digits to be set. Other digits are not affected.
  //! Эта функция получает исходные значения сегментов в качестве входных данных и отображает их. Сегментные данные 
  //! приводятся в виде массива байтов, каждый байт соответствует одной цифре. Внутри каждого байта,
  //! бит 0 является сегментом A, бит 1 является сегментом B и т.д.
  //! Функция может устанавливать либо весь дисплей, либо любую желательную часть самостоятельно. Первая 
  //! цифра задается аргументом @ ref pos, где 0 является крайней левой цифрой. Аргумент @ ref length 
  //! - это количество цифр, которое необходимо задать. Другие цифры не затрагиваются.
  //!
  //! @param segments An array of size @ref length containing the raw segment values
  //!                 Массив размера @ ref length, содержащий исходные значения сегмента
  //! @param length The number of digits to be modified
  //!               Количество изменяемых цифр
  //! @param pos The position from which to start the modification (0 - leftmost, 3 - rightmost)
  //!            Положение начала изменения (0 - крайний левый, 3 - крайний правый)
  void setSegments(const uint8_t segments[], uint8_t length = 4, uint8_t pos = 0);
  //! Выводит байт вида 0xe6 и буквы-константы вида _a , _b .... массивом
  void displayByte(uint8_t DispData[]);
  void displayByte(uint8_t bit0, uint8_t bit1, uint8_t bit2, uint8_t bit3);
  //! Clear the display
  //! Очистка дисплея
  void clear();

  //! Display a decimal number
  //! Отображение десятичного числа
  //!
  //! Dispaly the given argument as a decimal number.
  //! Распределить данный аргумент как десятичное число.
  //!
  //! @param num The number to be shown
  //!            Отображаемый номер
  //! @param leading_zero When true, leading zeros are displayed. Otherwise unnecessary digits are
  //!        blank. NOTE: leading zero is not supported with negative numbers.
  //!                     Если значение равно true, отображаются первые нули. В противном случае ненужные цифры
  //!        пусты. ПРИМЕЧАНИЕ: ведущий ноль не поддерживается отрицательными числами.
  //! @param length The number of digits to set. The user must ensure that the number to be shown
  //!        fits to the number of digits requested (for example, if two digits are to be displayed,
  //!        the number must be between 0 to 99)
  //!               Количество устанавливаемых цифр. Пользователь должен убедиться, что отображаемый
  //!        номер соответствует запрошенному количеству цифр (например, при отображении двух цифр
  //!        номер должен находиться в диапазоне от 0 до 99).
  //! @param pos The position of the most significant digit (0 - leftmost, 3 - rightmost)
  //!            Позиция наиболее значимой цифры (0 - крайняя левая, 3 - крайняя правая)
  void showNumberDec(int num, bool leading_zero = false, uint8_t length = 4, uint8_t pos = 0);

  //! Display a decimal number, with dot control
  //! Отображение десятичного числа с управлением точками
  //!
  //! Dispaly the given argument as a decimal number. The dots between the digits (or colon)
  //! can be individually controlled.
  //! Распределить данный аргумент как десятичное число. Точки между цифрами (или двоеточием)
  //! могут управляться индивидуально.
  //!
  //! @param num The number to be shown
  //!            Отображаемый номер
  //! @param dots Dot/Colon enable. The argument is a bitmask, with each bit corresponding to a dot
  //!        between the digits (or colon mark, as implemented by each module). i.e.
  //!        For displays with dots between each digit:
  //!             Включение точки/двоеточия. Аргумент представляет собой битовую маску, причем каждый бит соответствует точке
  //!        между цифрами (или метке двоеточия, реализованной каждым модулем). Например,
  //!        для дисплеев с точками между каждой цифрой:
  //!        * 0.000 (0b10000000)
  //!        * 00.00 (0b01000000)
  //!        * 000.0 (0b00100000)
  //!        * 0.0.0.0 (0b11100000)
  //!        For displays with just a colon:
  //!        Для дисплеев с двоеточием:
  //!        * 00:00 (0b01000000)
  //!        For displays with dots and colons colon:
  //!        Для дисплеев с точками и двоеточием:
  //!        * 0.0:0.0 (0b11100000)
  //! @param leading_zero When true, leading zeros are displayed. Otherwise unnecessary digits are
  //!        blank. NOTE: leading zero is not supported with negative numbers.
  //!                     Если значение равно true, отображаются первые нули. В противном случае ненужные цифры
  //!        пусты. ПРИМЕЧАНИЕ: ведущий ноль не поддерживается отрицательными числами.
  //! @param length The number of digits to set. The user must ensure that the number to be shown
  //!        fits to the number of digits requested (for example, if two digits are to be displayed,
  //!        the number must be between 0 to 99)
  //!               Количество устанавливаемых цифр. Пользователь должен убедиться, что отображаемый номер 
  //!        соответствует запрошенному количеству цифр (например, при отображении двух цифр номер 
  //!        должен находиться в диапазоне от 0 до 99).
  //! @param pos The position of the most significant digit (0 - leftmost, 3 - rightmost)
  //!            Позиция наиболее значимой цифры (0 - крайняя левая, 3 - крайняя правая)
  void showNumberDecEx(int num, uint8_t dots = 0, bool leading_zero = false, uint8_t length = 4, uint8_t pos = 0);

  //! Display a hexadecimal number, with dot control
  //! Отображение шестнадцатеричного числа с контролем точек
  //!
  //! Dispaly the given argument as a hexadecimal number. The dots between the digits (or colon)
  //! can be individually controlled.
  //! Распределите данный аргумент как шестнадцатеричное число. Точки между цифрами (или двоеточием)
  //! Может управляться индивидуально.
  //!
  //! @param num The number to be shown
  //!            Отображаемый номер
  //! @param dots Dot/Colon enable. The argument is a bitmask, with each bit corresponding to a dot
  //!        between the digits (or colon mark, as implemented by each module). i.e.
  //!        For displays with dots between each digit:
  //!             Включение точки/двоеточия. Аргумент является битовой маской, причем каждый бит соответствует точке
  //!             Между цифрами (или меткой двоеточия, реализованной каждым модулем). т.е.
  //!             Для дисплеев с точками между каждой цифрой:
  //!        * 0.000 (0b10000000)
  //!        * 00.00 (0b01000000)
  //!        * 000.0 (0b00100000)
  //!        * 0.0.0.0 (0b11100000)
  //!        For displays with just a colon:
  //!        Для дисплеев с двоеточием
  //!        * 00:00 (0b01000000)
  //!        For displays with dots and colons colon:
  //!        Для дисплеев с точками и двоеточием:
  //!        * 0.0:0.0 (0b11100000)
  //! @param leading_zero When true, leading zeros are displayed. Otherwise unnecessary digits are
  //!                     Если значение равно true, отображаются первые нули. В противном случае ненужные цифры:
  //!        blank
  //!

  //! @param length The number of digits to set. The user must ensure that the number to be shown
  //!        fits to the number of digits requested (for example, if two digits are to be displayed,
  //!        the number must be between 0 to 99)
  //!               Количество устанавливаемых цифр. Пользователь должен убедиться, что отображаемый номер
  //!        Соответствует количеству запрошенных цифр (например, если требуется отобразить две цифры,
  //!        Число должно быть от 0 до 99)
  //!
  //! @param pos The position of the most significant digit (0 - leftmost, 3 - rightmost)
  //!            Позиция наиболее значимой цифры (0 - крайняя левая, 3 - крайняя правая)
  void showNumberHexEx(uint16_t num, uint8_t dots = 0, bool leading_zero = false, uint8_t length = 4, uint8_t pos = 0);

  //! Translate a single digit into 7 segment code
  //! Перевести одну цифру в 7 сегментный код
  //!
  //! The method accepts a number between 0 - 15 and converts it to the
  //! code required to display the number on a 7 segment display.
  //! Numbers between 10-15 are converted to hexadecimal digits (A-F)
  //! Метод принимает число от 0 до 15 и преобразует его в
  //! код, необходимый для отображения номера на дисплее сегмента 7.
  //! Числа между 10-15 преобразуются в шестнадцатеричные цифры (A-F)
  //!
  //! @param digit A number between 0 to 15
  //! Число от 0 до 15
  //! @return A code representing the 7 segment image of the digit (LSB - segment A;
  //!         bit 6 - segment G; bit 7 - always zero)
  //!         Возвращает Код, представляющий изображение 7 сегмента цифры (LSB - сегмент A;
  //!         Бит 6 - сегмент G; Бит 7 - всегда ноль)
  uint8_t encodeDigit(uint8_t digit);
  void showDots(uint8_t dots, uint8_t* digits);
  void clearDots(uint8_t dots, uint8_t* digits);

protected:
   void bitDelay();

   void start();

   void stop();

   bool writeByte(uint8_t b);

//   void showDots(uint8_t dots, uint8_t* digits);
   
   void showNumberBaseEx(int8_t base, uint16_t num, uint8_t dots = 0, bool leading_zero = false, uint8_t length = 4, uint8_t pos = 0);


private:
  void update();
  uint8_t m_pinClk;
  uint8_t m_pinDIO;
  uint8_t m_brightness;
  uint8_t lastData[4];
  uint8_t PointData;
  unsigned int m_bitDelay;
};

#define _dash 0x40
#define _under 0x08
#define _equal 0x48
#define _empty 0x00
#define _degree 0x63

#define _0 0x3f
#define _1 0x06
#define _2 0x5b
#define _3 0x4f
#define _4 0x66
#define _5 0x6d
#define _6 0x7d
#define _7 0x07
#define _8 0x7f
#define _9 0x6f

#endif // __TM1637DISPLAY__
