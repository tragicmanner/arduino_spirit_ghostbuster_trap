#include "YX5300.h"

YX5300::YX5300()
{
	ARDUINO_RX = 5;
	ARDUINO_TX = 6;
}

YX5300::YX5300(int rx, int tx)
{
	ARDUINO_RX = rx;
	ARDUINO_TX = tx;
}

void YX5300::Initialize()
{
	SendCommand(CMD_SEL_DEV, DEV_TF);
}

void YX5300::PlayNextFile()
{
	return;
}
void YX5300::PlayPreviousFile()
{
	return;
}
void YX5300::PlayFile(int file)
{
	return;
}
void YX5300::IncreaseVolume()
{
	return;
}
void YX5300::DecreaseVolume()
{
	return;
}
void YX5300::SetVolume(int volume)
{
	SendCommand(CMD_SET_VOLUME, volume);
}
void YX5300::CycleAudio(int file)
{
	SendCommand(CMD_SINGLE_CYCLE_PLAY, file);
}
void YX5300::Play()
{
	SendCommand(CMD_PLAY, 0);
}
void YX5300::Pause()
{
	SendCommand(CMD_PAUSE, 0);
}
void YX5300::PlayFolderFile(int folder, int file)
{
	return;
}
void YX5300::Stop()
{
	SendCommand(CMD_STOP_PLAY, 0);
}
void YX5300::CycleFolder(int folder)
{
	return;
}
void YX5300::ShufflePlay()
{
	return;
}
void YX5300::PlayFileWithVolume(int file, int volume)
{
	SendCommand(CMD_PLAY_W_VOL, ShiftAndCombine(volume, file));
}

int YX5300::ShiftAndCombine(int one, int two)
{
	return (one<<8)|(two);
}

void YX5300::SendCommand(int8_t command, int16_t dat)
{
	SoftwareSerial mySerial(ARDUINO_RX, ARDUINO_TX);
	mySerial.begin(9600);
	Send_buf[0] = 0x7e; //
	Send_buf[1] = 0xff; //
	Send_buf[2] = 0x06; //
	Send_buf[3] = command; //
	Send_buf[4] = 0x00;//
	Send_buf[5] = (int8_t)(dat >> 8);//datah
	Send_buf[6] = (int8_t)(dat); //datal
	Send_buf[7] = 0xef; //
	for(uint8_t i=0; i<8; i++)//
	{
		mySerial.write(Send_buf[i]);
	}
}