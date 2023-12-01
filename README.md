# SmartParking
21900764 최준혁, 22100113 김성빈

### Project Folder Structure
```
README.md

Design/
  ESPCAM/
    ESPCAM.ino
  UltraSonic/
    UltraSonic.ino
  hass/
    configuration.yaml
  py/
    cam_mqtt_countCar_when_signal.py

Doc/
  car.png
  5조_최준혁_김성빈_설계과제2_보고서.docx
```

### Hardware
* ESP32-CAM
* HC-SR04 Ultrasonic Sensor
* Raspberry Pi
* ESP8266 NodeMCU

### Build
빌드를 하기 위해서는 하드웨어 각각의 파일을 빌드하고 업로드하는 작업을 진행해야 한다.

1. ESP32-CAM

ESPCAM/ESPCAM.ino 파일을 사용한다.

2. HC-SR04 Ultrasonic Sensor

UltraSonic/UltraSonic.ino 파일을 사용한다.

3. Raspberyy Pi

* Home Assistant를 빌드 할때 사용되는 configuration.yaml을 hass/configuration.yaml 파일로 대체하야 사용한다.
* ESP32-CAM을 통해 받은 영상 자동차 감지를 위해 py/cam_mqtt_countCar_when_signal.py를 사용해서 실행한다.

### Test
실내 환경에서 카메라 감지등 기능을 테스트하기 위해 Doc/car.png 이미지가 준비되어있다.
