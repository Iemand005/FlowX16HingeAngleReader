#include "HingeSensorReader.h"

HingeSensorReader::HingeSensorReader()
{
    Init();
}

HingeSensorReader::~HingeSensorReader()
{
	if (pSensor) pSensor->Release();
	if (pDataFields) pDataFields->Release();
	if (pSensorManager) pSensorManager->Release();
}

HRESULT HingeSensorReader::Init()
{
    HRESULT hr = S_OK;

    try {

        ThrowIfFailed(CoInitializeEx(NULL, COINIT_MULTITHREADED));
        ThrowIfFailed(CoCreateInstance(CLSID_SensorManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pSensorManager)));

        REFSENSOR_ID sensorID = { 0x020B0000, 0x0000, 0x0000, 0x0000, 0x000000000000 };
        ThrowIfFailed(pSensorManager->GetSensorByID(sensorID, &pSensor));

        ThrowIfFailed(pSensor->GetSupportedDataFields(&pDataFields));

        ThrowIfFailed(pDataFields->GetAt(3, &pAngleKey));
        ThrowIfFailed(pDataFields->GetAt(4, &pLidAngleKey));
        ThrowIfFailed(pDataFields->GetAt(5, &pBodyAngleKey));
    }
    catch (HRESULT hrError) { hr = hrError; }

    pSensorManager->Release();
    pSensorManager = NULL;

    return hr;
}

bool HingeSensorReader::IsReady() const
{
    return pSensor != NULL;
}

HRESULT HingeSensorReader::GetHingeAngle(int* angle, int* lidAngle, int* bodyAngle)
{
    HRESULT hr = S_OK;
    int fieldIndex = 0;

    ISensorDataReport* pDataReport = NULL;

    try {
        ThrowIfFailed(pSensor->GetData(&pDataReport));

        PROPVARIANT pAngleValue, pBodyAngleValue, pLidyAngleValue;
        PropVariantInit(&pAngleValue);
        PropVariantInit(&pBodyAngleValue);
        PropVariantInit(&pLidyAngleValue);

        ThrowIfFailed(pDataReport->GetSensorValue(pAngleKey, &pAngleValue));
        ThrowIfFailed(pDataReport->GetSensorValue(pLidAngleKey, &pBodyAngleValue));
        ThrowIfFailed(pDataReport->GetSensorValue(pBodyAngleKey, &pLidyAngleValue));

        *angle = pAngleValue.intVal, *lidAngle = pLidyAngleValue.intVal, *bodyAngle = pBodyAngleValue.intVal;

        ThrowIfFailed(PropVariantClear(&pAngleValue));
        ThrowIfFailed(PropVariantClear(&pBodyAngleValue));
        ThrowIfFailed(PropVariantClear(&pLidyAngleValue));
    }
    catch (HRESULT hrError) { hr = hrError; }

    if (pDataReport) pDataReport->Release();

	return hr;
}