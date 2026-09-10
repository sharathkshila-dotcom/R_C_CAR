//TECHBOTS

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// WiFi AP
const char *ssid = "RC_CAR_ESP";
const char *password = "12345678";

// L298N
#define ENA D5
#define ENB D6
#define IN1 D7
#define IN2 D8
#define IN3 D1
#define IN4 D2

ESP8266WebServer server(80);

int leftSpeed = 0;
int rightSpeed = 0;
int maxBaseSpeed = 150;

void setup()
{
  Serial.begin(115200);

  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  stopMotors();

  WiFi.softAP(ssid, password);
  Serial.println("AP Started");
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/forward", forward);
  server.on("/backward", backward);
  server.on("/left", left);
  server.on("/right", right);

  server.on("/forward_left", forwardLeft);
  server.on("/forward_right", forwardRight);
  server.on("/backward_left", backwardLeft);
  server.on("/backward_right", backwardRight);

  server.on("/stop", stopCar);
  server.on("/speed", setSpeed);

  server.begin();
}

void loop()
{
  server.handleClient();
  updateMotors();
  delay(20);
}

/* ---------- FIXED HTML (NO ERRORS) ---------- */

void handleRoot()
{
  String html = R"HTML(
<!DOCTYPE html>
<html>
<head>
<meta name='viewport' content='width=device-width,initial-scale=1'>
<title>RC Car</title>

<style>
body{background:#111;color:#eee;text-align:center;font-family:sans-serif;margin:0;padding:20px}
#joy{width:220px;height:220px;background:#333;border-radius:50%;margin:20px auto;position:relative;touch-action:none}
#knob{width:70px;height:70px;background:#0f0;border-radius:50%;position:absolute;left:75px;top:75px;transition:0.05s}
.btn{padding:15px 25px;margin:5px;font-size:16px;border:0;border-radius:8px;background:#444;color:#fff}
</style>
</head>

<body>
<h2>&#128663; ESP8266 RC Car</h2>

<div id='joy'>
  <div id='knob'></div>
</div>

<button class='btn' onclick='setSpeed(255)'>MAX</button>
<button class='btn' onclick='setSpeed(150)'>MID</button>
<button class='btn' onclick='setSpeed(80)'>SLOW</button>

<script>
let joy=document.getElementById("joy");
let knob=document.getElementById("knob");
let lastCmd="";
let pressed=false;

function send(c){
  if(c!==lastCmd){ fetch(c); lastCmd=c; }
}

function handle(e){
  if(e.type==="mousemove" && !pressed) return; // ignore hover on desktop
  e.preventDefault();
  let r=joy.getBoundingClientRect();
  let x=(e.touches?e.touches[0].clientX:e.clientX)-(r.left+110);
  let y=(e.touches?e.touches[0].clientY:e.clientY)-(r.top+110);

  let d=Math.hypot(x,y);
  if(d>100){ x=x/d*100; y=y/d*100; }

  knob.style.transform=`translate(${x}px,${y}px)`;

  let cmd="/stop";
  if(Math.abs(y)>25) cmd=y<0?"/forward":"/backward";
  if(Math.abs(x)>25){
    if(cmd=="/forward") cmd=x>0?"/forward_right":"/forward_left";
    else if(cmd=="/backward") cmd=x>0?"/backward_right":"/backward_left";
    else cmd=x>0?"/right":"/left";
  }
  send(cmd);
}

function reset(){
  pressed=false;
  knob.style.transform="translate(0,0)";
  send("/stop");
}

joy.addEventListener("mousedown",()=>{pressed=true;});
joy.addEventListener("mousemove",handle);
joy.addEventListener("touchmove",handle);
joy.addEventListener("mouseup",reset);
joy.addEventListener("mouseleave",reset);
joy.addEventListener("touchend",reset);

function setSpeed(v){ fetch("/speed?value="+v); }
</script>

</body>
</html>
)HTML";

  server.send(200, "text/html", html);
}

/* ---------- Motor Control ---------- */

void forward()
{
  leftSpeed = maxBaseSpeed;
  rightSpeed = maxBaseSpeed;
}
void backward()
{
  leftSpeed = -maxBaseSpeed;
  rightSpeed = -maxBaseSpeed;
}
void left()
{
  leftSpeed = -100;
  rightSpeed = 100;
}
void right()
{
  leftSpeed = 100;
  rightSpeed = -100;
}

void forwardLeft()
{
  leftSpeed = maxBaseSpeed * 0.5;
  rightSpeed = maxBaseSpeed;
}
void forwardRight()
{
  leftSpeed = maxBaseSpeed;
  rightSpeed = maxBaseSpeed * 0.5;
}

void backwardLeft()
{
  leftSpeed = -(maxBaseSpeed * 0.5);
  rightSpeed = -maxBaseSpeed;
}
void backwardRight()
{
  leftSpeed = -maxBaseSpeed;
  rightSpeed = -(maxBaseSpeed * 0.5);
}

void stopCar()
{
  leftSpeed = 0;
  rightSpeed = 0;
}

void setSpeed()
{
  if (server.hasArg("value"))
    maxBaseSpeed = server.arg("value").toInt();
  server.send(200, "text/plain", "OK");
}

void updateMotors()
{
  if (leftSpeed > 0)
  {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, leftSpeed);
  }
  else if (leftSpeed < 0)
  {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(ENA, -leftSpeed);
  }
  else
  {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 0);
  }

  if (rightSpeed > 0)
  {
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, rightSpeed);
  }
  else if (rightSpeed < 0)
  {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENB, -rightSpeed);
  }
  else
  {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, 0);
  }
}

void stopMotors()
{
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}
