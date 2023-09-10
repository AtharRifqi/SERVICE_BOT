#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

//// ganti sesuai punya lu sendiri
#define ssid "Athar"
#define password "pat13456"   

//// ganti sesuai punya lu sendiri
#define BOTtoken "6307094638:AAFTzUA_fKXjoLu0IZxW36fl-BuDZaVXgE8"  
#define idAnda "6348922504"

////
const int pompa = 14;
#define setpoint    65    // setpoint 
#define trigger     0     // 0 karena pake relay low act
#define durasiPompa 3000  
bool Mode = 1; // 1 mode otomatis 0 mode manual. (ini mode harus gua define lebih jelas si, tapi gua bingung sendiri malah.)

//// 
X509List cert(TELEGRAM_CERTIFICATE_ROOT); // tadi ditambahin ini
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

////
const unsigned long BOT_MTBS = 1500; 
unsigned long bot_lasttime;  
int persen = 0;

//// setup connection udah bisa
void setup() {
  Serial.begin(9600);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
 
 
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  client.setTrustAnchors(&cert);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("Retrieving time: ");
  configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);
  if (!client.connect("api.telegram.org", 443)) {
    Serial.println("Telegram connection failed");
    return;
  }

  client.setFingerprint("F2 AD 29 9C 34 48 DD 8D F4 CF 52 32 F6 57 33 68 2E 81 C1 90");

  pinMode(pompa, OUTPUT);
  digitalWrite(pompa, !trigger);

  Serial.println("");
  Serial.println("Connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  bot.sendMessage(idAnda, "Online");
  
 
}


//// function handle new message masih bermasalah 
void handleNewMessages(int numNewMessages) 
{
  Serial.println("handleNewMessages");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++) 
  {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "")
      from_name = "Guest";

    if (text == "/siram" && Mode == 0)
    {
      digitalWrite (pompa, trigger);
      delay (durasiPompa);
      digitalWrite (pompa, !trigger);
      
      bot.sendMessage(idAnda, "Tanaman disiram");
    }

    if (text == "/manual")
    {
      bot.sendMessage(idAnda, "Manual");
      Mode = 0;
    }
 
    if (text == "/otomatis")
    {
      bot.sendMessage(idAnda, "Auto");
      Mode = 1;
    }

    if (text == "/status")
    {
      bot.sendMessage(idAnda, "Kelembaban tanah:" + int(persen));
    } 

    if (text == "/start")
    {
      String welcome = "Selamat datang" + from_name + ".\n";
      welcome += "Ini adalah bot penyiram tanaman\n\n";
      welcome += "/manual untuk masuk ke mode manual\n";
      welcome += "/otomatis untuk masuk ke mode otomatis\n";
      welcome += "/status untuk cek status kelembaban tanah\n";
      bot.sendMessage(chat_id, welcome);
    }
  }
}



void loop()
{
persen = map(analogRead(A0),0,1024,0,100);
Serial.println (persen);
delay(1000);

  if (millis() - bot_lasttime > BOT_MTBS)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      Serial.println("Ada pesan masuk");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }

  if (persen > setpoint)
  { 
    digitalWrite (pompa, trigger);
    delay (durasiPompa);
    digitalWrite (pompa, !trigger);
      
    bot.sendMessage(idAnda, "Tanaman disiram otomatis");
      
  }
}
