/***********************************************************
//Library for YX5300 MP3 Players
***********************************************************/
#include <SoftwareSerial.h>

class YX5300
{
	private:
		int ARDUINO_RX;//should connect to TX of the Serial MP3 Player module
		int ARDUINO_TX; //connect to RX of the module
		
		int8_t Send_buf[8] = {0};
		
		/************Command byte**************************/
		static const int8_t CMD_NEXT_SONG = 0x01;
		static const int8_t CMD_PREV_SONG = 0x02;
		static const int8_t CMD_PLAY_W_INDEX = 0x03;
		static const int8_t CMD_VOLUME_UP = 0x04;
		static const int8_t CMD_VOLUME_DOWN = 0x05;
		static const int8_t CMD_SET_VOLUME = 0x06;
		static const int8_t CMD_SINGLE_CYCLE_PLAY = 0x08;
		static const int8_t CMD_SEL_DEV = 0x09;
		static const int8_t DEV_TF = 0x02;
		static const int8_t CMD_SLEEP_MODE = 0x0A;
		static const int8_t CMD_WAKE_UP = 0x0B;
		static const int8_t CMD_RESET = 0x0C;
		static const int8_t CMD_PLAY = 0x0D;
		static const int8_t CMD_PAUSE = 0x0E;
		static const int8_t CMD_PLAY_FOLDER_FILE = 0x0F;
		static const int8_t CMD_STOP_PLAY = 0x16;
		static const int8_t CMD_FOLDER_CYCLE = 0x17;
		static const int8_t CMD_SHUFFLE_PLAY = 0x18;
		static const int8_t CMD_SET_SINGLE_CYCLE = 0x19;
		static const int8_t SINGLE_CYCLE_ON = 0x00;
		static const int8_t SINGLE_CYCLE_OFF = 0x01;
		static const int8_t CMD_SET_DAC = 0x1A;
		static const int8_t DAC_ON  = 0x00;
		static const int8_t DAC_OFF = 0x01;
		static const int8_t CMD_PLAY_W_VOL = 0x22;
		
		void SendCommand(int8_t command, int16_t dat);
		int ShiftAndCombine(int one, int two);
		 
	public:
		YX5300();
		YX5300(int rx, int tx);
		
		void Initialize();
		
		void PlayNextFile();
		void PlayPreviousFile();
		void PlayFile(int file);
		void IncreaseVolume();
		void DecreaseVolume();
		void SetVolume(int volume);
		void CycleAudio(int file);
		void Play();
		void Pause();
		void PlayFolderFile(int folder, int file);
		void Stop();
		void CycleFolder(int folder);
		void ShufflePlay();
		void PlayFileWithVolume(int file, int volume);
};
