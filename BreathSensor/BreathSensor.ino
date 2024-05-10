#define AMBANG_BATAS_DETEKSI 100 // Nilai ambang batas untuk mendeteksi suara
#define AMBANG_BATAS_TIDAK_DETEKSI 4095 // Nilai ambang batas saat tidak ada suara

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  static unsigned long lastMinute = 0; // variabel untuk menyimpan waktu terakhir perhitungan
  static unsigned long soundCount = 0; // variabel untuk menyimpan jumlah suara yang terdeteksi
  static int ambangBatas = AMBANG_BATAS_TIDAK_DETEKSI; // Nilai ambang batas sensor default
  
  unsigned long currentMillis = millis(); // mendapatkan waktu saat ini
  
  // Membaca sensor suara
  int soundValue = analogRead(34);
  Serial.println(soundValue);
  // Menyesuaikan nilai ambang batas berdasarkan deteksi suara
  if(soundValue < AMBANG_BATAS_DETEKSI) {
    ambangBatas = AMBANG_BATAS_TIDAK_DETEKSI; // Gunakan nilai ambang batas tinggi saat tidak ada suara terdeteksi
    soundCount++;
  } else {
    ambangBatas = 0; // Ubah nilai ambang batas kembali ke default jika suara terdeteksi
  }
  
  // Menghitung jumlah suara per menit
  if(currentMillis - lastMinute >= 60000) { // 60000 milliseconds = 1 menit
    float soundPerMinute = (float)soundCount / ((float)(currentMillis - lastMinute) / 60000.0);
    Serial.print("Jumlah suara per menit: ");
    Serial.println(soundPerMinute);
    
    // Reset jumlah suara dan waktu terakhir perhitungan
    soundCount = 0;
    lastMinute = currentMillis;
    delay(5000);
  }
  
  delay(50);
}