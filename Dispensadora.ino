//Celda de carga
#include "HX711.h" //libreria de la celda de carga
#include <Wire.h>
#include <EEPROM.h> //libreria para minupular la memora dl arduino

#include <LiquidCrystal_I2C.h> //libreria para minupular lcd
#include <Keypad_I2C.h> //libreria para el teclado
#include <LiquidMenu.h>
#include <Servo.h> //generador de pwm para controlae el servo
LiquidCrystal_I2C lcd(0x27, 16, 2);
//celda de carga
HX711 balanza;
const int zero = 2;
int DT = 4;
int CLK = 5;
int peso_calibracion= 714; // Es el peso referencial a poner, en mi caso mi celular pesa 185g (SAMSUMG A20)
long escala;
int state_zero = 0;
int last_state_zero = 0;
unsigned long int Numero;

//teclado
const byte Filas = 4;
const byte Columnas = 4;
char Teclado[Filas][Columnas] ={
{'D','#','0','*'},
{'C','9','8','7'},
{'B','6','5','4'},
{'A','3','2','1'}
};

byte PinesFilas[Filas] = {0,1,2,3};
byte PinesColumnas[Columnas] = {4,5,6,7};


Keypad_I2C kpd = Keypad_I2C(makeKeymap(Teclado),PinesFilas,PinesColumnas,Filas,Columnas,0x38);//makeKeymap(Teclado),PinesFilas,PinesColumnas,Filas,Columnas,direccion
//////////////////////////////////////////
int y=0;
String valorpedido="";
int opcion1_seleccionado = 0;
int formato_medicion=0;
int cafe_seleccionado=0;
int espera_mensaje=450;
int menu1=0;
int i=0;
int angulo=0;
char Tecla;
double peso=0;
int valor_servo=0;
int potPin = A0  ; // Pin analógico utilizado por el potenciómetro
Servo myservo; // Crea un objeto servo
//menu principal
LiquidLine linea1(1, 0, "Manual");
LiquidLine linea2(1, 1, "Teclado");
LiquidLine linea3(1, 0, "Wifi");
LiquidLine linea4(1, 1, "Calibrar");
LiquidScreen pantalla1(linea1,linea2,linea3,linea4);
//atras LiquidMenu
LiquidLine linea1_4(11,0,"Atras");
LiquidScreen pantalla4(linea1_4);
LiquidMenu menu(lcd,pantalla1,pantalla4);

void setup() {

  myservo.attach(9);//pwm del servo
  lcd.init();
  kpd.begin();
  lcd.backlight();
  menu.init();
  linea1.set_focusPosition(Position::LEFT);
  linea2.set_focusPosition(Position::LEFT);
  linea3.set_focusPosition(Position::LEFT);
  linea4.set_focusPosition(Position::LEFT);

  linea1.attach_function(1, fn_Manual1);
  linea2.attach_function(1, fn_Teclado2);
  linea3.attach_function(1, fn_wifi3);
  linea4.attach_function(1, fn_calibrar);

  menu.add_screen(pantalla1);
  /////////////////////////////////////////////


  linea1_4.set_focusPosition(Position::LEFT);
  linea1_4.attach_function(1, fn_atras1);
  menu.add_screen(pantalla4);
  pantalla1.set_displayLineCount(2);
  pantalla4.set_displayLineCount(2);
  menu.set_focusedLine(0);
  //Sensor de carga
  balanza.begin(DT, CLK);//asigana los pines para el recibir el trama del pulsos que viene del modulo
  pinMode(zero, INPUT);//declaramos el pin2 como entrada del pulsador
  pinMode(13,OUTPUT);
  lcd.init(); // Inicializamos el lcd
  lcd.backlight(); // encendemos la luz de fondo del lcd
  EEPROM.get( 0, escala );//Lee el valor de la escala en la EEPROM
  if (digitalRead(zero) == 1) 
    { //esta accion solo sirve la primera vez para calibrar la balanza, es decir se presionar ni bien se enciende el sistema
    calibration();
    }
  balanza.set_scale(escala); // Establecemos la escala
  balanza.tare(20);  //El peso actual de la base es considerado zero.
  menu.update();
}
void loop()
{
  Tecla = kpd.getKey();
  selectOption(Tecla);
  actualizar_menu(Tecla);

    if(menu1==2)
    {
        switch (opcion1_seleccionado) {
        case 1:
        encoder();//Manual
        break;
        case 2:
        pedirvalor();//teclado matricial
        break;
        case 3:
        lectura_wifi();//wifi
        break;
        case 4:
        calibration();//calibrar bascula
        break;
      }
  }
}

//Funciones:::::
void actualizar_menu(char Tecla){

  if ((Tecla=='B')||(Tecla=='A')){
      if (Tecla== 'A'){
          menu.switch_focus(false);
        }
      else if(Tecla=='B'){
        menu.switch_focus(true);
        }

      menu.update();
      }
       if(Tecla=='D')
        {
          if(menu1!=1)
        {
          menu1=menu1-1;

          menu.change_screen(menu1);
          menu.set_focusedLine(0);
        }
        }
}
void selectOption(char Tecla){
    if (Tecla=='C'){
      menu.call_function(1);
      delay(500);
  }
}
void fn_Manual1(){
  menu1=2;
  menu.change_screen(menu1);
  menu.set_focusedLine(0);
  opcion1_seleccionado = 1;
}
void fn_Teclado2(){
  menu1=2;
  menu.change_screen(menu1);
  menu.set_focusedLine(0);
  opcion1_seleccionado = 2;
}
void fn_wifi3(){
  menu1=2;
  menu.change_screen(menu1);
  menu.set_focusedLine(0);
  opcion1_seleccionado = 3;
 

}
void fn_calibrar(){
  menu1=2;
  menu.change_screen(menu1);
  menu.set_focusedLine(0);
  opcion1_seleccionado = 4;
}
void fn_atras1(){
  menu1=1;
  menu.change_screen(menu1);
  menu.set_focusedLine(0);
}

void calibration() {//Función calibración



  boolean conf = true;
  long adc_lecture;
  // restamos el peso de la base de la balaza
  lcd.setCursor(0, 0);
  lcd.print("Calibrando base");
  lcd.setCursor(4, 1);
  lcd.print("Balanza");
  delay(3000);
  balanza.read();
  balanza.set_scale(); //La escala por defecto es 1
  balanza.tare(20);  //El peso actual es considerado zero.
  lcd.clear();

 lcd.clear();
  lcd.print("dame el peso");
  lcd.print(" g  ");
  char key;
 do{
   key =kpd.getKey();
   if (key != NO_KEY) 
   {
    if(key=='D')
      {
      menu1=1;
      menu.change_screen(menu1);
      menu.set_focusedLine(0);
      break;
      }
    if(key=='C')
    {
      Numero=valorpedido.toInt();
      if(Numero>20000)
      {
      lcd.setCursor(0,1);
      lcd.print("            ");
      lcd.setCursor(0,1);
      lcd.print("entrada invalida");
      delay(800);
      lcd.setCursor(0,1);
      lcd.print("valor max 20000 ");
      delay(800);
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      key='p';
      }
      valorpedido="";
    }
    if (key == '*') {
      valorpedido = valorpedido.substring(0, valorpedido.length() - 1);
      lcd.setCursor(0,1);
      lcd.print("               ");
      lcd.setCursor(0,1);
      lcd.print(valorpedido);
    }
    if (key >= '0' && key <= '9') {
       lcd.setCursor(0,1);
      valorpedido += key;
      lcd.print(valorpedido);
      }
    }
  }while(key!='C');
lcd.clear();
peso_calibracion=Numero;

  //Iniciando calibración
  while (conf == true) {
    lcd.setCursor(1, 0);
    lcd.print("Peso referencial:");
    lcd.setCursor(1, 1);
    lcd.print(peso_calibracion);
    lcd.print(" g        ");
    delay(3000);
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Ponga el Peso");
    lcd.setCursor(1, 1);
    lcd.print("Referencial");
    delay(3000);
    //Lee el valor del HX711
    adc_lecture =-balanza.get_value(100);
    //Calcula la escala con el valor leido dividiendo el peso conocido
    escala = adc_lecture / peso_calibracion;
    //Guarda la escala en la EEPROM
    EEPROM.put( 0, escala );
    delay(100);
    lcd.setCursor(1, 0);
    lcd.print("Retire el Peso");
    lcd.setCursor(1, 1);
    lcd.print("referencial");
    delay(3000);
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("READY!!....");
    delay(3000);
    lcd.clear();
    conf = false; //para salir de while
  }
    lcd.clear();
    menu1=1;
   setup();
   menu.change_screen(menu1);
   menu.set_focusedLine(0);
   
}
void bascula_peso(){
  int state_zero = digitalRead(zero);
  peso =-balanza.get_units(10);  //Mide el peso de la balanza
  //Muestra el peso
  lcd.print("        ");
  lcd.setCursor(0, 0);
  lcd.print(peso, 0);
  lcd.print(" g    ");
  delay(5);
  //Botón de zero, esto sirve para restar el peso de un recipiente
  if ( state_zero != last_state_zero) {
    if (state_zero == LOW) {
      balanza.tare(10);  //El peso actual es considerado zero.
    }
  }
  char key;
   key =kpd.getKey();
   if (key != NO_KEY) 
   {
    if(key=='#')
    {
      balanza.tare(10);  
    }
      
    if (key =='D') {
        controlarServo(80);
        y=1;
        lcd.clear();
        lcd.print("Apagado de");
        lcd.setCursor(0,1);
        lcd.print("emergencia");
        delay(1000);
         lcd.clear();
   
    }
  }

  last_state_zero  = state_zero;
  if (peso>=500)digitalWrite(13,1);
  else if(peso<=500)digitalWrite(13,0);
}

void encoder(){
   Tecla='A';
   while(Tecla!='C')
     {
    valor_servo= analogRead(potPin);    
    valor_servo= map(valor_servo, 0, 1023, 0, 180);
    showProgressBar(valor_servo);
    bascula_peso();
    delay(50); // Velocidad de la animación
    Tecla=kpd.getKey();
    if ( Tecla!=NO_KEY)
    {
      if(Tecla=='C')
      {
      menu1=1;
      menu.change_screen(menu1);
      menu.set_focusedLine(0);
      break;
      }
     }
    }//while(Tecla!='C')
}//Manual
void showProgressBar(int progress) {
  controlarServo(progress);
  progress= map(progress, 0, 180, 0, 100);
   lcd.setCursor(13,1);
  lcd.print(progress);
  lcd.print("%");
  lcd.setCursor(0, 1);
  lcd.print("[");
  for (int i = 0; i < progress/10; i++) {
    lcd.print("=");
  }
  for (int i = progress/10; i < 10; i++) {
    lcd.print(" ");
  }
  lcd.print("]");
}
void controlarServo(int valor) {
 // Mueve el servo a la posición correspondiente
  myservo.write(valor);
 
 }
//funciones no hechas
void pedirvalor(){
  int p=0;
  lcd.clear();
  lcd.print("dame el peso");
  lcd.print(" g  ");
  char key;
 do{
   key =kpd.getKey();
   if (key != NO_KEY) 
   {
    if(key=='D')
      {
      menu1=1;
      menu.change_screen(menu1);
      menu.set_focusedLine(0);
      break;
      }
    if(key=='C')
    {
      Numero=valorpedido.toInt();
      if(Numero>20000)
      {
      lcd.setCursor(0,1);
      lcd.print("            ");
      lcd.setCursor(0,1);
      lcd.print("entrada invalida");
      delay(800);
      lcd.setCursor(0,1);
      lcd.print("valor max 20000 ");
      delay(800);
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      key='p';
      valorpedido="";
      }
    }
    if (key == '*') {
      valorpedido = valorpedido.substring(0, valorpedido.length() - 1);
      lcd.setCursor(0,1);
      lcd.print("               ");
      lcd.setCursor(0,1);
      lcd.print(valorpedido);
    }
    if (key >= '0' && key <= '9') {
       lcd.setCursor(0,1);
      valorpedido += key;
      lcd.print(valorpedido);
      }
    }
  }while(key!='C');
  if(key!='D'){
   Numero=valorpedido.toInt();
   valorpedido="";
    llenado_automatico(Numero);
  }
}//teclado pedir los valores con el teclado y validacion
void lectura_wifi(){
  lcd.clear();
  lcd.print("No disponible");
  delay(1000);
  menu1=1;
  menu.change_screen(menu1);
  menu.set_focusedLine(0);
}//wifi
void llenado_automatico(unsigned long int Numero){
y=0;
long int mas;
long int menos;
char key;
mas=Numero+10;
menos=Numero-10;
lcd.clear();
do
{
  bascula_peso();
  if((menos>peso)&&(y==0))// Abrir la tolva cuando aun no llega al peso
   {
     controlarServo(0);
    }
    if(y==1){
      controlarServo(80);
    }
   if(peso>=menos)// si ya llegamos a la cantidad deseada entonces cerramos el motor
   {
   if(peso<=mas)
    {
      controlarServo(80); 
      lcd.clear();
      lcd.setCursor(0,1);
      lcd.print("peso deseado");
      delay(1500);
      y=1;
      preguntar();
    }
  }
  key=kpd.getKey();
  if (key != NO_KEY) {
    if(key=='D'){
        controlarServo(80);
        y=1;
        lcd.clear();
        lcd.print("Apagado de");
        lcd.setCursor(0,1);
        lcd.print("emergencia");
        delay(1000);
         menu1=1;
        menu.change_screen(menu1);
        menu.set_focusedLine(0);

    }
  }
  } while (y==0);
}
void preguntar(){//pregunta el peso  nuevamente y lo guarda
 char opc;
 int u=0;
 lcd.clear();
 lcd.setCursor(0,0);
 lcd.print("mismo peso");
 lcd.setCursor(0,1);
 lcd.print("  si=1  , no=0  ");
  do{
    opc=kpd.getKey();
    if((opc=='0')||(opc=='1'))
      {
        u=1;
        lcd.clear();
      }
    }while(u==0);
    if(opc=='1')
      {
        y=0;
        lcd.clear();
      }
    
      if(opc=='0'){
         menu1=1;
      menu.change_screen(menu1);
      menu.set_focusedLine(0);
      }
}
