#include <Adafruit_NeoPixel.h>

#define PIN            6  // NeoPixel modülünün bağlı olduğu pin
#define NUM_PIXELS     12 // LED sayısı

Adafruit_NeoPixel strip(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);

unsigned long lastUpdate = 0;
unsigned long rainbowDelay = 20; // Rainbow efektinin güncellenme hızı

bool rainbowActive = false;  // Rainbow efektinin aktif olup olmadığını kontrol etmek için

void setup() {
  Serial.begin(9600);  // Seri iletişim başlat
  strip.begin();  // NeoPixel modülünü başlat
  strip.show();   // Başlangıçta tüm LED'leri kapalı tut
  Serial.println("LED Control: Send commands like '1red', 'allblue', 'rainbow' or '1#FF5733'.");
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();  // Girilen komutu temizle (boşluklardan arındır)
    processCommand(input);  // Komutu işle
  }

  // Gökkuşağı efekti her 'rainbowDelay' milisaniyede bir çalışır
  if (rainbowActive) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastUpdate >= rainbowDelay) {
      lastUpdate = currentMillis;
      rainbowEffect();  // Gökkuşağı efekti çalıştır
    }
  }
}

// Komutları işleyen fonksiyon
void processCommand(String command) {
  // Rainbow efekti çalışırken herhangi bir LED komutu verilirse, rainbow durdurulacak
  if (rainbowActive && !command.startsWith("rainbow")) {
    rainbowActive = false;  // Rainbow efektini durdur
    strip.clear();  // Tüm LED'leri kapat
    strip.show();
    Serial.println("Rainbow effect stopped.");
  }

  if (command.startsWith("all")) {
    // 'all' komutunun ardından gelen renk ne olursa olsun işlenecek
    if (command == "allred") {
      setAllColor(strip.Color(255, 0, 0));  // Kırmızı
    } else if (command == "allgreen") {
      setAllColor(strip.Color(0, 255, 0));  // Yeşil
    } else if (command == "allblue") {
      setAllColor(strip.Color(0, 0, 255));  // Mavi
    } else if (command == "allwhite") {
      setAllColor(strip.Color(255, 255, 255));  // Beyaz
    } else if (command == "alloff") {
      strip.clear();  // Tüm LED'leri kapat
      strip.show();
      rainbowActive = false;  // Rainbow efekti kapatıldı
      Serial.println("All LEDs are off.");
    } else if (command.startsWith("all#")) {
      setAllColor(hexToColor(command.substring(4)));  // HEX renk kodu
    } else {
      Serial.println("Invalid 'all' command.");
    }
  } else if (command.startsWith("rainbow")) {
    rainbowActive = true;  // Rainbow efekti aktif yapıldı
    Serial.println("Rainbow effect started.");
  } else if (command == "off") {
    rainbowActive = false;  // Rainbow efekti durduruldu
    strip.clear();  // Tüm LED'leri kapat
    strip.show();
    Serial.println("Rainbow effect stopped.");
  } else {
    // LED numarası ve renk komutları (1red, 10green, vb.)
    int ledNumber = 0;
    String color = "";
    
    // İlk önce komutun sayısal kısmını kontrol edelim
    int i = 0;
    while (i < command.length() && isDigit(command.charAt(i))) {
      ledNumber = ledNumber * 10 + (command.charAt(i) - '0');
      i++;
    }

    // Eğer sayısal kısmı bulduysak, geri kalan kısmı alalım
    if (ledNumber >= 1 && ledNumber <= NUM_PIXELS) {
      // Geri kalan kısmı renk olarak al
      color = command.substring(i);

      uint32_t colorCode;
      
      // HEX kodu ile giriş
      if (color.startsWith("#")) {
        colorCode = hexToColor(color.substring(1));  // HEX renk kodu ile renk ata
      }
      // Yazılı renk komutu ile renk atama
      else {
        colorCode = getColorFromString(color);  // Yazılı renk komutunu işleyerek renk kodunu al
      }

      setLEDColor(ledNumber - 1, colorCode);  // LED'e renk ata
    } else {
      Serial.println("Invalid command.");
    }
  }
}

// Belirtilen LED'in rengini ayarlama
void setLEDColor(int index, uint32_t color) {
  strip.setPixelColor(index, color);  // Renk kodu ile LED'i ayarla
  strip.show();  // LED'leri güncelle
}

// Tüm LED'lere aynı rengi atama
void setAllColor(uint32_t color) {
  for (int i = 0; i < NUM_PIXELS; i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}

// Gökkuşağı efekti
void rainbowEffect() {
  // LED'lere renk atama
  int wheelPos = millis() / 10;  // Zamanla hareket eden renkler
  for (int i = 0; i < NUM_PIXELS; i++) {
    int colorWheelPos = (i * 256 / NUM_PIXELS + wheelPos) % 256;  // Her LED için farklı renk tonu
    strip.setPixelColor(i, Wheel(colorWheelPos));  // Wheel fonksiyonu renk yaratır
  }
  strip.show();
}

// Renk çarkı fonksiyonu (gökkuşağını oluşturur)
uint32_t Wheel(byte WheelPos) {
  if (WheelPos < 85) {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);  // Kırmızı'dan Sarı'ya
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);  // Sarı'dan Mavi'ye
  } else {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);  // Mavi'den Kırmızı'ya
  }
}

// HEX renk kodunu uint32_t renk formatına dönüştürme
uint32_t hexToColor(String hexColor) {
  // HEX renk kodu doğru formatta mı?
  if (hexColor.length() == 6) {
    long number = strtol(hexColor.c_str(), NULL, 16);  // Hex kodunu long tipe dönüştür
    byte r = (number >> 16) & 0xFF;
    byte g = (number >> 8) & 0xFF;
    byte b = number & 0xFF;
    return strip.Color(r, g, b);  // Renk kodunu döndür
  } else {
    Serial.println("Invalid HEX color code.");
    return strip.Color(0, 0, 0);  // Hatalı hex kodu durumunda siyah döndür
  }
}

// Yazılı renk komutlarından `uint32_t` değerini döndürme
uint32_t getColorFromString(String color) {
  if (color == "red") {
    return strip.Color(255, 0, 0);  // Kırmızı
  } else if (color == "green") {
    return strip.Color(0, 255, 0);  // Yeşil
  } else if (color == "blue") {
    return strip.Color(0, 0, 255);  // Mavi
  } else if (color == "yellow") {
    return strip.Color(255, 255, 0);  // Sarı
  } else if (color == "white") {
    return strip.Color(255, 255, 255);  // Beyaz
  } else {
    return strip.Color(0, 0, 0);  // Geçersiz renk için siyah döndür
  }
}
