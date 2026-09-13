#include "HingeSensorReader.h"

#include <cmath>

HingeSensorReader::HingeSensorReader()
{
    Init();
}

HingeSensorReader::~HingeSensorReader()
{
    if (pSensor) pSensor->Release();
    if (pDataFields) pDataFields->Release();

    if (pSensorManager) {
        pSensorManager->Release();
        pSensorManager = NULL;
    }
}

HRESULT HingeSensorReader::Init()
{
    HRESULT hr = S_OK;

    try {

        ThrowIfFailed(CoInitializeEx(NULL, COINIT_MULTITHREADED));
        ThrowIfFailed(CoCreateInstance(
            CLSID_SensorManager,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&pSensorManager)
        ));

        REFSENSOR_ID sensorID = {
            0x020B0000,
            0x0000,
            0x0000,
            0x0000,
            0x000000000000
        };

        ThrowIfFailed(pSensorManager->GetSensorByID(
            sensorID,
            &pSensor
        ));

        ThrowIfFailed(pSensor->GetSupportedDataFields(
            &pDataFields
        ));

        ThrowIfFailed(pDataFields->GetAt(3, &pAngleKey));
        ThrowIfFailed(pDataFields->GetAt(4, &pLidAngleKey));
        ThrowIfFailed(pDataFields->GetAt(5, &pBodyAngleKey));
    }
    catch (HRESULT hrError) {
        hr = hrError;
    }

    if (pSensorManager) {
        pSensorManager->Release();
        pSensorManager = NULL;
    }

    return hr;
}

bool HingeSensorReader::IsReady() const
{
    return pSensor != NULL;
}

HRESULT HingeSensorReader::GetHingeAngleFloat(int* angle, int* lidAngle, int* bodyAngle) {
}

HRESULT HingeSensorReader::GetHingeAngleFloat(int* angle, int* lidAngle, int* bodyAngle) {
    if (useRawAccelerometer)
    {
        double lx, ly, lz;
        double bx, by, bz;

        HRESULT hr = accelerometerReader.GetReading(
            0,
            &lx,
            &ly,
            &lz
        );

        if (FAILED(hr))
            return hr;

        hr = accelerometerReader.GetReading(
            1,
            &bx,
            &by,
            &bz
        );

        if (FAILED(hr))
            return hr;

        float lidLength = sqrtf(lx * lx + ly * ly + lz * lz);
        float bodyLength = sqrtf(bx * bx + by * by + bz * bz);

        if (lidLength == 0.0f || bodyLength == 0.0f)
            return E_FAIL;

        lx /= lidLength;
        ly /= lidLength;
        lz /= lidLength;

        bx /= bodyLength;
        by /= bodyLength;
        bz /= bodyLength;

        float dot =
            lx * bx +
            ly * by +
            lz * bz;

        if (dot > 1.0f)
            dot = 1.0f;

        if (dot < -1.0f)
            dot = -1.0f;

        *angle = acosf(dot) * 180.0f / 3.14159265358979323846f;

        return S_OK;
    }

    HRESULT hr = S_OK;
    ISensorDataReport* pDataReport = NULL;

    try {
        ThrowIfFailed(pSensor->GetData(&pDataReport));

        PROPVARIANT pAngleValue, pBodyAngleValue, pLidyAngleValue;

        PropVariantInit(&pAngleValue);
        PropVariantInit(&pBodyAngleValue);
        PropVariantInit(&pLidyAngleValue);

        ThrowIfFailed(pDataReport->GetSensorValue(
            pAngleKey,
            &pAngleValue
        ));

        ThrowIfFailed(pDataReport->GetSensorValue(
            pLidAngleKey,
            &pBodyAngleValue
        ));

        ThrowIfFailed(pDataReport->GetSensorValue(
            pBodyAngleKey,
            &pLidyAngleValue
        ));

        *angle = pAngleValue.intVal;
        *lidAngle = pLidyAngleValue.intVal;
        *bodyAngle = pBodyAngleValue.intVal;

        ThrowIfFailed(PropVariantClear(&pAngleValue));
        ThrowIfFailed(PropVariantClear(&pBodyAngleValue));
        ThrowIfFailed(PropVariantClear(&pLidyAngleValue));
    }
    catch (HRESULT hrError) {
        hr = hrError;
    }

    if (pDataReport)
        pDataReport->Release();

    return hr;
}