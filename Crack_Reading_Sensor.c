
//본  코드는 주기적으로 가변저항값을 받아 웹사이트 https://console.particle.io 에 업로드 하는 코드입니다.

#include "Particle.h"

SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(SEMI_AUTOMATIC);

Serial1LogHandler logHandler(115200);

enum State {                                                            //본 과정는 총 9단계로 이루어집니다. 데이터 송수신이 필요한 단계마다 인터넷 연결을 확인 및 시도할 것 입니다.
    STATE_WAIT_CONNECTED = 0,                                               //인터넷 연결 대기
    STATE_READ_SENSOR,                                                      //가변저항값 확인
    STATE_PUBLISH,                                                          //변수 송신 시도
    STATE_PUBLISH_WAIT,                                                     //송신 확인
    STATE_SLEEP,                                                            //슬립 모드
    STATE_DISCONNECT_WAIT,                                                  //인터넷 연결 재확인 및 시도
    STATE_CONNECT_WAIT,                                                     //클라우드 연결 확인 및 시도
    STATE_CLOUD_WAIT,                                                       //펌웨어 업데이트 확인
    STATE_FIRMWARE_UPDATE,                                                  //펌웨어 업데이트
    };

const std::chrono::milliseconds connectMaxTime = 6min;                  //5분 이상으로 설정해주세요. 인터넷 연결 최대 대기 시간입니다. 본 시간 경과 시 슬립모드로 진입합니다. 
const std::chrono::milliseconds cloudMinTime = 10s;                     //클라우드에서 업데이트를 받아오기 위해 10초 이상을 권장합니다. 클라우드 최소 연결 시간입니다. 
const std::chrono::milliseconds publishMaxTime = 3min;                  //80초 이상으로 설정해주세요. 업로드 최대 대기 시간입니다. 업로드 시도로부터 본 시간 경과 후 업로드 실패시 슬립모드로 진입합니다.  
const std::chrono::milliseconds firmwareUpdateMaxTime = 5min;           //코드 업데이트 최대 대기 시간입니다. 업데이트 시작 후 본 시간이내에 완료되지 않으면 슬립모드로 진입합니다.

const std::chrono::seconds sleepTime = 1min;                            // 15분 미만으로 설정해주세요.슬립모드 주기입니다.
const std::chrono::seconds firmwareUpdateCheckTime = 1min;              //펌웨어 업데이트 확인 주기입니다. 업데이트를 바라지 않으면 0분으로 설정해주세요.
const std::chrono::seconds diagnosticPublishTime = 3min;                //보드 진단 주기입니다. 데이터 사용량을 줄이기 위해 주기를 24시간 이상으로도 설정할 수 있습니다. 펌웨어 업데이트시 설정된 주기와 별개로 진단 정보도 함께 전송될 것 입니다.

State state = STATE_WAIT_CONNECTED;
unsigned long stateTime=0; 
bool firmwareUpdateInProgress = false;
long lastFirmwareUpdateCheck = 0;
long lastDiagnosticsPublish = 0;
char publishData[256];
particle::Future<bool> publishFuture;



void setup() {
    System.on(firmware_update, firmwareUpdateHandler);
    Cellular.on();
    Particle.connect();
    stateTime = millis();
}

void loop() {
    switch(state) {
        case STATE_WAIT_CONNECTED:                                                                          //인터넷 연결 대기
 
            if (Particle.connected()) {                                                                         //인터넷 연결 성공시 다음 단계로 진행
                Log.info("connected to the cloud in %lu ms", millis() - stateTime);
                state = STATE_READ_SENSOR; 
                stateTime = millis(); 
            }
            else
            if (millis() - stateTime >= connectMaxTime.count()) {                                               //최대 인터넷 연결 대기시간 경과시 슬립모드로 진입
                Log.info("failed to connect, going to sleep");
                state = STATE_SLEEP;
            }
            break;
        
        case STATE_READ_SENSOR:                                                                             //가변저항값 확인
            {
                int crack = analogRead(A0);
                snprintf(publishData, sizeof(publishData), "{\"Crack_value\" : %d}", crack);                   //변수에 가변저항값 저장 및 다음 단계로 진행
            }
            state = STATE_PUBLISH;
            stateTime = millis();
            break;

        case STATE_PUBLISH:                                                                                 //변수 송신 시도
            if (Particle.connected()) {
                publishFuture = Particle.publish("sensorValue", publishData, PRIVATE | WITH_ACK);              //변수 송신 시도 후 다음 단계로 진행
                state = STATE_PUBLISH_WAIT;
                stateTime = millis();

            }
            else
            if (millis() - stateTime >= connectMaxTime.count()) {                                               //최대 인터넷 연결 대기시간 경과시 슬립모드로 진입
                Log.info("failed to connect, going to sleep and discarding sample");                            
                state = STATE_SLEEP;
            }
            break;
            
        case STATE_PUBLISH_WAIT:                                                                            //송신 확인
             if (publishFuture.isDone()) {
                 if (publishFuture.isSucceeded()) {                                                             //송신 성공시 다음 단계로 진행
                    Log.info("successfully published %s", publishData);
                    state = STATE_SLEEP;
                }
                else {                                                                                          //송신 실패시 슬립모드로 진입
                    Log.info("failed to publish, will discard sample");
                    state = STATE_SLEEP;
                }
            }
            else 
            if (millis() - stateTime >= publishMaxTime.count()) {                                               //최대 업로드 대기시간 내에 송신 미완료시 슬립모드로 진입
                Log.info("failed to publish, timed out, will discard sample");
                state = STATE_SLEEP;
            }
            break;
            
        case STATE_SLEEP:                                                                                   //슬립 모드
            if (Time.isValid() && Particle.connected()) {
                if (lastFirmwareUpdateCheck && firmwareUpdateCheckTime.count() &&                                   //설정한 업데이트 주기마다 다음 단계로 진입 
                    Time.now() > (lastFirmwareUpdateCheck + firmwareUpdateCheckTime.count())) {
                    Log.info("starting firmware update check");
                    Particle.disconnect();
                    state = STATE_DISCONNECT_WAIT;
                    break;
                }
                if (lastDiagnosticsPublish && diagnosticPublishTime.count() &&                                      //설정한 진단 주기마다 진단정보 전송
                    Time.now() > (lastDiagnosticsPublish + diagnosticPublishTime.count())) {
                    Log.info("publishing device vitals");
                    Particle.publishVitals(0);
                    lastDiagnosticsPublish = Time.now();
                }
            }
            System.sleep(WKP, RISING, sleepTime, SLEEP_NETWORK_STANDBY);                                        //업데이트 및 진단 불필요시 설정한 시간동안 sleep & restart
            Log.info("woke from sleep");
            state = STATE_READ_SENSOR;
            stateTime = millis();
            break;
            
        case STATE_DISCONNECT_WAIT:                                                                         //인터넷 연결 재확인 및 시도
            if (!Particle.connected()) {
                Log.info("reconnecting to the cloud");                                                          //인터넷 미연결시 연결 시도 후 다음단계로 진행
                Particle.connect();
                state = STATE_CONNECT_WAIT;
                stateTime = millis(); 
        
        case STATE_CONNECT_WAIT:                                                                            //클라우드 연결 확인 및 시도
            if (Particle.connected()) {
                Log.info("connected to the cloud in %lu ms, checking for updates", millis() - stateTime);
                state = STATE_CLOUD_WAIT; 
                stateTime = millis(); 
                lastFirmwareUpdateCheck = lastDiagnosticsPublish = Time.now();                                  //인터텟 연결 성공시 최종 업데이트 확인 시간 저장 후 다음 단계로 진행
            }
            else
            if (millis() - stateTime >= connectMaxTime.count()) {                                               //인터넷 연결 실패 시 슬립모드로 진입
                Log.info("failed to connect, going to sleep");
                state = STATE_SLEEP;
            }
            break;
            
        case STATE_CLOUD_WAIT:                                                                               //펌웨어 업데이트 확인
            if (firmwareUpdateInProgress) {
                Log.info("firmware update detected");                                                           //업데이트 할 코드 발견시 다음 단계로 진행
                state = STATE_FIRMWARE_UPDATE;
                stateTime = millis();
            }
            else
            if (millis() - stateTime >= cloudMinTime.count()) {                                                 //클라우드 최소 연결 시간동안 업데이트 할 코드 미발견시 슬립모드로 진입
                Log.info("no update detected, going to sleep");
                state = STATE_SLEEP;
            }
            break;
        
        case STATE_FIRMWARE_UPDATE:                                                                         //펌웨어 업데이트
            if (!firmwareUpdateInProgress) {
                Log.info("firmware update completed");                                                           //업데이트 완료시 슬립모드로 진입
                state = STATE_SLEEP;
            }
            else
            if (millis() - stateTime >= firmwareUpdateMaxTime.count()) {
                Log.info("firmware update timed out");                                                           //설정한 업데이트 최대시간 내에 업데이트 실패시 슬립모드로 진입
                state = STATE_SLEEP;
            }
            break;
    }
    
    if (lastFirmwareUpdateCheck == 0 && Time.isValid()) {
        lastFirmwareUpdateCheck = Time.now();
    }
    if (lastDiagnosticsPublish == 0 && Time.isValid()) {
        lastDiagnosticsPublish = Time.now();
    }
    }
}

void firmwareUpdateHandler(system_event_t event, int param) {
    switch(param) {
        case firmware_update_begin:
            firmwareUpdateInProgress = true;
            break;

        case firmware_update_complete:
        case firmware_update_failed:
            firmwareUpdateInProgress = false;
            break;
    }
}
