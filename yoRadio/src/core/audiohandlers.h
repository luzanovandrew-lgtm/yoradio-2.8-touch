#ifndef AUDIOHANDLERS_H
#define AUDIOHANDLERS_H

//=============================================//
//              Audio handlers                 //
//=============================================//

void audio_bitrate(const char *info);

void audio_info(const char *info) {
  if(player.lockOutput) return;
  if(config.store.audioinfo) telnet.printf("##AUDIO.INFO#: %s\n", info);
  #ifdef USE_NEXTION
    nextion.audioinfo(info);
  #endif
  if (strcasestr(info, "aac") != NULL) { config.setBitrateFormat(BF_AAC); display.putRequest(DBITRATE); }
  if (strcasestr(info, "flac") != NULL) { config.setBitrateFormat(BF_FLAC); display.putRequest(DBITRATE); }
  if (strcasestr(info, "mpeg") != NULL || strcasestr(info, "mp3") != NULL) { config.setBitrateFormat(BF_MP3); display.putRequest(DBITRATE); }
  if (strcasestr(info, "wav") != NULL) { config.setBitrateFormat(BF_WAV); display.putRequest(DBITRATE); }
  if (strcasestr(info, "ogg") != NULL || strcasestr(info, "vorbis") != NULL || strcasestr(info, "opus") != NULL) { config.setBitrateFormat(BF_OGG); display.putRequest(DBITRATE); }
  if (strstr(info, "skip metadata") != NULL) config.setTitle(config.station.name);
  if (strstr(info, "Account already in use") != NULL || strstr(info, "HTTP/1.0 401") != NULL) {
    player.setError(info);
    
  }
  char* ici; char b[20]={0};
  if ((ici = strstr(info, "BitRate: ")) != NULL) {
    strlcpy(b, ici + 9, 50);
    audio_bitrate(b);
  }
  if (strstr(info, "stream ready") != NULL) player.resumeFileIfNeeded();
}

void audio_bitrate(const char *info)
{
  if(config.store.audioinfo) telnet.printf("%s %s\n", "##AUDIO.BITRATE#:", info);
  char* end = nullptr;
  const long bitrateBps = strtol(info, &end, 10);
  if(end == info || bitrateBps <= 0) return;
  const long bitrateKbps = (bitrateBps + 500) / 1000;
  if(config.station.bitrate > 0) return;
  config.station.bitrate = static_cast<uint16_t>(bitrateKbps > 999 ? 999 : bitrateKbps);
  display.putRequest(DBITRATE);
  #ifdef USE_NEXTION
    nextion.bitrate(config.station.bitrate);
  #endif
  netserver.requestOnChange(BITRATE, 0);
}

bool printable(const char *info) {
  if(L10N_LANGUAGE!=RU) return true;
  bool p = true;
  for (int c = 0; c < strlen(info); c++)
  {
    if ((uint8_t)info[c] > 0x7e || (uint8_t)info[c] < 0x20) p = false;
  }
  if (!p) p = (uint8_t)info[0] >= 0xC2 && (uint8_t)info[1] >= 0x80 && (uint8_t)info[1] <= 0xBF;
  return p;
}

void audio_showstation(const char *info) {
  bool p = printable(info) && (strlen(info) > 0);(void)p;
  if(player.remoteStationName){
    config.setStation(p?info:config.station.name);
    display.putRequest(NEWSTATION);
    netserver.requestOnChange(STATION, 0);
  }
}

void audio_showstreamtitle(const char *info) {
  if (strstr(info, "Account already in use") != NULL || strstr(info, "HTTP/1.0 401") != NULL || strstr(info, "HTTP/1.1 401") != NULL) player.setError(info);
  bool p = printable(info) && (strlen(info) > 0);
  #ifdef DEBUG_TITLES
    config.setTitle(DEBUG_TITLES);
  #else
    config.setTitle(p?info:config.station.name);
  #endif
}

void audio_error(const char *info) {
  player.setError(info);
}

void audio_id3artist(const char *info){
  if(printable(info)) config.setStation(info);
  display.putRequest(NEWSTATION);
  netserver.requestOnChange(STATION, 0);
}

void audio_id3album(const char *info){
  if(player.lockOutput) return;
  if(printable(info)){
    if(strlen(config.station.title)==0){
      config.setTitle(info);
    }else{
      char tmp[BUFLEN+3];
      snprintf(tmp, BUFLEN+3, "%s - %s", config.station.title, info);
      config.setTitle(tmp);
    }
  }
}

void audio_id3title(const char *info){
  audio_id3album(info);
}

void audio_beginSDread(){
  config.setTitle("");
}

void audio_id3data(const char *info){  //id3 metadata
    if(player.lockOutput) return;
    telnet.printf("##AUDIO.ID3#: %s\n", info);
}

void audio_eof_mp3(const char *info){  //end of file
    config.sdResumePos = 0;
    player.next();
}

void audio_eof_stream(const char *info){
  player.sendCommand({PR_STOP, 0});
  if(!player.resumeAfterUrl) return;
  if (config.getMode()==PM_WEB){
    player.sendCommand({PR_PLAY, config.lastStation()});
  }else{
    player.setResumeFilePos( config.sdResumePos==0?0:config.sdResumePos-player.sd_min);
    player.sendCommand({PR_PLAY, config.lastStation()});
  }
}

void audio_progress(uint32_t startpos, uint32_t endpos){
  player.sd_min = startpos;
  player.sd_max = endpos;
  netserver.requestOnChange(SDLEN, 0);
}

#if I2S_DOUT!=255 || I2S_INTERNAL
void my_audio_info(Audio::msg_t message) {
  const char *info = message.msg ? message.msg : "";
  switch(message.e) {
    case Audio::evt_info:           audio_info(info); break;
    case Audio::evt_id3data:        audio_id3data(info); break;
    case Audio::evt_eof:
      if(player.resumeAfterUrl || config.getMode()==PM_WEB) audio_eof_stream(info);
      else audio_eof_mp3(info);
      break;
    case Audio::evt_name:           audio_showstation(info); break;
    case Audio::evt_streamtitle:    audio_showstreamtitle(info); break;
    case Audio::evt_bitrate:        audio_bitrate(info); break;
    case Audio::evt_icydescription:
    case Audio::evt_icyurl:
    case Audio::evt_icylogo:
    case Audio::evt_genre:
    case Audio::evt_lasthost:
    case Audio::evt_image:
    case Audio::evt_lyrics:
    case Audio::evt_log:
      if(config.store.audioinfo) telnet.printf("##AUDIO.%s#: %s\n", message.s ? message.s : "INFO", info);
      break;
  }
}
#endif

#endif
