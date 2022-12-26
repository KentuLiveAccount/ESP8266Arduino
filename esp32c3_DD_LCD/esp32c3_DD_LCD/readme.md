Direct drive 3 digit 7 segment LED panel using ESP32-C3

Product: https://www.aliexpress.us/item/3256803559313291.html?spm=a2g0o.order_detail.order_detail_item.3.6262f19cuI4nMN&gatewayAdapt=glo2usa&_randl_shipto=US

Reference article: https://forum.arduino.cc/t/driving-a-salvaged-lcd-directly-from-arduino/51558

key concept: you can use two digital out to create AC flow, alternate between (D0, D1)={(1, 0), (0, 1)}

LCD needs to be driven with AC current to protect it from getting damaged (the voltage needs to sum up to 0 over time, with few tens of millisecond frequency)