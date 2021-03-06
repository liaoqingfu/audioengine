﻿#include "header.h"
#include "codec/aac/libAACenc/include/aacenc_lib.h"
#include "io/include/audioencoder.h"
#include "io/include/audiodecoder.h"
#include "backward_redundant_frame.h"
#include "audio_resample.h"
using namespace audio_engine;
void test_aac_enc()
{
    WavReader reader( "d:/泉水叮咚响.wav" );
    int channel = reader.NumChannels();
    int samplerate = reader.SampleRate();

    auto aacfile = AudioWriter::Create( "D:/泉水叮咚响3500.aac", samplerate, channel, AFT_AAC );
    int frame = 4096;
    int16_t*buf = new int16_t[frame];
    memset( buf, 0, frame );
    for ( ;; )
    {
        auto len = reader.ReadSamples( frame, buf );
        if ( len > 0 )
        {
            aacfile->WriteSamples( buf, len );
        }
        else
        {
            break;
        }
    }

    aacfile->Destroy();
    delete[] buf;
}

#include "codec/aac/libAACdec/include/aacdecoder_lib.h"
void test_aac_dec()
{
    AudioReader*pReader = AudioReader::Create( "D:/diff-40ms.aac", AFT_AAC );
    if (!pReader)
    {
        return;
    }
    AudioWriter*pWriter = AudioWriter::Create( "D:/diff-40ms111.wav", pReader->SampleRate(), pReader->NumChannels(), AFT_WAV );
    if (!pWriter)
    {
        pReader->Destroy();
        return;
    }
    const int frame_size = 4096;
    int16_t buf[frame_size] = { 0 };
    for ( ;; )
    {
        int len = pReader->ReadSamples( frame_size, buf );
        if (len>0)
        {
            pWriter->WriteSamples( buf, len );
        }
        else
        {
            break;
        }
    }
    pReader->Destroy();
    pWriter->Destroy();
#if 0 
    WavWriter* writer = nullptr;// ( "D:/myvoice.wav" );
    FILE* file = fopen( "D:/Mariage.aac", "rb" );
    HANDLE_AACDECODER aacDecoderInfo = aacDecoder_Open( TT_MP4_ADTS, 1 );

    AAC_DECODER_ERROR err;
    err = aacDecoder_SetParam( aacDecoderInfo, AAC_DRC_REFERENCE_LEVEL, 1 );
    uint8_t* buf = new uint8_t[2048];
    UINT bufsize = 2048;
    INT_PCM* outbuf = new INT_PCM[4096];
    CStreamInfo* pStreamInfo = nullptr;
    UINT bytevalid = 0;
    for ( ;; )
    {
        if ( bytevalid == 0 )
        {
            bufsize = fread( buf, 1, 2048, file );
            if ( bufsize == 0 )
            {
                break;
            }
            bytevalid = bufsize;
        }
        UINT startpos = bufsize - bytevalid;
        bufsize = bytevalid;
        UCHAR*p = buf + startpos;
        err = aacDecoder_Fill( aacDecoderInfo, &p, &bufsize, &bytevalid );
        if ( err != AAC_DEC_OK )
        {
            printf( "aacDecoder_Fill() failed: 0x%x", err );
            return;
        }
        if ( bytevalid != 0 )
        {
            printf( "bytevalid = %d\n", bytevalid );
        }
        for ( ;; )
        {
            err = aacDecoder_DecodeFrame( aacDecoderInfo, outbuf, 4096, 0 );
            if ( err == AAC_DEC_NOT_ENOUGH_BITS )
            {
                break;
            }
            else if ( err != AAC_DEC_OK )
            {
                break;
            }
            if ( !pStreamInfo )
            {
                pStreamInfo = aacDecoder_GetStreamInfo( aacDecoderInfo );
                writer = new WavWriter( "D:/myvoice.wav", pStreamInfo->sampleRate, pStreamInfo->numChannels );
            }
            writer->WriteSamples( outbuf, pStreamInfo->frameSize*pStreamInfo->numChannels );
        }

    }
    aacDecoder_Close( aacDecoderInfo );
    if ( writer )delete writer;
    delete[] buf;
    delete[] outbuf;
#endif
}
struct AACDecHeader
{
    uint16_t Syncword;//12
    uint8_t version;//1
    uint8_t Layer;//2
    uint8_t ProtAbsent;//1
    uint8_t Profile;//2
    uint8_t sampleindex;//4
    uint8_t Private_Stream;//1
    uint8_t ChannelMode;//3;
    uint8_t Originality;//1
    uint8_t Home;//1
    uint8_t cr_Stream;//1
    uint8_t cr_Start;//1
    uint16_t frame_length;//13
    uint16_t buf_fullness;//11
    uint8_t num_frame;//2
    uint16_t CRC;//16
};
void test_aac_pasre_head()
{
    //‭111111111111 0 00 1 01 0111 0 010 0 0 0 0 0000010111001 00010101000 00‬
    FILE* file = fopen( "D:/myvoice.aac", "rb" );
    uint8_t header[7] = { 0 };
    AACDecHeader aacHeader;
    int index = 0;
    int seek = 0;
    for ( ;; )
    {
        if ( 7 != fread( header, 1, 7, file ) )
        {
            break;
        }
        uint8_t*p = header;
        aacHeader.Syncword = *p;
        p++;
        aacHeader.Syncword <<= 4;
        aacHeader.Syncword += ( *p >> 4 );
        aacHeader.version = ( *p >> 3 ) & 0x1;
        aacHeader.Layer = ( *p >> 1 ) & 0x3;
        aacHeader.ProtAbsent = *p & 0x1;
        p++;
        aacHeader.Profile = *p >> 6;//2
        aacHeader.sampleindex = ( *p >> 2 ) & 0x0f;//4
        aacHeader.Private_Stream = ( *p >> 1 ) & 0x1;
        aacHeader.ChannelMode = ( *p ) & 0x1;
        aacHeader.ChannelMode <<= 2;
        p++;
        aacHeader.ChannelMode += ( *p >> 6 ) & 0x3;//2
        aacHeader.Originality = ( *p >> 5 ) & 0x1;//1
        aacHeader.Home = ( *p >> 4 ) & 0x1;//1
        aacHeader.cr_Stream = ( *p >> 3 ) & 0x1;//1
        aacHeader.cr_Start = ( *p >> 2 ) & 0x1;//1
        aacHeader.frame_length = ( *p ) & 0x3;//2
        ++p;
        aacHeader.frame_length <<= 8;
        aacHeader.frame_length |= *p;
        aacHeader.frame_length <<= 3;
        ++p;
        aacHeader.frame_length |= *p >> 5;//3

        aacHeader.buf_fullness = *p & 0x1f;//5
        ++p;
        aacHeader.buf_fullness <<= 6;
        aacHeader.buf_fullness |= *p >> 2;//6
        aacHeader.num_frame = *p & 0x3;
        index++;
        printf( "[%d]frame_len = %d\t\t", index, aacHeader.frame_length );

        seek += aacHeader.frame_length;
        fseek( file, seek, SEEK_SET );
    }

    printf( "\n\n\n\n\n\n\nframe number=%d,sample_num=%d,time=%d", index, 4096 * ( index ), 4096 * ( index ) / 44100 / 2 );
    fclose( file );
}

#include "io/include/audioreader.h"
void test_aac_dec_file()
{
    AudioReader* pReader = AudioReader::Create( "D:/myvoice.aac", AFT_AAC );
    printf( "channel:%d", pReader->NumChannels() );
    printf( "samplerate:%d", pReader->SampleRate() );
    printf( "number sample:%d", pReader->NumSamples() );
    WavWriter writer( "D:/myvoice.wav", pReader->SampleRate(), pReader->NumChannels() );
    int16_t buf[4096];
    for ( int i = 0; i < 100; i++ )
    {
        int len = pReader->ReadSamples( 3000, buf );
        if ( len == 0 )
        {
            break;
        }
        writer.WriteSamples( buf, len );
    }
    pReader->SeekSamples( pReader->NumSamples() / 2 );
    for ( ;; )
    {
        int len = pReader->ReadSamples( 3000, buf );
        if ( len == 0 )
        {
            break;
        }
        writer.WriteSamples( buf, len );
    }

    pReader->Destroy();
}
#include <filesystem>

void run_mp32wav( const char* filename )
{
    using file_path = std::tr2::sys::path;

    AudioReader *mp3reader = AudioReader::Create( filename, AFT_MP3 );
    if ( mp3reader == nullptr )
    {
        return;
    }
    file_path p1 = filename;
    p1.replace_extension( ".wav" );

    WavWriter writer( p1.string().c_str(), mp3reader->SampleRate(), mp3reader->NumChannels() );
    int16_t buf[4096];
    for ( ;; )
    {
        int len = mp3reader->ReadSamples( 4096, buf );
        if ( len == 0 )
        {
            break;
        }
        writer.WriteSamples( buf, len );
    }
    mp3reader->Destroy();
}



class Mp3ReadProc : public  AudioBufferProc
{
    AudioReader* pMp3Reader;
    AudioWriter* pMp3Writer;
    int64_t   m_ts = 0;
public:
    Mp3ReadProc()
    {
        pMp3Reader = AudioReader::Create( "E:/CloudMusic/Mariage.mp3", AFT_MP3 );
        // pMp3Reader->SetSpeed( 1 );
        pMp3Writer = AudioWriter::Create( "D:/myvoice.aac", 44100, 2, AFT_AAC );
        std::cout << audio_engine::GetTickCount() << std::endl;
        m_ts = audio_engine::GetTickCount();
    }
    ~Mp3ReadProc()
    {
        auto ts = audio_engine::GetTickCount();
        std::cout << "\n record time: " << ts - m_ts << " sample number： " << pMp3Writer->NumSamples() / pMp3Writer->SampleRate() / pMp3Writer->NumChannels() << std::endl;
        pMp3Reader->Destroy();
        pMp3Writer->Destroy();
    }
    virtual void RecordingDataIsAvailable( const void*data, size_t size_in_byte )
    {
        pMp3Writer->WriteSamples( (int16_t*)data, size_in_byte / 2 );
    };
    virtual size_t NeedMorePlayoutData( void*data, size_t size_in_byte )
    {
        int len = pMp3Reader->ReadSamples( size_in_byte / 2, (int16_t*)data );
        pMp3Writer->WriteSamples( (int16_t*)data, size_in_byte / 2 );
        if ( len == 0 )
        {
            pMp3Reader->SeekTime( 0 );
        }
        return size_in_byte;
    }
};



void test_play_mp3()
{
    AudioDevice* pWinDevice = AudioDevice::Create(AudioDevice::eCoreAudio);
    pWinDevice->Initialize();

    Mp3ReadProc cb;
    pWinDevice->SetAudioBufferCallback( &cb );
    pWinDevice->StartPlayout();
    //  pWinDevice->StartRecording();
    system( "pause" );
    pWinDevice->StopPlayout();
    pWinDevice->StopRecording();
    pWinDevice->Terminate();
    pWinDevice->Release();
}

void run_wav2mp3( const char* infile, const char*outfile )
{
    WavReader reader( infile );
    AudioWriter* pMp3 = AudioWriter::Create( outfile, reader.SampleRate(), reader.NumChannels(), AFT_MP3 );
    const int NUM_SAMPLES = 256;
    int16_t pcm[NUM_SAMPLES];
    float fpcm[NUM_SAMPLES];
    for ( ;; )
    {
        if ( NUM_SAMPLES != reader.ReadSamples( NUM_SAMPLES, pcm ) )
            break;
        S16ToFloat( pcm, NUM_SAMPLES, fpcm );
        pMp3->WriteSamples( pcm, NUM_SAMPLES );
    }
    pMp3->Destroy();
}


#include "G7221Interface.h"

std::list < char*> pList;
std::mutex _lock;

void __stdcall EncodeCallback( const void* data, int lenofbytes, void* pcmData,int len )
{
    std::lock_guard<std::mutex> ls( _lock );
    char* p = new char[lenofbytes];
    memcpy( p, data, lenofbytes );
    pList.push_back(p);
   // printf( "[%d]recv encode len = %d\n", (int)timer.elapsed(),lenofbytes );
}

void __stdcall DecodeCallback( const void* data, int lenofbytes, void* pcmData, int len )
{
    std::lock_guard<std::mutex> ls( _lock );
    if (pList.size()<50)
    {
        //memset( (void*)data, 0, lenofbytes );
        return;
    }
    char* p = pList.front();
    memcpy( (void*)data, p, lenofbytes );
    pList.pop_front();
    //printf( "[%d]recv encode len = %d\n", (int)timer.elapsed(), lenofbytes );
}

void* encoder = nullptr;
void* decoder = nullptr;
void test_encoder_g7221()
{
    encoder = CreateEncoder( EncodeCallback );
    StartEncode( encoder );
    decoder = CreateDecoder( DecodeCallback );
    StartDecode( decoder );
    system( "pause" );
    StopEncode( encoder );
    DeleteEncoder( encoder );
    StopDecode( decoder );
    DeleteEncoder( decoder );
}



#define FRAME_SIZE 40
void test_g7221_encode()
{
    FILE* file = fopen( "D:/rec.pak","wb+" );
    auto pG7221 = AudioEncoder::Create( AFT_G7221, 16000, 1, 2000 );
    WavReader reader( "D:/直接采集PCM音频.wav" );
    int16_t pcm[16000 * FRAME_SIZE / 1000] = { 0 };
    int16_t encBuf[80 * 2] = {0};
    int bufLen = 80*2*2;
    for ( ;; )
    {
        if ( FRAME_SIZE * 16 != reader.ReadSamples( FRAME_SIZE*16, pcm ) )
        {
            break;
        }
        pG7221->Encode( pcm, FRAME_SIZE * 16 * 2, (char*)encBuf, bufLen );
        fwrite( encBuf, 1, 80, file );
    }
    fclose(file);
    pG7221->Release();

}


void test_g7221_decode()
{
    FILE* file = fopen( "D:/rec.pak", "rb+" );
    auto pG7221 = AudioDecoder::Create( AFT_G7221 );
    WavWriter writer( "D:/rec.wav", 16000, 1 );
    int16_t pcm[16000 * FRAME_SIZE / 1000];
    uint8_t encBuf[80] = { 0 };
    int bufLen = FRAME_SIZE*2;
    int decLen = FRAME_SIZE*16*2;
    for ( ;; )
    {
        int len = fread( encBuf, 1, bufLen, file );
        if( bufLen != len)
            break;
        pG7221->Decode( encBuf, bufLen, pcm, decLen );
        writer.WriteSamples( pcm, decLen / 2 );
    }
    fclose( file );
    pG7221->Release();
}

#ifdef WEBRTC_G722
#include "webrtc/modules/audio_coding/codecs/g722/g722_interface.h"
void test_g722_encode()
{

}

void test_g722_decode()
{
    FILE* file = fopen("D:/rec.","rb+");
    WavWriter writer( "D:/g722.wav",16000,1 );
     G722DecInst *G722dec_inst;
     WebRtcG722_CreateDecoder( &G722dec_inst );
     if (!G722dec_inst)
     {
         return;
     }
     WebRtcG722_DecoderInit( G722dec_inst );
     char encBuf[80] = { 0 };
     int16_t decBuf[512] = { 0 };
     int16_t speechType = 0;
     for ( ;; )
     {
         int ret = fread( encBuf, 1, 20, file );
         if (ret != 20)
         {
             break;
         }

         size_t len = WebRtcG722_Decode( G722dec_inst, (uint8_t*)encBuf, 20, decBuf, &speechType );
         writer.WriteSamples( decBuf, len );
     }
     WebRtcG722_FreeDecoder( G722dec_inst );
     fclose( file );

}

#endif

#ifdef HK_G722_COEDC
#include "C:/Users/zhangnaigan/Desktop/CH-HCNetSDK(Windows32)V5.2.1.3_build20160513/头文件/HCNetSDK.h"
//#pragma comment(lib,"HCNetSDK.lib")
void test_hk_g722_encode()
{
    bool bInit = NET_DVR_Init();
    if (!bInit)
    {
        return;
    }
    void*encoder = NET_DVR_InitG722Encoder();
    FILE* file = fopen( "D:/rec.pak", "wb+" );
    WavReader reader( "D:/直接采集PCM音频.wav" );
    int16_t pcm[16000 * FRAME_SIZE / 1000] = { 0 };
    int16_t encBuf[80 * 2] = { 0 };
    int bufLen = 80 * 2 * 2;
    for ( ;; )
    {
        if ( FRAME_SIZE * 16 != reader.ReadSamples( FRAME_SIZE * 16, pcm ) )
        {
            break;
        }
        bool bencode = NET_DVR_EncodeG722Frame( encoder, (BYTE*)pcm, (BYTE*)encBuf );
        fwrite( encBuf, 1, 80, file );
    }
    fclose( file );
    NET_DVR_ReleaseG722Encoder( encoder );
    NET_DVR_Cleanup();
}


void test_hk_g722_decode()
{
    FILE* file = fopen( "D:/rec.pak", "rb+" );
    WavWriter writer( "D:/g722.wav", 16000, 1 );
    bool bInit = NET_DVR_Init();
    if ( !bInit )
    {
        return;
    }
    G722DecInst *G722dec_inst;
    WebRtcG722_CreateDecoder( &G722dec_inst );
    if ( !G722dec_inst )
    {
        return;
    }
    WebRtcG722_DecoderInit( G722dec_inst );
    char encBuf[80] = { 0 };
    int16_t decBuf[512] = { 0 };
    int16_t speechType = 0;
    for ( ;; )
    {
        int ret = fread( encBuf, 1, 80, file );
        if ( ret != 20 )
        {
            break;
        }

        size_t len = WebRtcG722_Decode( G722dec_inst, (uint8_t*)encBuf, 20, decBuf, &speechType );
        writer.WriteSamples( decBuf, len );
    }
    WebRtcG722_FreeDecoder( G722dec_inst );
    fclose( file );
}
#endif

// 测试看看一个AAC包能多携带3倍的数据而不影响音质。AAC至少要开5k的bitrate
void test_encode_backward_redundant_frame()
{
    AudioReader* pReader = AudioReader::Create( "D:/log/test-16000.wav", AFT_WAV );
    if (!pReader)
    {
        return;
    }
    AudioWriter* pWriter = AudioWriter::Create( "D:/log/test-48000-2.wav", 48000,2, AFT_WAV );
    if (!pWriter)
    {
        pReader->Destroy();
        return;
    }
    int frame_size = pReader->SampleRate() / 100*2;
    BackwardRedunantFrame frame48000;
    frame48000.Init(pReader->SampleRate());

    int16_t frame[160*2] = { 0 };
    int16_t outFrame[960] = { 0 };
    for ( ;; )
    {
        int len = pReader->ReadSamples(frame_size,frame);
        if (len == 0)
        {
            break;
        }
        int outLen = 0;
        frame48000.Process( frame, frame_size, outFrame, outLen );
        assert( outLen == 960 );
        pWriter->WriteSamples( outFrame, outLen );
    }

    pReader->Destroy();
    pWriter->Destroy();
    pReader = nullptr;
    pWriter = nullptr;
}

void test_decode_backward_redundant_frame()
{
    AudioReader* pReader = AudioReader::Create( "D:/log/test-48000-2.wav", AFT_WAV );
    if ( !pReader )
    {
        return;
    }
    AudioWriter* pWriter = AudioWriter::Create( "D:/log/test-48000-2.wav", 24000, 1, AFT_WAV );
    if ( !pWriter )
    {
        pReader->Destroy();
        return;
    }
    int16_t frame[240] = { 0 };
    int16_t outFrame[960] = { 0 };
    for ( ;; )
    {
        int len = pReader->ReadSamples( 960, outFrame );

    }
}


void test_aacencoder()
{
    FILE* file = fopen( "D:/diff-40ms.aac","wb+" );
    if (!file)
    {
        return;
    }
    AudioReader*pReader = AudioReader::Create( "D:/es01.wav", AFT_WAV );
    AudioEncoder* pEncoder = AudioEncoder::Create( AFT_AAC, pReader->SampleRate(), 2, 8 * 3000 );//2channel,3k
    
    int16_t input_buf[4096] = { 0 };
    int16_t input_buf_s[4096] = { 0 };
    char output_buf[768] = { 0 };
    int outLen = 0;

    //缓存4个包
    const int DELAY_PACKET = 4;
    int16_t vecbuf[DELAY_PACKET][4096] = {0};
    int num_sample = 4096;

    for ( ;; )
    {
        int len = pReader->ReadSamples( 4096, input_buf );
        if (pReader->NumChannels() == 2)
        {
            AudioResample::ToMono( input_buf, num_sample );
        }
        AudioResample::Tostereo( input_buf, vecbuf[0], num_sample/2, input_buf_s );
        memcpy( vecbuf[0], vecbuf[1], 3 * num_sample*2 );
        memcpy( vecbuf[3], input_buf, num_sample );

        pEncoder->Encode( input_buf_s, len, output_buf, outLen );
        if (outLen>0)
        {
            fwrite( &outLen, sizeof( int ), 1, file );
            fwrite( output_buf, 1, outLen, file );
            outLen = 0;
        }
        if (len == 0)
        {
            break;
        }
    }
    pReader->Destroy();
    pEncoder->Release();
    fclose(file);
}

void test_aacdecoder()
{
    AudioDecoder* pDecoder = AudioDecoder::Create( AFT_AAC );
    AudioWriter* pWriter = AudioWriter::Create("D:/diff-40ms-dec.wav",48000,1,AFT_WAV);
    FILE*aacfile =  fopen("D:/diff-40ms.aac","rb+");
    int16_t output_buf[4096] = { 0 };
    char input_buf[768*2] = { 0 };
    int lost = 0;
    //缓存4个包
    const int DELAY_PACKET = 5;
    int16_t vecbuf[DELAY_PACKET][4096] = { 0 };
    int num_sample =4096;
    int16_t input_buf_s[4096] = { 0 };
    int lost_list[5];
    memset( lost_list, 0, sizeof( int )*5 );
    for ( ;; )
    {
        int framelen = 0;
        int len = fread( &framelen, 4, 1, aacfile );
        if ( framelen > 0 )
        {
            len = fread( input_buf, 1, framelen, aacfile );
           if (len != framelen)
           {
               break;
           }
           if (lost++ < 0)
           {
               printf( "[%d]lost\n", lost );

               memcpy( lost_list, lost_list + 1, sizeof( int ) * 4 );
               lost_list[4] = 1;
           }
           else
           {
               if (lost%8==0)
               {
                   lost = 0;
               }
               printf( "[%d]normal\n", lost );
               if ( pDecoder->Decode( input_buf, framelen, output_buf, framelen ) )
               {
                   memcpy( input_buf_s, vecbuf[0], num_sample );
                   memcpy( vecbuf[0], vecbuf[1], 4 * num_sample * 2 );
                   memcpy( vecbuf[4], output_buf, num_sample * 2 );
                   memcpy( lost_list, lost_list + 1, sizeof( int ) * 4 );
                   lost_list[4] = 0;
               }
           }   

           if (lost_list[0] )
           {
               AudioResample::ToMono( vecbuf[4], num_sample, nullptr, vecbuf[0] );
              // AudioResample::ToMono( vecbuf[3], num_sample, nullptr, vecbuf[0] );
           }
           else
           {
               AudioResample::ToMono( vecbuf[0], num_sample);
           }
           pWriter->WriteSamples( input_buf_s, num_sample / 2 );
        }
        else
        {
            break;
        }
    }
    pDecoder->Release();
    fclose( aacfile );
    pWriter->Destroy();
}

void test_opusencoder()
{
    FILE* out = fopen( "d:/v1114.opus", "wb+" );
    AudioReader*pReader = AudioReader::Create( "D:/es01.wav", AFT_WAV );
    int channel = pReader->NumChannels();
    AudioEncoder*pEncoder = AudioEncoder::Create( AFT_OPUS, pReader->SampleRate(), channel, 24000 );
    const int frame_size = pReader->SampleRate() / 1000 * 40*channel;
    int16_t* src = new int16_t[frame_size];
    char encbuf[1024];
    for ( ;; )
    {
        int nSamples = pReader->ReadSamples( frame_size, src );
        if (nSamples == 0)
        {
            break;
        }
        if (nSamples < frame_size)
        {
            memset( src+nSamples, 0, 2*(frame_size-nSamples));
        }
        int outLen = 0;
        if ( !pEncoder->Encode( src, frame_size, encbuf, outLen ) )
        {
            break;
        }
        fwrite( encbuf, 1, outLen, out );
    }

    fclose( out );
    pReader->Destroy();
    pEncoder->Release();
    delete src;
};

static uint32_t char_to_int( unsigned char ch[4] )
{
    return ( (uint32_t)ch[0] << 24 ) | ( (uint32_t)ch[1] << 16 )
        | ( (uint32_t)ch[2] << 8 ) | (uint32_t)ch[3];
}

void test_opusdecoder()
{
    const int max_payload_bytes = 1500;
    FILE* fin = fopen( "d:/v1014.opus", "rb+" );
    if (!fin)
    {
        return;
    }
    AudioWriter*pWriter = AudioWriter::Create( "d:/v10141.wav", 48000, 2, AFT_WAV );
    AudioDecoder*pDecoder = AudioDecoder::Create( AFT_OPUS,48000,2 );
    int16_t outBuf[1920 * 2] = {0};
    unsigned char inBuf[max_payload_bytes] = {0};
    for ( int i=0;;i++ )
    {
        unsigned char ch[4];
        int err = fread( ch, 1, 4, fin );
        if ( feof( fin ) )
            break;
        int len = char_to_int( ch );
        if ( len > max_payload_bytes || len < 0 )
        {
            fprintf( stderr, "Invalid payload length: %d\n", len );
            break;
        }
        err = fread( ch, 1, 4, fin );
        int enc_final_range = char_to_int( ch );
        err = fread( inBuf, 1, len, fin );
        if ( err < len )
        {
            fprintf( stderr, "Ran out of input, "
                     "expecting %d bytes got %d\n",
                     len, err );
            break;
        }
        int outLen = 0;
        if (i%2>0)//选择性的丢包
        {
            len = 0;
        }
        pDecoder->Decode( inBuf, len, outBuf, outLen );
        if (outLen>0)
        {
            pWriter->WriteSamples( outBuf, outLen );
        }
    }
    fclose( fin );
    pWriter->Destroy();
    pDecoder->Release();
}

void test_codec()
{
    
   // run_wav2mp3( "C:/Users/zhangnaigan/Desktop/歌曲.wav", "C:/Users/zhangnaigan/Desktop/歌曲1.mp3" );
   // run_wav2mp3( "E:/CloudMusic/Mariage.wav", "C:/Users/zhangnaigan/Desktop/Mariage.mp3" );
   // run_mp32wav( "E:/CloudMusic/Mariage.mp3" );
   // test_encoder_g7221();
   // test_g7221_encode();
    //test_g7221_decode();
   // test_g722_encode();
   // test_g722_decode();
   // test_aac_enc();
   // test_aac_dec();
    //test_encode_backward_redundant_frame();
    //test_decode_backward_redundant_frame();

    //test_aacencoder();
   // test_aacdecoder();

    //test_opusencoder();
    auto t1 = std::thread( test_opusdecoder );
    auto t2 = std::thread( test_opusencoder );
    t1.join();
    t2.join();
}