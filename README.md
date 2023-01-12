Эксперимент по определению возможностей применения ESP32 в SDR

На данный момент проект в очень предварительной версии, но основной функционал реализован
в том числе:
1.Прием и передача SSB
2.Отображение основной информации, включая спектр и "водопад"
3.Управление основными функциями:
  - изменение полос пропускания фильтра основной селекции
  - переключение  LSB/USB
  - перестройка по диапазону или по отображаемой панораме

Кратко об особенностях реализации.
Для отображения информации выбран LCD с параллельным (tft24) интерфейсом как наименее
затратный с точки зрения нагрузки процессора способ вывода изображения (подробнее в моем 
соседнем репозитарии).

Фильтры FIR/IIR реализованы на ассемблере (позаимствовано из ESP-DSP).
Для FFT-функций используется набор xtensa-math (переписанный cmsis-dsp для arm)
В виду наличия в esp32 двух ядер , удалось гладко распараллелить задачи - 
операции ЦОС в основном выполняются на одном ядре, а управление и отображение - на другом.

Код написан в среде Ардуино 1.8.13
Никаких ограничений на использование этого проекта или его частей нет.
