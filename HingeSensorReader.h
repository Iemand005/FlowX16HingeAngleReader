#pragma once

#include <iostream>
#include <string>
#include <iomanip>

#include <Windows.h>

#include <sensorsapi.h>
#include <sensors.h>
#pragma comment(lib, "sensorsapi.lib")


class HingeSensorReader
{
private:
    ISensorManager* pSensorManager = NULL;
    ISensorCollection* pSensorCollection = NULL;
    ISensor* pSensor = NULL;
    IPortableDeviceKeyCollection* pDataFields = NULL;

    PROPERTYKEY pAngleKey, pLidAngleKey, pBodyAngleKey;

    void ThrowIfFailed(HRESULT hr) {
        if (FAILED(hr)) throw hr;
    }

public:
    HingeSensorReader();
    ~HingeSensorReader();

    HRESULT Init();
    
    HRESULT GetHingeAngle(int* angle, int* lidAngle, int* bodyAngle);
};

