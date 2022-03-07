#include "WavReader.h"
#include "windows.h"
#include "stdio.h"


wav_header_t WavReader::LoadWav(const char* fileName, std::vector<double> &datas)
{
	FILE *fin = NULL;
	fopen_s(&fin,fileName, "rb");
	//Read WAV header
	wav_header_t header;
	fread(&header, sizeof(header), 1, fin);

	//Print WAV header
	//printf("WAV File Header read:\n");
	//printf("File Type: %s\n", header.chunkID);
	//printf("File Size: %ld\n", header.chunkSize);
	//printf("WAV Marker: %s\n", header.format);
	//printf("Format Name: %s\n", header.subchunk1ID);
	//printf("Format Length: %ld\n", header.subchunk1Size);
	//printf("Format Type: %hd\n", header.audioFormat);
	//printf("Number of Channels: %hd\n", header.numChannels);
	//printf("Sample Rate: %ld\n", header.sampleRate);
	//printf("Sample Rate * Bits/Sample * Channels / 8: %ld\n", header.byteRate);
	//printf("Bits per Sample * Channels / 8.1: %hd\n", header.blockAlign);
	//printf("Bits per Sample: %hd\n", header.bitsPerSample);

	//skip wExtraFormatBytes & extra format bytes
	//fseek(f, header.chunkSize - 16, SEEK_CUR);

	//Reading file
	chunk_t chunk;
	//printf("id\t" "size\n");
	//go to data chunk
	while (true)
	{
		fread(&chunk, sizeof(chunk), 1, fin);
		//printf("%c%c%c%c\t" "%li\n", chunk.ID[0], chunk.ID[1], chunk.ID[2], chunk.ID[3], chunk.size);
		if (*(unsigned int *)&chunk.ID == 0x61746164)
			break;
		//skip chunk data bytes
		fseek(fin, chunk.size, SEEK_CUR);
	}

	//Number of samples
	int sample_size = header.bitsPerSample / 8;
	int samples_count = chunk.size * 8 / header.bitsPerSample;
	//printf("Samples count = %i\n", samples_count);

	short int *value = new short int[samples_count];
	memset(value, 0, sizeof(short int) * samples_count);
	fread(value, sample_size, samples_count, fin);
	//Reading data
	bool is_find_not_zero = false;
	for (int i = 0; i < samples_count; i+= header.numChannels)
	{
		if (!is_find_not_zero)is_find_not_zero = value[i] != 0;
		if(is_find_not_zero)
		datas.push_back(value[i]);
	}
	free(value);
	fclose(fin);
	return header;
}