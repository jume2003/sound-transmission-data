#pragma once
#include <vector>
#include <iostream>
//Wav Header
struct wav_header_t
{
	char chunkID[4]; //"RIFF" = 0x46464952
	unsigned long chunkSize; //28 [+ sizeof(wExtraFormatBytes) + wExtraFormatBytes] + sum(sizeof(chunk.id) + sizeof(chunk.size) + chunk.size)
	char format[4]; //"WAVE" = 0x45564157
	char subchunk1ID[4]; //"fmt " = 0x20746D66
	unsigned long subchunk1Size; //16 [+ sizeof(wExtraFormatBytes) + wExtraFormatBytes]
	unsigned short audioFormat;
	unsigned short numChannels;
	unsigned long sampleRate;
	unsigned long byteRate;
	unsigned short blockAlign;
	unsigned short bitsPerSample;
	//[WORD wExtraFormatBytes;]
	//[Extra format bytes]
};
//Chunks
struct chunk_t
{
	char ID[4]; //"data" = 0x61746164
	unsigned long size;  //Chunk data bytes
};
class WavReader
{
public:
	wav_header_t LoadWav(const char* fileName, std::vector<double> &datas);

};

