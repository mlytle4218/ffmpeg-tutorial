#include <stdio.h>
#include <stdlib.h>

extern "C"
{
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

int main(int argc, char *argv[])
{
    av_log_set_level(AV_LOG_QUIET);
    if (argc < 2)
    {
        fprintf(stderr, "Must include a media file\n");
        return 1;
    }
    const char *fileName = argv[1];

    // allocate memory for th format context - this is the bit that holds the
    // information about the container
    AVFormatContext *pFormatContext = avformat_alloc_context();

    // this will take the file, open it and read the header information
    // from it giving the meta information
    avformat_open_input(&pFormatContext, fileName, NULL, NULL);

    printf("Format %s, duration %ld us\n", pFormatContext->iformat->long_name, pFormatContext->duration);

    // This loads finds the stream information. before this, the pFormatContext doesn't have
    //  streams array, afterwards it does
    avformat_find_stream_info(pFormatContext, NULL);

    for (int i = 0; i < pFormatContext->nb_streams; i++)
    {
        AVCodecParameters *pLocalCodecParameters = pFormatContext->streams[i]->codecpar;

        AVCodec *pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);
        printf("codec_type: %d\n",pLocalCodecParameters->codec_type );
        if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            printf("Video Codec: resolution %d x %d\n", pLocalCodecParameters->width, pLocalCodecParameters->height);
            // general
            printf("\tCodec %s ID %d bit_rate %ld\n", pLocalCodec->long_name, pLocalCodec->id, pLocalCodecParameters->bit_rate);

            AVCodecContext *pCodecContext = avcodec_alloc_context3(pLocalCodec);
            avcodec_parameters_to_context(pCodecContext, pLocalCodecParameters);
            avcodec_open2(pCodecContext, pLocalCodec, NULL);

            AVPacket *pPacket = av_packet_alloc();
            AVFrame *pFrame = av_frame_alloc();

            while (av_read_frame(pFormatContext, pPacket) >= 0)
            {
                avcodec_send_packet(pCodecContext, pPacket);
                avcodec_receive_frame(pCodecContext, pFrame);
                // printf("linesize:%d\n", pFrame->linesize[0]);
                // if (pFrame->linesize[0] > 0)
                // {
                //     printf(
                //         "Video Frame %c (%d) pts %ld dts %ld key_frame %d [coded_picture_number %d, display_picture_number %d]\n",
                //         av_get_picture_type_char(pFrame->pict_type),
                //         pCodecContext->frame_number,
                //         pFrame->pts,
                //         pFrame->pkt_dts,
                //         pFrame->key_frame,
                //         pFrame->coded_picture_number,
                //         pFrame->display_picture_number);
                // }
            } 
        }
        avcodec_parameters_free(&pLocalCodecParameters);

    }

    fprintf(stderr, "done \n");
}