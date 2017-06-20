
add_library(car Car.cpp Gpio.cpp I2cBus.cpp PCA9685.cpp Pin.cpp PwmEsc.cpp PwmServo.cpp RaspberryPi.cpp)

# Make sure the compiler can find include files for our car library
# when other libraries or executables link to Hello
target_include_directories (car PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ../../wiring_pi_lite)
