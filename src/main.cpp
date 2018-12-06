#include <Arduino.h>

//************************************************************
// this is a simple example that uses the painlessMesh library
//
// 1. sends a silly message to every node on the mesh at a random time between 1 and 5 seconds
// 2. prints anything it receives to Serial.print
//
//
//************************************************************
#include "painlessMesh.h"

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

uint32_t nNodoGatewayId;
bool bCoordinadorConectado;
bool bTodosLosNodosADormir;
bool bDatoRecibidoEnGateway;

// User stub

// Prototypes so PlatformIO doesn't complain
void pedirDato();
void leerDato();
void EsperarTodosLosNodosParaDormir();
void ComandoADormir();
void ConfirmacionRecibido();

//------ Debug Sync
void LEDOn();
void LEDOff();
//------

Task taskPedirDatoAArduino(TASK_SECOND * 1, TASK_ONCE, &pedirDato, &userScheduler, true);
Task taskLeerDatoSerial(TASK_SECOND * 1, TASK_ONCE, &leerDato, &userScheduler, false);
Task taskEsperarConfirmacionRecibidoGateway(TASK_SECOND * 3, TASK_ONCE, &ConfirmacionRecibido, &userScheduler, false);
Task taskEsperarTodosLosNodosParaDormir(TASK_SECOND * 0 , TASK_FOREVER, &EsperarTodosLosNodosParaDormir, &userScheduler, false);
Task taskComandoADormir(TASK_SECOND * 1 , TASK_ONCE, &ComandoADormir, &userScheduler, false);
//Task leerDato(TASK_SECOND * 1, TASK_ONCE, &leerDato, &userScheduler, true);

//------ Debug Sync
Task tLED(TASK_SECOND * 3, TASK_FOREVER, NULL, &userScheduler, false, NULL, &LEDOff);
//------

void pedirDato() {

  Serial.printf("QuieroDatoSensor\n");
  taskLeerDatoSerial.restartDelayed();

}
 
void leerDato() {
  //bool datoRecibidoDeArduino = false;
  //bool datoEnviadoACoordinador = false;

  if (Serial.available())
  {
    //datoRecibidoDeArduino = true;
    String data = Serial.readStringUntil('\n');
    Serial.printf("datoRecibidoDeArduino = %s\n", data.c_str());

    if(bCoordinadorConectado){
      data = "Dato_"+data;
      mesh.sendSingle(nNodoGatewayId, data);
      Serial.printf("DatoEnviado\n");

      taskEsperarConfirmacionRecibidoGateway.restartDelayed();

      //---------------------SLEEP (Descomentar)------------------------
      taskEsperarTodosLosNodosParaDormir.enable();

    }else{
      taskPedirDatoAArduino.restartDelayed();
    }

  }else{
    taskPedirDatoAArduino.restartDelayed();
  }
}

void EsperarTodosLosNodosParaDormir() {

  if(bTodosLosNodosADormir){
    taskComandoADormir.enable();
  }

}

void ComandoADormir(){
  //---------------------SLEEP (comando para Arduino)------------------------
  //Serial.printf("ADormir\n");

  //------ TEST (Prueba de que los nodos pueden dormir SINCRONIZADOS)
  tLED.setCallback( &LEDOn);
  tLED.enableDelayed(3000 - (mesh.getNodeTime() % (3000 * 1000))/1000);
  //------
}

void ConfirmacionRecibido(){
  if(bDatoRecibidoEnGateway == true){
    taskEsperarTodosLosNodosParaDormir.enable();
  }else{
    taskPedirDatoAArduino.restartDelayed();
  }
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  //Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  if( msg == "soyNodoGateway" && bCoordinadorConectado == false ){
    nNodoGatewayId = from;
    bCoordinadorConectado = true;
    Serial.printf("Mensaje recibido del Coordinador\n");
  }

  if( msg == "TodosLosNodosADormir"){
    bTodosLosNodosADormir = true;
  }

  if( msg == "recibido"){
    bDatoRecibidoEnGateway = true;
  }

}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection,) nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
    Serial.printf("Changed connections %s\n",mesh.subConnectionJson().c_str());
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}


void setup() {

  bCoordinadorConectado = false;
  bTodosLosNodosADormir = false;
  bDatoRecibidoEnGateway = false;

  Serial.begin(115200);

  //------ Debug Sync
  pinMode(2, OUTPUT);
  //------

//mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  taskPedirDatoAArduino.enable();
}

void loop() {
  userScheduler.execute(); // it will run mesh scheduler as well
  mesh.update();
}

//------ Debug Sync

void LEDOn () {
  Serial.printf("LEDOn\n");
	digitalWrite(2 , HIGH);
	tLED.setCallback( &LEDOff);
}

void LEDOff () {
  Serial.printf("LEDOff\n");
	digitalWrite(2 , LOW);
	tLED.setCallback( &LEDOn);
}

//------vsstudioeditadop
// COMMIT2
// COMMIT3
