#include <GDBStub.h>

#include <SPI.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <SD.h>
#include <Arduino.h>
ESP8266WiFiMulti WiFiMulti;

File SdCard;
File saveCode;
int kmAwal;
int kmAkhir;
int KmTotal;
int timer=1000;
int namaCode;
String exe = ".log";
String namaFile="log";
String code = "code.log";
String dataLamaAkhir="";
String dataIsi;
String sdCardData;
String payload;
int pinCS = 4;
String datas;
bool pertama = true;

void setup() {
  
  Serial.begin(115200);
  while (!Serial) {
    
  }
  pinMode(pinCS,OUTPUT);
  Serial.print("Initializing SD card...");
  if (!SD.begin(pinCS)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
    for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("Kedai ichi", "beliduludong");
  
}





void loop() {
    
    datas = String(ambilDataSensor()) ;
    tulisDataSdCard(datas);
    delay(1000);
    
    httpGet();
delay(1000);

}     
   
void cekFileSize(){
  int titikPertama;
  int titikkedua; 
  String dataLama= "";
  
  Serial.println("nama file yang di cek: "+namaFile+exe);
  SdCard=SD.open(namaFile+exe,FILE_READ);
  Serial.println("Ukuran file Sekarang: "+String(SdCard.size()));
    while(SdCard.available()){
    char data = SdCard.read();
    dataLama += String(data);
    titikPertama = dataLama.lastIndexOf("(");
    titikkedua = dataLama.lastIndexOf(")");
    dataLamaAkhir = dataLama.substring(titikPertama + 1,titikkedua);
  }
  if(SdCard.size()>= 200){
     for(int i = 0 ;i < saveCode.size();i++){
      namaFile = String(i)+exe;
      cekFileSize();
     }
     saveCode=SD.open(code,FILE_WRITE);
     saveCode.print("1");
      
     
      
    namaFile = String(saveCode.size(),DEC);
    Serial.println("Save code size: "+String(saveCode.size()));
    Serial.println("Nama File baru: "+namaFile+exe);
    
    saveCode.flush();
    saveCode.close();
    SdCard.close();
     bacaDataSdCard();
    tulisDataSdCard(datas);
    Serial.println("###########################################");
      
  } else{
    SdCard.close();
     Serial.println("Nama File Sekarang: "+namaFile);
    Serial.println("###########################################");
   } 
}
  









void tulisDataSdCard(String baca){
  Serial.println("Menambahkan data Pada Sdcard Km Awal Dan Km Akhir");
  bacaDataSdCard();
  String dataSdcardAkhir = dataIsi;
  Serial.println("Km Sdcard Akhir "+ dataSdcardAkhir);
  
  kmAkhir = dataSdcardAkhir.toInt();
  
  kmAwal = baca.toInt();
  KmTotal = kmAkhir + kmAwal;
  
  
  Serial.println("Km AWAL: "+String(kmAwal));
  Serial.println("Km AKHIR:" +String(KmTotal));

 
  
  SdCard=SD.open("LOG.txt",FILE_WRITE);
  if(SdCard){
    SdCard.println("sensor data->"+baca+" Total Km ->("+String(KmTotal)+")");
   
    SdCard.flush();
    SdCard.close();
  }
  else{
    Serial.println("Gagal Menyimpan data");
  }
  Serial.println("###########################################");
delay(2000);
}



void bacaDataSdCard(){
  Serial.println("Ambil Data Km Terakhir Pada Sd Card");
    int titikPertama;
    int titikkedua; 
    
 SdCard = SD.open("LOG.txt",FILE_READ);
 
  while(SdCard.available()){
    char data = SdCard.read();
    sdCardData += String(data);// global SdcardData type String (isi semua data dalam sdcard)  
  }
 
  SdCard.flush();
  SdCard.close();

 
  
  titikPertama = sdCardData.lastIndexOf("(");//titik pengambilan data start
  titikkedua = sdCardData.lastIndexOf(")");//titik pengambilan data end
  dataIsi = sdCardData.substring(titikPertama + 1,titikkedua);//ambil data akhir dari sdcard simpan di var dataIsi
  Serial.println("Data Terbaca : "+dataIsi);
  Serial.println("###########################################");
  delay(2000);
}



void httpGet(){
  Serial.println("Mengirim Database Server sesuia no plat mobil");
   if ((WiFiMulti.run() == WL_CONNECTED)) {
    int dataUrl = dataIsi.toInt();
    String noPol= "b2200cia";
    int dataSensor = ambilDataSensor();
    int dataTotal = dataUrl + dataSensor; 
    String url;
    if(pertama){
    url ="http://reminder.96.lt/pertama.php?no=b2525via";
    pertama = false;
    Serial.println(url);
    }else{
      
   
    url = "http://reminder.96.lt/SETTER.php?km=";
    String urlPlat = "&no="+noPol;
    url += dataTotal;
    url += urlPlat;
    Serial.println(url);

     }
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
   
    http.begin(url);

    Serial.print("[HTTP] GET...\n");
   
    int httpCode = http.GET();

   
    if (httpCode > 0) {
     
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

     
      if (httpCode == HTTP_CODE_OK) {
       payload = http.getString();
        Serial.println(payload);
        Serial.println("#######################################################");
        Serial.println();
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }

  delay(timer);
}






int ambilDataSensor(){
  Serial.println("Ambil data Sensor ICU jarak Tempuh/jam");
  int dataKmRata2 = 0;
  for(int i = 0 ;i < 100;i++){
      int data = analogRead(A0);
      
      int dataDalamKm = map(data,100,1024,0,120);
      dataKmRata2 += dataDalamKm;
    delay(10);
  }
  //Kondisi dimana kedaraan berhenti
  if(dataKmRata2/100 <= 0 || dataKmRata2/100 >= 120){

    Serial.println("Data Sensor = 0 ");
    Serial.println("###########################################");
    return 0; //jika kendaraan berhenti akan mengembalikan 0
    
    
  }else{

    Serial.println("Data Sensor = "+String(dataKmRata2/100));
    Serial.println("###########################################");
    return dataKmRata2/100+200; //jika kendaraan berjalan akan mengambalikan
                            //nilai km tempuh dalam 1 jam bukan km/jam
    
  }
  delay(1000);
}
