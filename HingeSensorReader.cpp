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

    return hr;
}

HRESULT HingeSensorReader::GetHingeAngle(int* angle, int* lidAngle, int* bodyAngle)
{
    HRESULT hr = S_OK;
    int fieldIndex = 0;

    ISensorDataReport* pDataReport = NULL;

    try {
        ThrowIfFailed(pSensor->GetData(&pDataReport));

        PROPVARIANT pAngleValue, pLidAngleValue, pBodyAngleValue;
        PropVariantInit(&pAngleValue);
        PropVariantInit(&pLidAngleValue);
        PropVariantInit(&pBodyAngleValue);

        ThrowIfFailed(pDataReport->GetSensorValue(pAngleKey, &pAngleValue));
        ThrowIfFailed(pDataReport->GetSensorValue(pLidAngleKey, &pLidAngleValue));
        ThrowIfFailed(pDataReport->GetSensorValue(pBodyAngleKey, &pBodyAngleValue));

        *angle = pAngleValue.intVal, *lidAngle = pBodyAngleValue.intVal, *bodyAngle = pLidAngleValue.intVal;

        ThrowIfFailed(PropVariantClear(&pAngleValue));
        ThrowIfFailed(PropVariantClear(&pLidAngleValue));
        ThrowIfFailed(PropVariantClear(&pBodyAngleValue));
    }
    catch (HRESULT hrError) { hr = hrError; }

    if (pDataReport) pDataReport->Release();

	return hr;
}