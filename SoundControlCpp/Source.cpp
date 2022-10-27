#include <iostream>
#include <Audioclient.h>
#include <mmdeviceapi.h>

#define REFTIMES_PER_SEC  10000000

HRESULT CreateAudioClient(IMMDevice* pDevice, IAudioClient** ppAudioClient)
{
    if (!pDevice)
    {
        return E_INVALIDARG;
    }

    if (!ppAudioClient)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    WAVEFORMATEX* pwfx = NULL;

    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;

    UINT32 nFrames = 0;

    IAudioClient* pAudioClient = NULL;

    // Get the audio client.
    hr = pDevice->Activate(
        __uuidof(IAudioClient),
        CLSCTX_ALL,
        NULL,
        (void**)&pAudioClient);

    // Get the device format.
    hr = pAudioClient->GetMixFormat(&pwfx);

    // Open the stream and associate it with an audio session.
    hr = pAudioClient->Initialize(
        AUDCLNT_SHAREMODE_EXCLUSIVE,
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
        hnsRequestedDuration,
        hnsRequestedDuration,
        pwfx,
        NULL);

    // If the requested buffer size is not aligned...
    if (hr == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED)
    {
        // Get the next aligned frame.
        hr = pAudioClient->GetBufferSize(&nFrames);

        hnsRequestedDuration = (REFERENCE_TIME)
            ((10000.0 * 1000 / pwfx->nSamplesPerSec * nFrames) + 0.5);

        // Release the previous allocations.
        CoTaskMemFree(pwfx);

        // Create a new audio client.
        hr = pDevice->Activate(
            __uuidof(IAudioClient),
            CLSCTX_ALL,
            NULL,
            (void**)&pAudioClient);

        // Get the device format.
        hr = pAudioClient->GetMixFormat(&pwfx);

        // Open the stream and associate it with an audio session.
        hr = pAudioClient->Initialize(
            AUDCLNT_SHAREMODE_EXCLUSIVE,
            AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
            hnsRequestedDuration,
            hnsRequestedDuration,
            pwfx,
            NULL);
    }
    else
    {
      
    }

    // Return to the caller.
    *(ppAudioClient) = pAudioClient;
    (*ppAudioClient)->AddRef();

done:

    // Clean up.
    CoTaskMemFree(pwfx);
    return hr;
}
