#include "stdafx.h"
#include "Audio.h"
#include "al.h"
#include "alc.h"
#include <iostream>
#include "DebugNew.h"

namespace ToolKit
{
	// http://www.dunsanyinteractive.com/blogs/oliver/?p=72

	/*
	* Struct that holds the RIFF data of the Wave file.
	* The RIFF data is the meta data information that holds,
	* the ID, size and format of the wave file
	*/
	struct RIFF_Header {
		char chunkID[4];
		long chunkSize;//size not including chunkSize or chunkID
		char format[4];
	};

	/*
	* Struct to hold fmt subchunk data for WAVE files.
	*/
	struct WAVE_Format {
		char subChunkID[4];
		long subChunkSize;
		short audioFormat;
		short numChannels;
		long sampleRate;
		long byteRate;
		short blockAlign;
		short bitsPerSample;
	};

	/*
	* Struct to hold the data of the wave file
	*/
	struct WAVE_Data {
		char subChunkID[4]; //should contain the word data
		long subChunk2Size; //Stores the size of the data block
	};

	/*
	* Load wave file function. No need for ALUT with this
	*/
	bool loadWavFile(const std::string filename, ALuint* buffer,
		ALsizei* size, ALsizei* frequency,
		ALenum* format) {
		//Local Declarations
		FILE* soundFile = NULL;
		WAVE_Format wave_format;
		RIFF_Header riff_header;
		WAVE_Data wave_data;
		unsigned char* data;

		try {
			fopen_s(&soundFile, filename.c_str(), "rb");
			if (!soundFile)
				throw (filename);

			// Read in the first chunk into the struct
			fread(&riff_header, sizeof(RIFF_Header), 1, soundFile);

			//check for RIFF and WAVE tag in memeory
			if ((riff_header.chunkID[0] != 'R' ||
				riff_header.chunkID[1] != 'I' ||
				riff_header.chunkID[2] != 'F' ||
				riff_header.chunkID[3] != 'F') ||
				(riff_header.format[0] != 'W' ||
					riff_header.format[1] != 'A' ||
					riff_header.format[2] != 'V' ||
					riff_header.format[3] != 'E'))
				throw ("Invalid RIFF or WAVE Header");

			//Read in the 2nd chunk for the wave info
			fread(&wave_format, sizeof(WAVE_Format), 1, soundFile);
			//check for fmt tag in memory
			if (wave_format.subChunkID[0] != 'f' ||
				wave_format.subChunkID[1] != 'm' ||
				wave_format.subChunkID[2] != 't' ||
				wave_format.subChunkID[3] != ' ')
				throw ("Invalid Wave Format");

			//check for extra parameters;
			if (wave_format.subChunkSize > 16)
				fseek(soundFile, sizeof(short), SEEK_CUR);

			//Read in the the last byte of data before the sound file
			fread(&wave_data, sizeof(WAVE_Data), 1, soundFile);
			//check for data tag in memory
			if (wave_data.subChunkID[0] != 'd' ||
				wave_data.subChunkID[1] != 'a' ||
				wave_data.subChunkID[2] != 't' ||
				wave_data.subChunkID[3] != 'a')
				throw ("Invalid data header");

			//Allocate memory for data
			data = new unsigned char[wave_data.subChunk2Size];

			// Read in the sound data into the soundData variable
			if (!fread(data, wave_data.subChunk2Size, 1, soundFile))
				throw ("error loading WAVE data into struct!");

			//Now we set the variables that we passed in with the
			//data from the structs
			*size = wave_data.subChunk2Size;
			*frequency = wave_format.sampleRate;
			//The format is worked out by looking at the number of
			//channels and the bits per sample.
			if (wave_format.numChannels == 1) {
				if (wave_format.bitsPerSample == 8)
					*format = AL_FORMAT_MONO8;
				else if (wave_format.bitsPerSample == 16)
					*format = AL_FORMAT_MONO16;
			}
			else if (wave_format.numChannels == 2) {
				if (wave_format.bitsPerSample == 8)
					*format = AL_FORMAT_STEREO8;
				else if (wave_format.bitsPerSample == 16)
					*format = AL_FORMAT_STEREO16;
			}
			//create our openAL buffer and check for success
			alGenBuffers(1, buffer);
			//errorCheck();
			//now we put our data into the openAL buffer and
			//check for success
			alBufferData(*buffer, *format, (void*)data,
				*size, *frequency);
			//errorCheck();
			//clean up and return true if successful

			delete[] data;
			fclose(soundFile);
			return true;
		}
		catch (std::string error) {
			//our catch statement for if we throw a string
			std::cerr << error << " : trying to load " << filename << std::endl;
			//clean up memory if wave loading fails
			if (soundFile != NULL)
				fclose(soundFile);
			//return false to indicate the failure to load wave
			return false;
		}
	}

	Audio::Audio(std::string file)
	{
		m_file = file;
	}

	Audio::~Audio()
	{
		UnInit();
	}

	void Audio::Init(bool flushClientSideArray)
	{
	}

	void Audio::Load()
	{
		ALsizei tmp1, tmp2;
		ALenum tmp3;
		loadWavFile(m_file.c_str(), &m_buffer, &tmp1, &tmp2, &tmp3);
		m_loaded = true;
	}

	void Audio::UnInit()
	{
		alDeleteBuffers(1, &m_buffer);
		m_initiated = false;
	}

	void AudioManager::Init()
	{
		ResourceManager::Init();
		m_device = alcOpenDevice(nullptr);
		m_context = alcCreateContext((ALCdevice*)m_device, nullptr);
		alcMakeContextCurrent((ALCcontext*)m_context);
	}

	void AudioManager::Uninit()
	{
		alcDestroyContext((ALCcontext*)m_context);
		alcCloseDevice((ALCdevice*)m_device);
		ResourceManager::Uninit();
	}

	AudioSource::AudioSource()
	{
		alGenSources(1, &m_source);
	}

	AudioSource::~AudioSource()
	{
		alDeleteSources(1, &m_source);
	}

	EntityType AudioSource::GetType() const
	{
		return EntityType::Entity_AudioSource;
	}

	void AudioSource::AttachAudio(std::shared_ptr<Audio> audio)
	{
		m_audio = audio;
		alSourcei(m_source, AL_BUFFER, audio->m_buffer);
	}

	void AudioSource::SetLoop(bool enable)
	{
		if (enable)
			alSourcei(m_source, AL_LOOPING, 1);
		else
			alSourcei(m_source, AL_LOOPING, 0);
	}

	void AudioSource::SetVolume(float val)
	{
		alSourcef(m_source, AL_GAIN, val);
	}

	void AudioPlayer::Play(AudioSource* source)
	{
		alSourcePlay(source->m_source);
	}

	void AudioPlayer::Stop(AudioSource* source)
	{
		alSourceStop(source->m_source);
	}

	void AudioPlayer::Rewind(AudioSource* source)
	{
		alSourceRewind(source->m_source);
	}

	void AudioPlayer::Pause(AudioSource* source)
	{
		alSourcePause(source->m_source);
	}

}
