#include "windows.h"
#include "stdio.h"
#include "math.h"
#include <vector>
#include <iostream>
#include "WavReader.h"
#include "fftw3.h"
#define PI 3.141592653589793

double *MakeRange(double num1, double num2, double step)
{
	int data_len = int((num2 - num1) / step);
	double *data = (double *)malloc(sizeof(double)*data_len);
	for (int i = 0; i < data_len; i++)
	{
		data[i] = i * step;
	}
	return data;
}

double *MakeLinspace(double num1, double num2, double count)
{
	int data_len = int(num2 - num1);
	double *data = (double *)malloc(sizeof(double)*count);
	double step = data_len / count;

	for (int i = 0; i < count; i++)
	{
		data[i] = i * step;
	}
	return data;
}

//采样率, 振幅, 传输波特率(位 / s)，频率Hz(周期 / s)
double* MakeSinDatac(double framerate, double amplitude, double bitrate, double *frequency,int frequency_size,int *sound_len)
{
	double data_tls = (1.0f / bitrate);//总体数据时长
	int	data_len = int(framerate*data_tls);//数据长度
	double data_uns = data_tls / data_len; //单个数据时长
	double *t = MakeRange(0, data_tls, data_uns);//时域表
	double *sound_data = (double *)malloc(sizeof(double)*data_len);
	memset(sound_data, 0.0, sizeof(double)*data_len);
	for (int i = 0; i < data_len; i++)
	{
		for (int j = 0; j < frequency_size; j++)
		{
			double w = (2 * PI)*frequency[j] / 1.0f;//周期/s
			sound_data[i] = sound_data[i] + sin(t[i] * w);
		}
	}
	for (int i = 0; i < data_len; i++)
	sound_data[i] = sound_data[i] * amplitude;

	free(t);
	*sound_len = data_len;
	return sound_data;
}

//复数的模：将复数的实部与虚部的平方和的正的平方根的值，记作∣z∣.
//即对于复数z = a + bi，它的模：∣z∣ = √（a ^ 2 + b ^ 2)
void ComplexAbs(fftw_complex* data, int size)
{
	for (int i = 0; i < size; i++)
	{
		data[i][0] = sqrt(data[i][0] * data[i][0] + data[i][1] * data[i][1]);
	}
}
//复数的模：将复数的实部与虚部的平方和的正的平方根的值，记作∣z∣.
//即对于复数z = a + bi，它的模：∣z∣ = √（a ^ 2 + b ^ 2)
void ComplexDiv(fftw_complex* data,double n, int size)
{
	for (int i = 0; i < size; i++)
	{
		data[i][0] = data[i][0] / n;
		data[i][1] = data[i][1] / n;
	}
}
//
int GetMaxIndex(fftw_complex* data, int size)
{
	int index = 0;
	double maxd = 0;
	for (int i = 0; i < size; i++)
	{
		if (data[i][0] > maxd)
		{
			maxd = data[i][0];
			index = i;
		}
	}
	return index;
}

std::vector<double> MakeSinData(double framerate, double amplitude, double bitrate, std::vector<double> frequency)
{
	std::vector<double> sound_data;
	double data_tls = (1.0f / bitrate);//总体数据时长
	int	data_len = int(framerate*data_tls);//数据长度
	double data_uns = data_tls / data_len; //单个数据时长
	double *t = MakeRange(0, data_tls, data_uns);//时域表
	for (int i = 0; i < data_len; i++)
	{
		sound_data.push_back(0.0f);
		for (int j = 0; j < frequency.size(); j++)
		{
			double w = (2 * PI)*frequency[j] / 1.0f;//周期/s
			sound_data[i] = sound_data[i] + sin(t[i] * w);
		}
	}
	for (int i = 0; i < data_len; i++)
		sound_data[i] = sound_data[i] * amplitude;
	free(t);
	return sound_data;
}

int GetNumberByHz(std::vector<double> frequencys, int hz)
{
	int index = 0;
	if (hz < frequencys[0] * 0.7)return -1;
	double mindic = abs(hz - frequencys[0]);
	for (int i = 0; i < frequencys.size(); i++)
	{
		double dic = abs(hz - frequencys[i]);
		if (mindic > dic)
		{
			mindic = dic;
			index = i;
		}

	}
	return index;
}

int GetNumberByFft(std::vector<double> sound_data, double framesra, std::vector<double> frequencys, int fft_size)
{
	int number = 0;
	std::vector<double> xs(sound_data.begin(), sound_data.begin() + fft_size);
	fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * xs.size());
	fftw_plan plan = fftw_plan_dft_r2c_1d(xs.size(), xs.data(), out, FFTW_ESTIMATE);
	fftw_execute(plan);
	fftw_destroy_plan(plan);
	int out_size = int(fft_size / 2 + 1);
	ComplexDiv(out, fft_size, out_size);
	ComplexAbs(out, out_size);
	auto freqs = MakeLinspace(0, int(framesra / 2), out_size);
	int index = GetMaxIndex(out, out_size);
	number = GetNumberByHz(frequencys, freqs[index]);
	free(freqs);
	fftw_free(out);
	return number;
}

std::vector<int> DeCodeSound(std::vector<double> sound_data,double framesra,std::vector<double> frequencys, int bitrate)
{
	std::vector<int> number_list;
	double data_tls = (1.0 / bitrate);// #总体数据时长
	int data_len = int(framesra*data_tls);// #数据长度
	int fft_size = int(pow(2, log2(data_len)));
	//#寻找开始符号 10
	int data_index = 0;
	while (true)
	{
		int data_step = 64;//#(data_len / 2)
		int data_x = int(data_index*data_step);
		if (data_x + fft_size >= sound_data.size())break;
		std::vector<double> fft_data(sound_data.begin() + data_x, sound_data.begin() + data_x + fft_size);
		if (GetNumberByFft(fft_data, framesra, frequencys, fft_size) == 10)
		{
			while (true)
			{
				data_x = int((data_index + 1)*data_step);
				if (data_x + fft_size > sound_data.size())break;
				std::vector<double> ffft_data(sound_data.begin ()+ data_x, sound_data.begin()+data_x + fft_size);
				int number = GetNumberByFft(ffft_data, framesra, frequencys, fft_size);
				if (number != 10 && number != -1)
				{
					number_list.push_back(number);
					break;
				}
				data_index = data_index + 1;
			}
		}
		data_index = data_index + 1;
	}
	return number_list;
}

std::vector<int> MakeCrcData(std::vector<int> data_str)
{
	std::vector<int> datas;
	int crc_sum = 0;
	int crc_mul = 0;
	for (int i = 0; i < data_str.size(); i++)
	{
		int number = data_str[i];
		crc_sum = crc_sum + number;
		crc_mul = crc_mul ^ number;
	}
	crc_sum = crc_sum % 10;
	crc_mul = crc_mul % 10;
	datas.push_back(crc_sum);
	datas.push_back(crc_mul);
	return datas;
}
	
std::vector <std::vector<int>> DeCodeTransmissionData(std::vector<int> datas, int pack_size)
{
	int index = 0;
	std::vector <std::vector<int>> decode_datas;
	while (true)
	{
		int data_size = datas.size();
		if (index + pack_size > data_size -2)break;
		std::vector<int> data_part(datas.begin() + index, datas.begin() + index + pack_size);
		int data_crc1= datas[index + pack_size];
		int data_crc2 = datas[index + pack_size + 1];
		auto crc = MakeCrcData(data_part);
		if (data_crc1 == crc[0]&& data_crc2 == crc[1])
		{
			decode_datas.push_back(data_part);
			index = index + pack_size + 2;
		}
		else
		{
			index = index + 1;
		}
	}
	return decode_datas;
}
	

void main()
{

	WavReader wavreader;
	std::vector<double> frequencys = { 1000, 1900, 2800, 3700, 4600, 5500, 6400, 7300, 8200, 9100, 5000 };
	std::vector<double> sound_data;
	wav_header_t wavinfo = wavreader.LoadWav("5.wav", sound_data);
	std::vector<int> datas = DeCodeSound(sound_data, wavinfo.sampleRate, frequencys, 128);
	std::vector<std::vector<int>> tdatas = DeCodeTransmissionData(datas, 15);
	for (int i = 0; i < tdatas.size(); i++)
	{
		printf("%d [",i);
		for (int j = 0; j < tdatas[i].size(); j++)
		{
			printf("%d", tdatas[i][j]);
		}
		printf("]\n");
	}
	printf("bbq");
	while (true)Sleep(1000);
}