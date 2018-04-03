/*
 * The Clear BSD License
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 * that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Generated by erpcgen 1.5.0 on Mon Nov 13 15:29:32 2017.
 *
 * AUTOGENERATED - DO NOT EDIT
 */


#include "erpc_matrix_multiply_server.h"
#include <new>
#include "erpc_port.h"

#if 10500 != ERPC_VERSION_NUMBER
#error "The generated shim code version is different to the rest of eRPC code."
#endif


#if !defined(ERPC_GENERATED_CRC) || ERPC_GENERATED_CRC != 47304
#error "Macro 'ERPC_GENERATED_CRC' should be defined with value 47304."
#endif


using namespace erpc;
using namespace std;

#if ERPC_NESTED_CALLS_DETECTION
extern bool nestingDetection;
#endif

// for mdk/keil do not forgett add "--muldefweak" for linker
extern const uint32_t erpc_generated_crc;
#pragma weak erpc_generated_crc
extern const uint32_t erpc_generated_crc = 47304;


// Constant variable definitions
#pragma weak matrix_size
extern const int32_t matrix_size = 5;

// Call the correct server shim based on method unique ID.
erpc_status_t MatrixMultiplyService_service::handleInvocation(uint32_t methodId, uint32_t sequence, Codec * codec, MessageBufferFactory *messageFactory)
{
    switch (methodId)
    {
        case kMatrixMultiplyService_erpcMatrixMultiply_id:
            return erpcMatrixMultiply_shim(codec, messageFactory, sequence);

        default:
            return kErpcStatus_InvalidArgument;
    }
}

// Server shim for erpcMatrixMultiply of MatrixMultiplyService interface.
erpc_status_t MatrixMultiplyService_service::erpcMatrixMultiply_shim(Codec * codec, MessageBufferFactory *messageFactory, uint32_t sequence)
{
    erpc_status_t err = kErpcStatus_Success;

    Matrix matrix1;
    Matrix matrix2;
    Matrix result_matrix;

    // startReadMessage() was already called before this shim was invoked.

    if (!err)
    {
        for (uint32_t arrayCount1 = 0; arrayCount1 < 5; ++arrayCount1)
        {
            for (uint32_t arrayCount2 = 0; arrayCount2 < 5; ++arrayCount2)
            {
                if (!err)
                {
                    err = codec->read(&matrix1[arrayCount1][arrayCount2]);
                }
                else
                {
                    break;
                }
            }
        }
    }

    if (!err)
    {
        for (uint32_t arrayCount1 = 0; arrayCount1 < 5; ++arrayCount1)
        {
            for (uint32_t arrayCount2 = 0; arrayCount2 < 5; ++arrayCount2)
            {
                if (!err)
                {
                    err = codec->read(&matrix2[arrayCount1][arrayCount2]);
                }
                else
                {
                    break;
                }
            }
        }
    }

    if (!err)
    {
        err = codec->endReadMessage();
    }

    // Invoke the actual served function.
#if ERPC_NESTED_CALLS_DETECTION
    nestingDetection = true;
#endif
    if (!err)
    {
        erpcMatrixMultiply(matrix1, matrix2, result_matrix);
    }
#if ERPC_NESTED_CALLS_DETECTION
    nestingDetection = false;
#endif

    // preparing MessageBuffer for serializing data
    if (!err)
    {
        err = messageFactory->prepareServerBufferForSend(codec->getBuffer());
    }

    // preparing codec for serializing data
    codec->reset();

    // Build response message.
    if (!err)
    {
        err = codec->startWriteMessage(kReplyMessage, kMatrixMultiplyService_service_id, kMatrixMultiplyService_erpcMatrixMultiply_id, sequence);
    }

    if (!err)
    {
        for (uint32_t arrayCount1 = 0; arrayCount1 < 5; ++arrayCount1)
        {
            for (uint32_t arrayCount2 = 0; arrayCount2 < 5; ++arrayCount2)
            {
                if (!err)
                {
                    err = codec->write(result_matrix[arrayCount1][arrayCount2]);
                }
                else
                {
                    break;
                }
            }
        }
    }

    if (!err)
    {
        err = codec->endWriteMessage();
    }

    return err;
}
erpc_service_t create_MatrixMultiplyService_service()
{
    return new (nothrow) MatrixMultiplyService_service();
}
