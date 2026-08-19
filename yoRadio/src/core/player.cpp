#include "options.h"
#include "player.h"
#include "config.h"
#include "telnet.h"
#include "display.h"
#include "sdmanager.h"
#include "netserver.h"
#include "timekeeper.h"
#include "../displays/tools/l10n.h"
#include "../pluginsManager/pluginsManager.h"
#ifdef USE_NEXTION
#include "../displays/nextion.h"
#endif
#ifdef USE_ES8311
#include "../ES8311_Audio/es8311.h"
#endif
Player player;
QueueHandle_t playerQueue;

#if VS1053_CS!=255 && !I2S_INTERNAL
  #if VS_HSPI
    Player::Player(): Audio(VS1053_CS, VS1053_DCS, VS1053_DREQ, &SPI2) {}
  #else
    Player::Player(): Audio(VS1053_CS, VS1053_DCS, VS1053_DREQ, &SPI) {}
  #endif
  void ResetChip(){
    pinMode(VS1053_RST, OUTPUT);
    digitalWrite(VS1053_RST, LOW);
    delay(30);
    digitalWrite(VS1053_RST, HIGH);
    delay(100);
  }
#else
  #if !I2S_INTERNAL
    Player::Player() {}
  #else
    Player::Player(): Audio(true, I2S_DAC_CHANNEL_BOTH_EN)  {}
  #endif
#endif


void Player::init() {
  Serial.print("##[BOOT]#\tplayer.init\t");
  playerQueue=NULL;
  _resumeFilePos = 0;
  _audioInfoTicks = 0;
  _bitrateUpdateTicks = 0;
  _hasError=false;
  playerQueue = xQueueCreate( 5, sizeof( playerRequestParams_t ) );
  setOutputPins(false);
  applyVUSettings();
  delay(50);
#ifdef MQTT_ROOT_TOPIC
  memset(burl, 0, MQTT_BURL_SIZE);
#endif
  if(MUTE_PIN!=255) pinMode(MUTE_PIN, OUTPUT);
  #if I2S_DOUT!=255
    #if !I2S_INTERNAL
      setAudioTaskCore(0);
      settings.SPECTRUM = false;
      setOutputSampleRate(Audio::SR_ORIGIN);
      setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT, I2S_MCLK);
    #endif
  #else
    SPI.begin();
    if(VS1053_RST>0) ResetChip();
    begin();
  #endif
#ifdef USE_ES8311
  if (es.begin(ES8311_I2C_SDA, ES8311_I2C_SCL, 400000UL)) {
    es.setVolume(0);
  }
#endif
  setBalance(config.store.balance);
  setTone(config.store.bass, config.store.middle, config.store.trebble);
#ifdef USE_ES8311
  setVolume(VOLUME_SCALE);
#else
  setVolume(0);
#endif
  _status = STOPPED;
  _volTimer=false;
  //randomSeed(analogRead(0));
  #if PLAYER_FORCE_MONO
    forceMono(true);
  #endif
  _loadVol(config.store.volume);
#ifdef USE_ES8311
  es.setVolume(map(volToI2S(config.store.volume), 0, ES8311_MAX_I2S, 0, 100));
#endif
  setConnectionTimeout(CONNECTION_TIMEOUT, CONNECTION_TIMEOUT_SSL);
  Serial.println("done");
}

void Player::sendCommand(playerRequestParams_t request){
  if(playerQueue==NULL) return;
  xQueueSend(playerQueue, &request, PLQ_SEND_DELAY);
}

void Player::resetQueue(){
  if(playerQueue!=NULL) xQueueReset(playerQueue);
}

void Player::stopInfo() {
  config.setSmartStart(0);
  netserver.requestOnChange(MODE, 0);
}

void Player::setError(){
  _hasError=true;
  config.setTitle(config.tmpBuf);
  telnet.printf("##ERROR#:\t%s\n", config.tmpBuf);
}

void Player::setError(const char *e){
  strlcpy(config.tmpBuf, e, sizeof(config.tmpBuf));
  setError();
}

void Player::_stop(bool alreadyStopped){
  log_i("%s called", __func__);
  if(config.getMode()==PM_SDCARD && !alreadyStopped) config.sdResumePos = player.getAudioFilePosition();
  _status = STOPPED;
  setOutputPins(false);
  if(!_hasError) config.setTitle((display.mode()==LOST || display.mode()==UPDATING)?"":LANG::const_PlStopped);
  config.station.bitrate = 0;
  config.setBitrateFormat(BF_UNKNOWN);
  #ifdef USE_NEXTION
    nextion.bitrate(config.station.bitrate);
  #endif
  if(!alreadyStopped) stopSong();
  netserver.requestOnChange(BITRATE, 0);
  display.putRequest(DBITRATE);
  display.putRequest(PSTOP);
  //setDefaults();
  //if(!alreadyStopped) stopSong();
  if(!lockOutput) stopInfo();
  if (player_on_stop_play) player_on_stop_play();
  pm.on_stop_play();
}

void Player::initHeaders(const char *file) {
  (void)file; // Headers are parsed asynchronously by ESP32-audioI2S 3.x.
}

void Player::applyVUSettings() {
  #if I2S_DOUT!=255 || I2S_INTERNAL
    Audio::setVUSettings(config.store.vuGain, config.store.vuWindowMs, config.store.vuAttackMs,
                         config.store.vuReleaseMs, config.store.vuPeakHoldMs, config.store.vuPeakReleaseMs);
  #endif
}
void resetPlayer(){
  if(!config.store.watchdog) return;
  player.resetQueue();
  player.sendCommand({PR_STOP, 0});
  player.loop();
}

#ifndef PL_QUEUE_TICKS
  #define PL_QUEUE_TICKS 0
#endif
#ifndef PL_QUEUE_TICKS_ST
  #define PL_QUEUE_TICKS_ST 15
#endif
void Player::loop() {
  if(playerQueue==NULL) return;
  playerRequestParams_t requestP;
  if(xQueueReceive(playerQueue, &requestP, isRunning()?PL_QUEUE_TICKS:PL_QUEUE_TICKS_ST)){
    switch (requestP.type){
      case PR_STOP: _stop(); break;
      case PR_PLAY: {
        if (requestP.payload>0) {
          config.setLastStation((uint16_t)requestP.payload);
        }
        _play((uint16_t)abs(requestP.payload)); 
        if (player_on_station_change) player_on_station_change(); 
        pm.on_station_change();
        break;
      }
      case PR_TOGGLE: {
        toggle();
        break;
      }
      case PR_VOL: {
        config.setVolume(requestP.payload);
#ifdef USE_ES8311
        Audio::setVolume(VOLUME_SCALE);
        es.setVolume(map(volToI2S(requestP.payload), 0, ES8311_MAX_I2S, 0, 100));
#else
        Audio::setVolume(volToI2S(requestP.payload));
#endif
        break;
      }
      #ifdef USE_SD
      case PR_CHECKSD: {
        if(config.getMode()==PM_SDCARD){
          if(!sdman.cardPresent()){
            sdman.stop();
            config.changeMode(PM_WEB);
          }
        }
        break;
      }
      #endif
      case PR_VUTONUS: {
        if(config.vuThreshold>10) config.vuThreshold -=10;
        break;
      }
      case PR_BURL: {
      #ifdef MQTT_ROOT_TOPIC
        if(strlen(burl)>0){
          browseUrl();
        }
      #endif
        break;
      }
          
      default: break;
    }
  }
  Audio::loop();
  _syncAudioInfo();
  if(!isRunning() && _status==PLAYING) _stop(true);
  if(_volTimer){
    if((millis()-_volTicks)>3000){
      config.saveVolume();
      _volTimer=false;
    }
  }
  /*
#ifdef MQTT_ROOT_TOPIC
  if(strlen(burl)>0){
    browseUrl();
  }
#endif*/
}

void Player::setOutputPins(bool isPlaying) {
  if(REAL_LEDBUILTIN!=255) digitalWrite(REAL_LEDBUILTIN, LED_INVERT?!isPlaying:isPlaying);
  bool _ml = MUTE_LOCK?!MUTE_VAL:(isPlaying?!MUTE_VAL:MUTE_VAL);
  if(MUTE_PIN!=255) digitalWrite(MUTE_PIN, _ml);
}

void Player::_play(uint16_t stationId) {
  log_i("%s called, stationId=%d", __func__, stationId);
  _hasError=false;
  _status = STOPPED;
  _bitrateUpdateTicks = 0;
  setOutputPins(false);
  remoteStationName = false;
  
  if(!config.prepareForPlaying(stationId)) return;
  _loadVol(config.store.volume);
  
  bool isConnected = false;
  if(config.getMode()==PM_SDCARD && SDC_CS!=255){
    if(config.sdResumePos > 0) _resumeFilePos = config.sdResumePos;
    isConnected=connecttoFS(*sdman.filesystem(), config.station.url);
  }else {
    config.saveValue(&config.store.play_mode, static_cast<uint8_t>(PM_WEB));
  }
  connproc = false;
  if(config.getMode()==PM_WEB) isConnected=connecttohost(config.station.url);
  connproc = true;
  if(isConnected){
    _status = PLAYING;
    config.configPostPlaying(stationId);
    setOutputPins(true);
    if (player_on_start_play) player_on_start_play();
    pm.on_start_play();
  }else{
    telnet.printf("##ERROR#:\tError connecting to %.128s\n", config.station.url);
    snprintf(config.tmpBuf, sizeof(config.tmpBuf), "Error connecting to %.128s", config.station.url); setError();
    _stop(true);
  };
}

#ifdef MQTT_ROOT_TOPIC
void Player::browseUrl(){
  _hasError=false;
  remoteStationName = true;
  config.setDspOn(1);
  resumeAfterUrl = _status==PLAYING;
  display.putRequest(PSTOP);
  setOutputPins(false);
  config.setTitle(LANG::const_PlConnect);
  if (connecttohost(burl)){
    _status = PLAYING;
    config.setTitle("");
    netserver.requestOnChange(MODE, 0);
    setOutputPins(true);
    display.putRequest(PSTART);
    if (player_on_start_play) player_on_start_play();
    pm.on_start_play();
  }else{
    telnet.printf("##ERROR#:\tError connecting to %.128s\n", burl);
    snprintf(config.tmpBuf, sizeof(config.tmpBuf), "Error connecting to %.128s", burl); setError();
    _stop(true);
  }
  //memset(burl, 0, MQTT_BURL_SIZE);
}
#endif

void Player::prev() {
  uint16_t lastStation = config.lastStation();
  if(config.getMode()==PM_WEB || !config.store.sdsnuffle){
    if (lastStation == 1) config.lastStation(config.playlistLength()); else config.lastStation(lastStation-1);
  }
  sendCommand({PR_PLAY, config.lastStation()});
}

void Player::next() {
  uint16_t lastStation = config.lastStation();
  if(config.getMode()==PM_WEB || !config.store.sdsnuffle){
    if (lastStation == config.playlistLength()) config.lastStation(1); else config.lastStation(lastStation+1);
  }else{
    config.lastStation(random(1, config.playlistLength()));
  }
  sendCommand({PR_PLAY, config.lastStation()});
}

void Player::toggle() {
  if (_status == PLAYING) {
    sendCommand({PR_STOP, 0});
  } else {
    sendCommand({PR_PLAY, config.lastStation()});
  }
}

void Player::stepVol(bool up) {
  if (up) {
    if (config.store.volume <= 254 - config.store.volsteps) {
      setVol(config.store.volume + config.store.volsteps);
    }else{
      setVol(254);
    }
  } else {
    if (config.store.volume >= config.store.volsteps) {
      setVol(config.store.volume - config.store.volsteps);
    }else{
      setVol(0);
    }
  }
}

uint8_t Player::volToI2S(uint8_t volume) {
#ifdef USE_ES8311
  int maxIn = 254 - config.station.ovol * 3;
  if (maxIn < 1) maxIn = 1;
  float vnorm = (float)volume / (float)maxIn;
  if (vnorm < 0.0f) vnorm = 0.0f;
  if (vnorm > 1.0f) vnorm = 1.0f;
  float vout = powf(vnorm, 0.5f);
  int vol = (int)(vout * (float)ES8311_MAX_I2S + 0.5f);
#else
  int vol = map(volume, 0, 254 - config.station.ovol * 3 , 0, 254);
#endif
  if (vol > 254) vol = 254;
  if (vol < 0) vol = 0;
  return vol;
}

void Player::_syncAudioInfo() {
  #if I2S_DOUT!=255 || I2S_INTERNAL
    if(!isRunning() || millis() - _audioInfoTicks < 250) return;
    _audioInfoTicks = millis();

    BitrateFormat format = BF_UNKNOWN;
    const char* codec = Audio::getCodecname();
    if(codec != nullptr) {
      if(strcasecmp(codec, "MP3") == 0) format = BF_MP3;
      else if(strcasecmp(codec, "AAC") == 0 || strcasecmp(codec, "M4A") == 0) format = BF_AAC;
      else if(strcasecmp(codec, "FLAC") == 0) format = BF_FLAC;
      else if(strcasecmp(codec, "OPUS") == 0 || strcasecmp(codec, "VORBIS") == 0 || strcasecmp(codec, "OGG") == 0) format = BF_OGG;
      else if(strcasecmp(codec, "WAV") == 0) format = BF_WAV;
    }

    const uint32_t now = millis();
    const uint32_t bitrateBps = Audio::getBitRate();
    const uint16_t bitrateKbps = static_cast<uint16_t>(std::min<uint32_t>(999, (bitrateBps + 500) / 1000));
    bool changed = false;
    if(format != BF_UNKNOWN && config.configFmt != format) {
      config.setBitrateFormat(format);
      changed = true;
    }
    if(bitrateKbps > 0 && (config.station.bitrate == 0 || now - _bitrateUpdateTicks >= 5000)) {
      _bitrateUpdateTicks = now;
      if(config.station.bitrate != bitrateKbps) {
        config.station.bitrate = bitrateKbps;
        changed = true;
      }
    }
    if(changed) {
      display.putRequest(DBITRATE);
      netserver.requestOnChange(BITRATE, 0);
      #ifdef USE_NEXTION
        nextion.bitrate(config.station.bitrate);
      #endif
    }
  #endif
}

void Player::resumeFileIfNeeded() {
  if(_resumeFilePos == 0) return;
  if(setAudioFilePosition(_resumeFilePos)) _resumeFilePos = 0;
}

void Player::_loadVol(uint8_t volume) {
  setVolume(volToI2S(volume));
}

void Player::setVol(uint8_t volume) {
  _volTicks = millis();
  _volTimer = true;
  player.sendCommand({PR_VOL, volume});
}
