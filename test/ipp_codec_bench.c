/**
 * Copyright (C) 2022, Advanced Micro Devices. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
 
 /** @file ipp_codec_bench.c
 *
 *  @brief Test bench application to benchmark IPP library compression methods.
 *
 *  This file contains the functions to load, setup, call and benchmark the
 *  compression and decompression methods supported by IPP compression library.
 *
 *  @author S. Biplab Raut
 */

#ifndef _WIN32
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "api/types.h"
#include "api/api.h"
#include "utils/utils.h"
#include "codec_bench.h"
#include "algos/zlib/zlib.h"

//Handle to IPP dynamic library
VOID *hDL[AOCL_COMPRESSOR_ALGOS_NUM] = { NULL };

//Internal wrapper function declarations that make calls to IPP library APIs
INT64 ipp_lz4_run(aocl_codec_bench_info *codec_bench_handle,
            aocl_codec_desc *aocl_codec_handle, VOID *hDL,
            INT *verifyRes);
INT64 ipp_lz4hc_run(aocl_codec_bench_info *codec_bench_handle,
            aocl_codec_desc *aocl_codec_handle, VOID *hDL,
            INT *verifyRes);
INT64 ipp_zlib_run(aocl_codec_bench_info *codec_bench_handle,
            aocl_codec_desc *aocl_codec_handle, VOID *hDL,
            INT *verifyRes);

INT ipp_setup(aocl_codec_bench_info *codec_bench_handle,
              aocl_codec_desc *aocl_codec_handle)
{ 
    LOG(TRACE, aocl_codec_handle->printDebugLogs, "Enter");
    
    switch (codec_bench_handle->codec_method)
    {
        case LZ4:
        case LZ4HC:
            hDL[LZ4] = hDL[LZ4HC] =
			    dlopen("liblz4.so", RTLD_NOW | RTLD_DEEPBIND);
            if (hDL[LZ4] == NULL)
            {
                LOG(ERR, aocl_codec_handle->printDebugLogs,
                "Error in opening dynamic library [liblz4.so]:[%s]",
                dlerror());
                LOG(TRACE, aocl_codec_handle->printDebugLogs, "Exit");
                return -2;
            }
#ifdef _GNU_SOURCE
            if (aocl_codec_handle->printDebugLogs)
            {
                Dl_info dl_info;
                dladdr(dlsym(hDL[LZ4], "LZ4_decompress_safe"), &dl_info);
                LOG(INFO, aocl_codec_handle->printDebugLogs,
                "Library [liblz4.so] opened from path: [%s]",
                dl_info.dli_fname);
            }
#endif
            break;
        case ZLIB:
            hDL[ZLIB] = dlopen("libz.so", RTLD_NOW | RTLD_DEEPBIND);
            if (hDL[ZLIB] == NULL)
            {
                LOG(ERR, aocl_codec_handle->printDebugLogs,
                "Error in opening dynamic library [libz.so]:[%s]", dlerror());
                LOG(TRACE, aocl_codec_handle->printDebugLogs, "Exit");
                return -2;
            }
#ifdef _GNU_SOURCE
            if (aocl_codec_handle->printDebugLogs)
            {
                Dl_info dl_info;
                dladdr(dlsym(hDL[ZLIB], "uncompress"), &dl_info);
                LOG(INFO, aocl_codec_handle->printDebugLogs,
                "Library [libz.so] opened from path: [%s]", dl_info.dli_fname);
            }
#endif
            break;
        case LZMA:
        case BZIP2:
        case SNAPPY:
        case ZSTD:
            LOG(ERR, aocl_codec_handle->printDebugLogs,
            "Only supported compression methods are: LZ4, LZ4HC and ZLIB");
            LOG(TRACE, aocl_codec_handle->printDebugLogs, "Exit");
            return -1;
            break;
        case -1:
            hDL[LZ4] = hDL[LZ4HC] = 
                dlopen("liblz4.so", RTLD_NOW | RTLD_DEEPBIND);
            if (hDL[LZ4] == NULL)
            {
                LOG(ERR, aocl_codec_handle->printDebugLogs,
                "Error in opening dynamic library [liblz4.so]:[%s]",
                dlerror());
                LOG(TRACE, aocl_codec_handle->printDebugLogs, "Exit");
                return -2;
            }
#ifdef _GNU_SOURCE
            if (aocl_codec_handle->printDebugLogs)
            {
                Dl_info dl_info;
                dladdr(dlsym(hDL[LZ4], "LZ4_decompress_safe"), &dl_info);
                LOG(INFO, aocl_codec_handle->printDebugLogs,
                "Library [liblz4.so] opened from path: [%s]",
                dl_info.dli_fname);
            }
#endif
            hDL[ZLIB] = dlopen("libz.so", RTLD_NOW | RTLD_DEEPBIND);
            if (hDL[ZLIB] == NULL)
            {
                LOG(ERR, aocl_codec_handle->printDebugLogs,
                "Error in opening dynamic library [libz.so]:[%s]", dlerror());
                LOG(TRACE, aocl_codec_handle->printDebugLogs, "Exit");
                return -2;
            }
#ifdef _GNU_SOURCE
            if (aocl_codec_handle->printDebugLogs)
            {
                Dl_info dl_info;
                dladdr(dlsym(hDL[ZLIB], "uncompress"), &dl_info);
                LOG(INFO, aocl_codec_handle->printDebugLogs,
                "Library [libz.so] opened from path: [%s]", dl_info.dli_fname);
            }
#endif
            break;
        default:
            LOG(ERR, aocl_codec_handle->printDebugLogs,
            "Only supported compression methods are: LZ4, LZ4HC and ZLIB");
            LOG(TRACE, aocl_codec_handle->printDebugLogs, "Exit");
            return -1;
            break;
    }
    
    LOG(TRACE, aocl_codec_handle->printDebugLogs, "Exit");
    return 0;
}


INT64 ipp_lz4_run(aocl_codec_bench_info *codec_bench_handle,
                  aocl_codec_desc *aocl_codec_handle, VOID *hDL,
                  INT *verifyRes)
{
    INT64 resultComp = 0;
    INT64 resultDecomp = 0;
    UINT inSize, file_size;
    INT j, k, l;
    timer clkTick;
    timeVal startTime, endTime;
    FILE *inFp = codec_bench_handle->fp;
    INT64 (*fCompDL) (CHAR *, CHAR *, UINT, UINT);
    INT64 (*fDecompDL) (CHAR *, CHAR *, UINT, UINT);

    LOG(TRACE, aocl_codec_handle->printDebugLogs, "Enter");

    fCompDL = dlsym(hDL, "LZ4_compress_default");
    fDecompDL = dlsym(hDL, "LZ4_decompress_safe");
    if (fCompDL == NULL || fDecompDL == NULL)
    {
        LOG(ERR, aocl_codec_handle->printDebugLogs,
        "Error in loading symbols from dynamic library: [%s]", dlerror());
        return -2;
    }

    initTimer(clkTick);

    for (k = 0; k < codec_bench_handle->iterations; k++)
    {
        inSize = codec_bench_handle->inSize;
        file_size = codec_bench_handle->file_size;
        while (inSize)
        {
            inSize = fread(codec_bench_handle->inPtr, 1, inSize, inFp);

            //compress
            aocl_codec_handle->inSize = aocl_codec_handle->outSize =
                /*aocl_codec_handle->chunk_size =*/ inSize;
            aocl_codec_handle->inBuf = codec_bench_handle->inPtr;
            aocl_codec_handle->outBuf = codec_bench_handle->compPtr;
            getTime(startTime);
            resultComp = fCompDL(aocl_codec_handle->inBuf,
                                 aocl_codec_handle->outBuf,
                                 aocl_codec_handle->inSize,
                                 aocl_codec_handle->outSize);
            getTime(endTime);
            aocl_codec_handle->cSize = resultComp;
            aocl_codec_handle->cTime = diffTime(timer, startTime, endTime);
            aocl_codec_handle->cSpeed = (aocl_codec_handle->inSize * 1000.0) /
                                        aocl_codec_handle->cTime;

            if (resultComp <= 0)
                return -1;

            //decompress
            aocl_codec_handle->inSize = resultComp;
            aocl_codec_handle->outSize = inSize;
            aocl_codec_handle->inBuf = codec_bench_handle->compPtr;
            aocl_codec_handle->outBuf = codec_bench_handle->decompPtr;
            getTime(startTime);
            resultDecomp = fDecompDL(aocl_codec_handle->inBuf,
                                     aocl_codec_handle->outBuf,
                                     aocl_codec_handle->inSize,
                                     aocl_codec_handle->outSize);
            getTime(endTime);
            aocl_codec_handle->dSize = resultDecomp;
            aocl_codec_handle->dTime = diffTime(timer, startTime, endTime);
            aocl_codec_handle->dSpeed = (aocl_codec_handle->dSize * 1000.0) /
                                        aocl_codec_handle->dTime;

            if (resultDecomp <= 0)
                return -1;

            if (codec_bench_handle->verify)
            {
                *verifyRes = memcmp(codec_bench_handle->inPtr,
                                    codec_bench_handle->decompPtr, inSize);
                if (*verifyRes != 0)
                {
                    return -3;
                }
            }
            if (codec_bench_handle->print_stats)
            {
                codec_bench_handle->cTime +=
                    aocl_codec_handle->cTime;
                codec_bench_handle->cSize +=
                    aocl_codec_handle->cSize;
                codec_bench_handle->dTime +=
                    aocl_codec_handle->dTime;
                codec_bench_handle->dSize +=
                    aocl_codec_handle->dSize;
                codec_bench_handle->cBestTime = 
                    (codec_bench_handle->cBestTime == 0) ? 
                    aocl_codec_handle->cTime :
                    (codec_bench_handle->cBestTime > 
                     aocl_codec_handle->cTime) ?
                    aocl_codec_handle->cTime : 
                    codec_bench_handle->cBestTime;
                codec_bench_handle->dBestTime = 
                    (codec_bench_handle->dBestTime == 0) ? 
                    aocl_codec_handle->dTime :
                    (codec_bench_handle->dBestTime > 
                     aocl_codec_handle->dTime) ?
                    aocl_codec_handle->dTime : 
                    codec_bench_handle->dBestTime;
            }
            file_size -= inSize;
            inSize = (file_size > inSize) ? inSize : file_size;
        }
        rewind(inFp);
    }

    LOG(TRACE, aocl_codec_handle->printDebugLogs, "Exit");

    return 0;
}

INT64 ipp_lz4hc_run(aocl_codec_bench_info *codec_bench_handle,
                    aocl_codec_desc *aocl_codec_handle, VOID *hDL,
                    INT *verifyRes)
{
    INT64 resultComp = 0;
    INT64 resultDecomp = 0;
    UINT inSize, file_size;
    INT j, k, l;
    timer clkTick;
    timeVal startTime, endTime;
    FILE *inFp = codec_bench_handle->fp;
    INT64 (*fCompDL) (CHAR *, CHAR *, UINT, UINT, UINT);
    INT64 (*fDecompDL) (CHAR *, CHAR *, UINT, UINT);

    LOG(TRACE, aocl_codec_handle->printDebugLogs, "Enter");

    fCompDL = dlsym(hDL, "LZ4_compress_HC");
    fDecompDL = dlsym(hDL, "LZ4_decompress_safe");
    if (fCompDL == NULL || fDecompDL == NULL)
    {
        LOG(ERR, aocl_codec_handle->printDebugLogs,
        "Error in loading symbols from dynamic library: [%s]", dlerror());
        return -2;
    }

    initTimer(clkTick);

    for (k = 0; k < codec_bench_handle->iterations; k++)
    {
        inSize = codec_bench_handle->inSize;
        file_size = codec_bench_handle->file_size;
        while (inSize)
        {
            inSize = fread(codec_bench_handle->inPtr, 1, inSize, inFp);

            //compress
            aocl_codec_handle->inSize = aocl_codec_handle->outSize =
                /*aocl_codec_handle->chunk_size =*/ inSize;
            aocl_codec_handle->inBuf = codec_bench_handle->inPtr;
            aocl_codec_handle->outBuf = codec_bench_handle->compPtr;
            getTime(startTime);
            resultComp = fCompDL(aocl_codec_handle->inBuf,
                                 aocl_codec_handle->outBuf,
                                 aocl_codec_handle->inSize,
                                 aocl_codec_handle->outSize,
                                 aocl_codec_handle->level);
            getTime(endTime);
            aocl_codec_handle->cSize = resultComp;
            aocl_codec_handle->cTime = diffTime(timer, startTime, endTime);
            aocl_codec_handle->cSpeed = (aocl_codec_handle->inSize * 1000.0) /
                                        aocl_codec_handle->cTime;

            if (resultComp <= 0)
                return -1;

            //decompress
            aocl_codec_handle->inSize = resultComp;
            aocl_codec_handle->outSize = inSize;
            aocl_codec_handle->inBuf = codec_bench_handle->compPtr;
            aocl_codec_handle->outBuf = codec_bench_handle->decompPtr;
            getTime(startTime);
            resultDecomp = fDecompDL(aocl_codec_handle->inBuf,
                                     aocl_codec_handle->outBuf,
                                     aocl_codec_handle->inSize,
                                     aocl_codec_handle->outSize);
            getTime(endTime);
            aocl_codec_handle->dSize = resultDecomp;
            aocl_codec_handle->dTime = diffTime(timer, startTime, endTime);
            aocl_codec_handle->dSpeed = (aocl_codec_handle->dSize * 1000.0) /
                                        aocl_codec_handle->dTime;

            if (resultDecomp <= 0)
                return -1;

            if (codec_bench_handle->verify)
            {
                *verifyRes = memcmp(codec_bench_handle->inPtr,
                                    codec_bench_handle->decompPtr, inSize);
                if (*verifyRes != 0)
                {
                    return -3;
                }
            }
            if (codec_bench_handle->print_stats)
            {
                codec_bench_handle->cTime +=
                    aocl_codec_handle->cTime;
                codec_bench_handle->cSize +=
                    aocl_codec_handle->cSize;
                codec_bench_handle->dTime +=
                    aocl_codec_handle->dTime;
                codec_bench_handle->dSize +=
                    aocl_codec_handle->dSize;
                codec_bench_handle->cBestTime = 
                    (codec_bench_handle->cBestTime == 0) ? 
                    aocl_codec_handle->cTime :
                    (codec_bench_handle->cBestTime > 
                     aocl_codec_handle->cTime) ?
                    aocl_codec_handle->cTime : 
                    codec_bench_handle->cBestTime;
                codec_bench_handle->dBestTime = 
                    (codec_bench_handle->dBestTime == 0) ? 
                    aocl_codec_handle->dTime :
                    (codec_bench_handle->dBestTime > 
                     aocl_codec_handle->dTime) ?
                    aocl_codec_handle->dTime : 
                    codec_bench_handle->dBestTime;
            }
            file_size -= inSize;
            inSize = (file_size > inSize) ? inSize : file_size;
        }
        rewind(inFp);
    }

    LOG(TRACE, aocl_codec_handle->printDebugLogs, "Exit");

    return 0;
}

INT64 ipp_zlib_run(aocl_codec_bench_info *codec_bench_handle,
                   aocl_codec_desc *aocl_codec_handle, VOID *hDL,
                   INT *verifyRes)
{
    INT64 resultComp = 0;
    INT64 resultDecomp = 0;
    UINT inSize, file_size;
    INT j, k, l, ret;
    timer clkTick;
    timeVal startTime, endTime;
    FILE *inFp = codec_bench_handle->fp;
    INT64 (*fCompDL) (UINT8 *, uLongf *, UINT8 *, UINT, UINT);
    INT64 (*fDecompDL) (UINT8 *, uLongf *, UINT8 *, UINT);

    LOG(TRACE, aocl_codec_handle->printDebugLogs, "Enter");

    fCompDL = dlsym(hDL, "compress2");
    fDecompDL = dlsym(hDL, "uncompress");
    if (fCompDL == NULL || fDecompDL == NULL)
    {
        LOG(ERR, aocl_codec_handle->printDebugLogs,
        "Error in loading symbols from dynamic library: [%s]", dlerror());
        return -2;
    }

    initTimer(clkTick);

    for (k = 0; k < codec_bench_handle->iterations; k++)
    {
        inSize = codec_bench_handle->inSize;
        file_size = codec_bench_handle->file_size;
        while (inSize)
        {
            inSize = fread(codec_bench_handle->inPtr, 1, inSize, inFp);

            //compress
            aocl_codec_handle->inSize = aocl_codec_handle->outSize =
                /*aocl_codec_handle->chunk_size =*/ inSize;
            aocl_codec_handle->inBuf = codec_bench_handle->inPtr;
            aocl_codec_handle->outBuf = codec_bench_handle->compPtr;
            getTime(startTime);
            ret = fCompDL((UINT8 *)aocl_codec_handle->outBuf,
                          (uLongf *)&aocl_codec_handle->outSize,
                          (UINT8 *)aocl_codec_handle->inBuf,
                          (UINT)aocl_codec_handle->inSize,
                          (UINT)aocl_codec_handle->level);
            getTime(endTime);
            aocl_codec_handle->cSize = resultComp = aocl_codec_handle->outSize;
            aocl_codec_handle->cTime = diffTime(timer, startTime, endTime);
            aocl_codec_handle->cSpeed = (aocl_codec_handle->inSize * 1000.0) /
                                        aocl_codec_handle->cTime;

            if (ret != Z_OK)
                return -1;

            //decompress
            aocl_codec_handle->inSize = resultComp;
            aocl_codec_handle->outSize = inSize;
            aocl_codec_handle->inBuf = codec_bench_handle->compPtr;
            aocl_codec_handle->outBuf = codec_bench_handle->decompPtr;
            getTime(startTime);
            ret = fDecompDL((UINT8 *)aocl_codec_handle->outBuf,
                            (uLongf *)&aocl_codec_handle->outSize,
                            (UINT8 *)aocl_codec_handle->inBuf,
                            aocl_codec_handle->inSize);
            getTime(endTime);
            aocl_codec_handle->dSize = resultDecomp = aocl_codec_handle->outSize;
            aocl_codec_handle->dTime = diffTime(timer, startTime, endTime);
            aocl_codec_handle->dSpeed = (aocl_codec_handle->dSize * 1000.0) /
                                        aocl_codec_handle->dTime;

            if (ret != Z_OK)
                return -1;

            if (codec_bench_handle->verify)
            {
                *verifyRes = memcmp(codec_bench_handle->inPtr,
                                    codec_bench_handle->decompPtr, inSize);
                if (*verifyRes != 0)
                {
                    return -3;
                }
            }
            if (codec_bench_handle->print_stats)
            {
                codec_bench_handle->cTime +=
                    aocl_codec_handle->cTime;
                codec_bench_handle->cSize +=
                    aocl_codec_handle->cSize;
                codec_bench_handle->dTime +=
                    aocl_codec_handle->dTime;
                codec_bench_handle->dSize +=
                    aocl_codec_handle->dSize;
                codec_bench_handle->cBestTime = 
                    (codec_bench_handle->cBestTime == 0) ? 
                    aocl_codec_handle->cTime :
                    (codec_bench_handle->cBestTime > 
                     aocl_codec_handle->cTime) ?
                    aocl_codec_handle->cTime : 
                    codec_bench_handle->cBestTime;
                codec_bench_handle->dBestTime = 
                    (codec_bench_handle->dBestTime == 0) ? 
                    aocl_codec_handle->dTime :
                    (codec_bench_handle->dBestTime > 
                     aocl_codec_handle->dTime) ?
                    aocl_codec_handle->dTime : 
                    codec_bench_handle->dBestTime;
            }
            file_size -= inSize;
            inSize = (file_size > inSize) ? inSize : file_size;
        }
        rewind(inFp);
    }

    LOG(TRACE, aocl_codec_handle->printDebugLogs, "Exit");

    return 0;
}

INT64 ipp_run(aocl_codec_bench_info *codec_bench_handle,
              aocl_codec_desc *aocl_codec_handle)
{
    aocl_codec_type i; 
    INT j, l, verifyRes, ret;

    LOG(TRACE, aocl_codec_handle->printDebugLogs, "Enter");

    aocl_codec_handle->memLimit = codec_bench_handle->mem_limit;

    if (codec_bench_handle->print_stats == 0 &&
        codec_bench_handle->verify == 1)
        codec_bench_handle->iterations = 1;


    if (codec_bench_handle->codec_method < 0)
    {
        for (i = LZ4; i < AOCL_COMPRESSOR_ALGOS_NUM; i++)
        {
            if (i == LZMA || i == BZIP2 || i == SNAPPY || i == ZSTD)
                continue;

            aocl_codec_handle->optVar = codec_list[i].extra_param;

            for (l = codec_list[i].lower_level;
                 l <= codec_list[i].upper_level; l++)
            {
                codec_bench_handle->cTime = 0;
                codec_bench_handle->cSize = 0;
                codec_bench_handle->dTime = 0;
                codec_bench_handle->dSize = 0;
                codec_bench_handle->cBestTime = 0;
                codec_bench_handle->dBestTime = 0;
                aocl_codec_handle->level = l;

        switch (i)
        {
            case LZ4:
                ret = ipp_lz4_run(codec_bench_handle,
                                  aocl_codec_handle, hDL[i],
                                  &verifyRes);
                break;
            case LZ4HC:
                ret = ipp_lz4hc_run(codec_bench_handle,
                                    aocl_codec_handle, hDL[i],
                                    &verifyRes);
                break;
            case ZLIB:
                ret = ipp_zlib_run(codec_bench_handle,
                                   aocl_codec_handle, hDL[i],
                                   &verifyRes);
                break;
            default:
                return -2;

        }
        
        if (ret != 0 && ret != -3)
        {
            LOG(ERR, aocl_codec_handle->printDebugLogs,
                "Error in executing the dynamic library");
            return -1;
        }

            if (codec_bench_handle->verify)
            {
                if (verifyRes != 0)
                {
                    printf("AOCL-CODEC [%s-%ld] [Filename:%s] verification: failed\n",
                           codec_list[i].codec_name,
                           l, codec_bench_handle->fName);
                    return -1;
                }
            else
            {
                printf("AOCL-CODEC [%s-%ld] [Filename:%s] verification: passed\n",
                        codec_list[i].codec_name,
                        l, codec_bench_handle->fName);
            }

                }
                if (codec_bench_handle->print_stats)
                {
                    codec_bench_handle->cSpeed = (codec_bench_handle->inSize * 
                        codec_bench_handle->iterations * 1000.0) / 
                        codec_bench_handle->cTime;
                    codec_bench_handle->dSpeed = (codec_bench_handle->inSize * 
                        codec_bench_handle->iterations * 1000.0) / 
                        codec_bench_handle->dTime;
                    codec_bench_handle->cBestSpeed = 
                        (codec_bench_handle->inSize * 1000.0) / 
                        codec_bench_handle->cBestTime;
                    codec_bench_handle->dBestSpeed = 
                        (codec_bench_handle->inSize * 1000.0) / 
                        codec_bench_handle->dBestTime;
                    printf("AOCL-CODEC [%s-%ld] [Filename:%s] -------------------------------------\n",
                           codec_list[i].codec_name,
                           l, codec_bench_handle->fName);
                    printf("Compression:         speed(avg) %.2f MB/s, time(avg) %.2f ms, size %ld, speed(best) %.2f MB/s, time(best) %.2f ms\n",
                           codec_bench_handle->cSpeed,
                           codec_bench_handle->cTime/
                           (codec_bench_handle->iterations * 1000000.0),
                           codec_bench_handle->cSize/codec_bench_handle->iterations,
                           codec_bench_handle->cBestSpeed,
                           codec_bench_handle->cBestTime/1000000.0);
                    printf("Decompression:       speed(avg) %.2f MB/s, time(avg) %.2f ms, size %ld, speed(best) %.2f MB/s, time(best) %.2f ms\n", 
                           codec_bench_handle->dSpeed,
                           codec_bench_handle->dTime/
                           (codec_bench_handle->iterations * 1000000.0),
                           codec_bench_handle->dSize/codec_bench_handle->iterations,
                           codec_bench_handle->dBestSpeed,
                           codec_bench_handle->dBestTime/1000000.0);
                    printf("Ratio:               %.2f\n",
                           (((codec_bench_handle->cSize*100.0)/
                           codec_bench_handle->iterations)/
                           codec_bench_handle->inSize));
                }
            }
        }
    }
    else
    {
        INT def_level = aocl_codec_handle->level;
        INT lower_level, upper_level;

        if (def_level == UNINIT_LEVEL)
        {
            lower_level = 
                codec_list[codec_bench_handle->codec_method].lower_level;
            upper_level = 
                codec_list[codec_bench_handle->codec_method].upper_level;
        }
        else
        {
            lower_level = upper_level = def_level;
        }
        if (aocl_codec_handle->optVar == UNINIT_OPT_VAR)
            aocl_codec_handle->optVar = 
                codec_list[codec_bench_handle->codec_method].extra_param;
                
        for (l = lower_level; l <= upper_level; l++)
        {
            codec_bench_handle->cTime = 0;
            codec_bench_handle->cSize = 0;
            codec_bench_handle->dTime = 0;
            codec_bench_handle->dSize = 0;
            codec_bench_handle->cBestTime = 0;
            codec_bench_handle->dBestTime = 0;
            aocl_codec_handle->level = l;

        switch (codec_bench_handle->codec_method)
        {
            case LZ4:
                ret = ipp_lz4_run(codec_bench_handle,
                                  aocl_codec_handle,
                                  hDL[codec_bench_handle->codec_method],
                                  &verifyRes);
                break;
            case LZ4HC:
                ret = ipp_lz4hc_run(codec_bench_handle,
                                    aocl_codec_handle,
                                    hDL[codec_bench_handle->codec_method],
                                    &verifyRes);
                break;
            case ZLIB:
                ret = ipp_zlib_run(codec_bench_handle,
                                   aocl_codec_handle,
                                   hDL[codec_bench_handle->codec_method],
                                   &verifyRes);
                break;
            default:
                return -2;
        }

        if (ret != 0 && ret != -3)
        {
            LOG(ERR, aocl_codec_handle->printDebugLogs,
                "Error in executing the dynamic library");
            return -1;
        }

            if (codec_bench_handle->verify)
            {
                if (verifyRes != 0)
                {
                    printf("AOCL-CODEC [%s-%ld] [Filename:%s] verification: failed\n",
                            codec_list[codec_bench_handle->codec_method].codec_name,
                            aocl_codec_handle->level,
                            codec_bench_handle->fName);
                    return -1;
                }
                else
                {
                    printf("AOCL-CODEC [%s-%ld] [Filename:%s] verification: passed\n", 
                            codec_list[codec_bench_handle->codec_method].codec_name,
                            aocl_codec_handle->level,
                            codec_bench_handle->fName);
                }
			    
            }
            if (codec_bench_handle->print_stats)
            {        
                codec_bench_handle->cSpeed = (codec_bench_handle->inSize * 
                    codec_bench_handle->iterations * 1000.0) /
                    codec_bench_handle->cTime;
                codec_bench_handle->dSpeed = (codec_bench_handle->inSize * 
                    codec_bench_handle->iterations * 1000.0) /
                    codec_bench_handle->dTime;
                codec_bench_handle->cBestSpeed = (codec_bench_handle->inSize * 
                    1000.0) / codec_bench_handle->cBestTime;
                codec_bench_handle->dBestSpeed = (codec_bench_handle->inSize * 
                    1000.0) / codec_bench_handle->dBestTime;
                printf("AOCL-CODEC [%s-%ld] [Filename:%s] -------------------------------------\n",
                        codec_list[codec_bench_handle->codec_method].codec_name,
                        aocl_codec_handle->level, codec_bench_handle->fName);
                printf("Compression:         speed(avg) %.2f MB/s, time(avg) %.2f ms, size %ld, speed(best) %.2f MB/s, time(best) %.2f ms\n",
                        codec_bench_handle->cSpeed,
                        codec_bench_handle->cTime/
                        (codec_bench_handle->iterations * 1000000.0),
                        codec_bench_handle->cSize/codec_bench_handle->iterations,
                        codec_bench_handle->cBestSpeed,
                        codec_bench_handle->cBestTime/1000000.0);
                printf("Decompression:       speed(avg) %.2f MB/s, time(avg) %.2f ms, size %ld, speed(best) %.2f MB/s, time(best) %.2f ms\n", 
                        codec_bench_handle->dSpeed,
                        codec_bench_handle->dTime/
                        (codec_bench_handle->iterations * 1000000.0),
                        codec_bench_handle->dSize/codec_bench_handle->iterations,
                        codec_bench_handle->dBestSpeed,
                        codec_bench_handle->dBestTime/1000000.0);
                printf("Ratio:               %.2f\n",
                        (((codec_bench_handle->cSize*100.0)/
                        codec_bench_handle->iterations)/
                        codec_bench_handle->inSize));
            }
        }
    }
    LOG(TRACE, aocl_codec_handle->printDebugLogs, "Exit");

    return 0;
}

INT ipp_destroy(aocl_codec_desc *aocl_codec_handle)
{
    LOG(TRACE, aocl_codec_handle->printDebugLogs, "Enter");
    dlclose(hDL);
    LOG(TRACE, aocl_codec_handle->printDebugLogs, "Exit");
}

INT ipp_bench_run(aocl_codec_desc *aocl_codec_handle,
                  aocl_codec_bench_info *codec_bench_handle)
{
    LOG(TRACE, aocl_codec_handle->printDebugLogs, "Enter");

    INT64 ret = 0;
    
    ret = ipp_setup(codec_bench_handle, aocl_codec_handle);
    if (ret != 0)
        goto error_exit;
    
    ret = ipp_run(codec_bench_handle, aocl_codec_handle);
    if (ret != 0)
        goto error_exit;

    ipp_destroy(aocl_codec_handle);

    LOG(TRACE, aocl_codec_handle->printDebugLogs, "Exit");

    return 0;

error_exit:
    return ret;
}

